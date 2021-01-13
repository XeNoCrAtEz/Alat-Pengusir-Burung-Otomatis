/**********************************************************************************
 * Kode untuk ESP32 Main Controller
 *********************************************************************************/

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <Ticker.h>


/****************** Deklarasi fungsi-fungsi Main Controller ******************/
void init_wifi();               // fungsi untuk inisialisasi wifi sebagai Access Point
void init_web_server();         // fungsi untuk inisialisasi async web server
void turn_off_system();         // fungsi untuk menon-aktifkan sistem
void turn_on_system();          // fungsi untuk mengaktifkan sistem
void ask_system_settings();     // fungsi untuk menanyakan pengaturan sistem kepada server


/****************** Deklarasi variable Main Controller ******************/
// ssid dan pass access point
#define AP_ssid "Alat Pengusir Burung"
#define AP_pass "123456789"

// ssid dan pass untuk hotspot HP
#define WIFI_ssid "XeNoCrAtEz_"
#define WIFI_pass "aptxsxfc2"

// variabel untuk komunikasi WiFi HTTP ke perangkat maupun server
AsyncWebServer asyncServer(80);         // AsyncWebServer di port 80
IPAddress IP_server(192, 168, 43, 26);  // IP Address server
IPAddress IP_self(192, 168, 43, 27);    // IP address dari main controller
String Link;                // untuk menyimpan link HTTP
HTTPClient http;            // untuk melakukan HTTP

// deklarasi variabel penyimpan pengaturan sistem
int turnOFFin_secs = 0;          // menyimpan berapa lama sistem harus ON (penyimpan jadwal ON)
int turnONin_secs = 0;         // menyimpan berapa lama sistem harus OFF (penyimpan jadwal OFF)
int sysSensitivity = 100;   // menyimpan nilai sensitivitas sistem
int sysStatus = 1;          // menyimpan status keaktifan sistem

// deklarasi variabel untuk jadwal on dan off
Ticker timer_turnON;
Ticker timer_turnOFF;


/***************************************************************************/
/********************** Program Utama Main Controller **********************/
void setup() {
    // komunikasi serial untuk debugging
    Serial.begin(115200);
    // mulai inisialisasi wifi
    init_wifi();
    // setup async web server
    init_web_server();
	delay(1000);
    // minta pengaturan awal ke server
    ask_system_settings();
	Serial.println("Main Controller is ON and running!\n\n");
}

void loop () {
}
/***************************************************************************/


/****************** Definisi fungsi-fungsi dalam perangkat ******************/
void init_wifi() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_ssid, AP_pass);      // aktifkan wifi access point
    Serial.print("\n\nAccess Point IP Address : ");
    Serial.println(WiFi.softAPIP());

    WiFi.begin(WIFI_ssid, WIFI_pass);   // connect ke hotspot HP
    Serial.print("Connecting to ");
    Serial.println(WIFI_ssid);
    while ( WiFi.status() != WL_CONNECTED ) {
        delay (1000);
        Serial.print(".");
    }
    Serial.println("\nConnection Successful!\n");
    Serial.print("Main Controller IP Address : ");
    Serial.println(WiFi.localIP());
	Serial.println();
    delay(1000);
}

