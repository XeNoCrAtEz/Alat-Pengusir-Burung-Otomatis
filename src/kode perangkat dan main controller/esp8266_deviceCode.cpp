/**************************************************************************************************
 * Kode untuk perangkat pendeteksi burung
 **************************************************************************************************/

#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <Ticker.h>


/****************** Deklarasi fungsi-fungsi dalam perangkat ******************/
void init_wifi();               // fungsi untuk inisialisasi wifi
// fungsi untuk menggoyangkan tali jika terdeteksi burung
void shake_string( int duration_ms, int delay_ms );
void update_self_data();        // fungsi untuk mengambil dan mengupdate data perangkat
void send_dev_data();           // fungsi untuk mengirimkan data ke main ctrl
void turn_off();                // fungsi untuk menon-aktifkan perangkat selama 60 menit


/****************** Deklarasi variable dalam perangkat ******************/
// nomor pin untuk sensor PIR, servo, dan sensor tegangan baterai
#define pinIN_PIR       D1
#define pinIN_battProbe A0
#define pinOUT_Servo    D4

// variabel servo pemegang tali pengusir
Servo servo_pengusir;

// ssid dan pass dari main ctrl
#define AP_ssid "Alat Pengusir Burung"
#define AP_pass "123456789"

// variabel untuk komunikasi HTTP
IPAddress mainCtrl_IPAddr(192, 168, 4, 1);      // IP Main Controller
String Link;            // untuk menyimpan link HTTP
String header;          // untuk menyimpan header HTTP dari main ctrl
WiFiClient client;
HTTPClient http;        // untuk melakukan HTTP ke main ctrlS
StaticJsonDocument<100> sysSettingsJSON;        // untuk menyimpan respons main ctrl mengenai pengaturan sistem

// deklarasi variabel penyimpan data perangkat
const int devNum = 2;       // data nomor perangkat
int devBattPercentage;      // persentase baterai
bool devStatus = true;      // status keaktifan perangkat
int devSensitivity;         // nilai sensitivitas perangkat (dalam persen)

// deklarasi variabel untuk mengirim data tiap 1 menit
unsigned long previousTime;

/***************************************************************************/
/***************** Program Utama Perangkat Pengusir Burung *****************/
void setup() {
    // komunikasi serial untuk debugging
    Serial.begin(115200);
    init_wifi();        // mulai inisialisasi wifi
    // atur pin untuk menghitung nilai tegangan baterai dan pin untuk PIR
    pinMode(pinIN_battProbe, INPUT);
    pinMode(pinIN_PIR, INPUT);
    // update data perangkat untuk pertama kalinya
    update_self_data();
    // setup pin pengendali servo
    servo_pengusir.attach(pinOUT_Servo);
    // setup timer untuk mengupdate dan mengirim data tiap 1 menit
    // sendDevStats_Ticker.attach(60, update_self_data);
}

void loop() {
    if ( millis() - previousTime > (unsigned long) 60000 ) {
        previousTime = millis();
        update_self_data();
    }
    // cek selalu nilai pembacaan sensor PIR. Jika terdeteksi burung...
    if ( digitalRead(pinIN_PIR) == HIGH && devSensitivity > 0 ) {
        Serial.println("Bird Detected!");
        // beritahu main ctrl bahwa seekor burung telah terdeteksi
        Serial.println("Sending report to Main Controller...\n");
        Link = "http://" + mainCtrl_IPAddr.toString() + "/birdDetected";
        http.begin(client, Link);
        http.GET();
        http.end();      // biarkan server yang melakukan http.end()
        
        shake_string(5000, 200);        // goyangkan tali
    }
}
/***************************************************************************/