void init_web_server() {
	// handle pengguna yang meminta website pengusir burung
	asyncServer.on("/", HTTP_GET, [] (AsyncWebServerRequest *request) {
		Serial.println("Received request for the website");
		
		// redirect pengguna ke server
		Serial.println("Redirecting user...");
		request->redirect("http://" + IP_server.toString() + "/IMM/Proyek_Akhir/front-end");
		Serial.println("User Redirected!\n");
	});

	// handle sinyal "ada burung"
	asyncServer.on("/birdDetected", HTTP_GET, [] (AsyncWebServerRequest *request) {
		Serial.println("Received \"bird detected\" signal");
		request->send(200);			// respon dengan OK
		
		// beri tahu server
		Serial.println("Updating log...");
		Link = "http://" + IP_server.toString() + "/IMM/Proyek_Akhir/back-end/dataRecv.php";
		String birdDetectedString = "procedure=updateLog&isDetected=1";
		http.begin(Link);
		http.addHeader("Content-Type", "application/x-www-form-urlencoded");
		int responseCode = http.POST(birdDetectedString);
		if ( responseCode != HTTP_CODE_OK ) {
			Serial.println(http.getString());
			Serial.println("Error when sending \"bird detected\" signal");
			Serial.println(http.errorToString(responseCode));
		}
		http.end();
		Serial.println("Log Updated!\n");
	});

	// handle perangkat yang mengirimkan datanya
	asyncServer.on("/recvDevData", HTTP_POST,
	[] (AsyncWebServerRequest *request) {
		Serial.println("Received data from device");
		Serial.println("Sending system settings back to device...");
		
		// respon dengan mengirimkan pengaturan sensitivitas dan status keaktifan dlm bntk JSON
		// {"sensitivity":50,"devStatus":1}
		String sysSettings = "{\"sensitivity\":" + String(sysSensitivity) + ",\"devStatus\":" + String(sysStatus) + "}";
		request->send(200, "text/plain", sysSettings);
	},
	NULL,
	[] (AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
		// kirim ulang data perangkat ke server
		Serial.println("Sending received data to server...");
		Link = "http://" + IP_server.toString() + "/IMM/Proyek_Akhir/back-end/dataRecv.php";
		String devData = "procedure=updateDevStat&";
		for ( int i = 0; i < len; i++ ) {
			devData += (char) data[i];
		}
		http.begin(Link);
		http.addHeader("Content-Type", "application/x-www-form-urlencoded");
		int responseCode = http.POST(devData);
		if ( responseCode != HTTP_CODE_OK ) {
			Serial.println(http.getString());
			Serial.println("Error when sending device data to server");
			Serial.println(http.errorToString(responseCode));
		}
		http.end();
		Serial.println("Device Data Updated!\n");
	});

	// handle web server yang meminta main controller untuk mengubah pengaturan sistem
	asyncServer.on("/changeSettings", HTTP_GET, [] (AsyncWebServerRequest *request) {
		Serial.println("Received command from server to change settings");
		request->send(200);

		// ambil pengaturan sistem yang diberikan oleh server dan update pengaturannya
		// data yang dikirim berbentuk "sysStatus=1&OFFTime=12345&ONTime=20000&sysSensitivity=50"
		Serial.println("Changing system settings...");
		sysStatus = request->getParam("sysStatus")->value().toInt();
		turnOFFin_secs = request->getParam("secsTillTurnOFF")->value().toInt();
		turnONin_secs = request->getParam("secsTillTurnON")->value().toInt();
		sysSensitivity = request->getParam("sysSensitivity")->value().toInt();
		
		// atur agar sistem otomatis mati pada waktu yang ditentukan
		timer_turnOFF.attach(turnOFFin_secs, turn_off_system);
		timer_turnON.attach(turnONin_secs, turn_on_system);

		Serial.println("System settings changed into :");
		Serial.print("sysStatus\t");
		Serial.println(sysStatus);
		Serial.print("sysSensitivity\t");
		Serial.println(sysSensitivity);
		Serial.print("turnONin_secs\t");
		Serial.println(turnONin_secs);
		Serial.print("turnOFFin_secs\t");
		Serial.println(turnOFFin_secs);
		Serial.println();
	});

	// aktifkan asyncWebServer
	asyncServer.begin();
}

void ask_system_settings() {
	Serial.println("Asking server for settings...");
    // siapkan link nya
    Link = "http://" + IP_server.toString() + "/IMM/Proyek_Akhir/back-end/settings.php?procedure=getSettingsESP";
    http.begin(Link);                   // minta pengaturan sistem ke server
    int responseCode = http.GET();      // ambil response code nya
    if ( responseCode != HTTP_CODE_OK) {
        // tampilkan error jika respons dari server bukan OK
        Serial.println("\n\nError when getting system settings from server...");
        Serial.println("Server Response Code : " + String(responseCode));
        Serial.println(http.errorToString(responseCode));
    }
	http.end();
}

void turn_off_system() {
    sysStatus = 0;			// ubah status sistem
	turnOFFin_secs = 24*3600;	// reset kembali turnOFFin_secs agar aktif 24 jam setelahnya
	timer_turnOFF.attach(turnOFFin_secs, turn_off_system);
	Serial.println("System is now turned OFF");
}

void turn_on_system() {
    sysStatus = 1;			// ubah status sistem
	turnONin_secs = 24*3600;	// reset kembali turnONin_secs agar aktif 24 jam setelahnya
	timer_turnON.attach(turnONin_secs, turn_on_system);
	Serial.println("System is now turned ON");
}