/****************** Definisi fungsi-fungsi dalam perangkat ******************/
void init_wifi() {
    // mulai koneksi dengan main ctrl
    Serial.println("Connecting to Main Controller...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(AP_ssid, AP_pass);
    while ( WiFi.status() != WL_CONNECTED ) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nDevice is connected to Main Controller");
    Serial.print("IP Address : ");
    Serial.println(WiFi.localIP());
    Serial.println();
    delay(1000);
}

void shake_string( int duration_ms, int delay_ms ) {
    Serial.println("Shaking String...\n ");
    int time_before = millis();
    while ( millis() - time_before < (unsigned long) duration_ms ) {    // selama dalam durasi waktu "duration_ms"
        servo_pengusir.write(0);        // ke arah 0 derajat
        delay(delay_ms);                // beri jeda...
        servo_pengusir.write(180);      // ke arah 180 derajat
        delay(delay_ms);                // beri jeda...
    }
    servo_pengusir.write(90);           // kembalikan posisi servo ke semula
}

void update_self_data() {
    Serial.println("Updating self data...");
    int battDigitalValue = analogRead(pinIN_battProbe);     // ambil nilai dari tegangan baterai
    devBattPercentage = (int) (battDigitalValue - 650) / 374 * 100;     // ubah ke persen

    Serial.print("Battery Percentage : ");
    Serial.println(devBattPercentage);
    Serial.println();
    // ubah status keaktifan berdasarkan persentase baterai
    // jika device masih menyala, tetapi persentase baterainya < 10%...
    if ( devStatus == true && devBattPercentage < 10 ) {
        Serial.println("Battery Low!! Turning OFF Device...");
        devStatus = false;      // ubah status device menjadi non-aktif
        send_dev_data();        // beritahu main ctrl bahwa perangkat akan non-aktif
        turn_off();             // non-aktifkan perangkat
    } else {
        // kirim data ke main ctrl
        send_dev_data();
    }
}

void send_dev_data() {
    Serial.println("Sending device data to Main Controller...");
    // siapkan link dan data yang akan dikirimkan
    Link = "http://" + mainCtrl_IPAddr.toString() + "/recvDevData";

    String devData = "devNum=" + String(devNum);
    devData += "&battPercent=" + String(devBattPercentage);
    devData += "&stat=" + String(devStatus);

    // kirim data ke main ctrl
    http.begin(client, Link);
    http.addHeader("Content-Type", "text/plain");
    int responseCode = http.POST(devData);      // kirim menggunakan method POST dan ambil response code nya
    
    if ( responseCode != HTTP_CODE_OK ) {
        // tampilkan error jika respons dari server bukan OK
        Serial.println("\n\nError when getting system settings from Main Controller...");
        Serial.println("Server Response Code : " + String(responseCode));
        Serial.println(http.errorToString(responseCode));
        return;
    }
    Serial.println("Data has been sent!");

    if ( devStatus != false ) {     // jika perangkat akan segera non-aktif, tidak perlu menunggu respons dari main ctrl
        Serial.println("Parsing system settings...");
        
        // ambil data JSON yang dikirimkan oleh main ctrl
        String mainCtrlResponse = http.getString();
        http.end();      // biarkan server yang melakukan http.end()
        Serial.println(mainCtrlResponse);

        // respons dari main ctrl adalah "sensitivity=20&devStatus=1"
        // parse nilai sensitivitas dan status perangkat tersebut
        DeserializationError error = deserializeJson(sysSettingsJSON, mainCtrlResponse);
        // Test if parsing succeeds.
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
        }
        
        // simpan ke variabel
        devSensitivity = sysSettingsJSON["sensitivity"];
        bool devStatusValue = sysSettingsJSON["devStatus"].as<bool>();

        Serial.println("Parsed system settings :");
        Serial.print("sensitivity\t\t");
        Serial.println(devSensitivity);
        Serial.print("devStatus\t\t");
        Serial.println(devStatusValue);
        Serial.println();
        if ( (devStatus == true) && (devStatusValue == false) ) {
            Serial.println("Device must turn OFF. Turning OFF now...");
            // jika perangkat harusnya non-aktif, segera matikan perangkat
            turn_off();
        } else {
            devStatus = devStatusValue;
        }
    }
    http.end();      // biarkan server yang melakukan http.end()
}

void turn_off() {
    Serial.println("Device is OFF");
    ESP.deepSleep(3600e6);      // masuk ke mode deep sleep selama 60 menit
}