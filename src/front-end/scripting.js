/****** Fungsi untuk menampilkan data perangkat sesuai dengan data di server ******/
// fungsi untuk load data perangkat
function loadPerangkat() {
    let xhttp = new XMLHttpRequest();
    let contPsgnPerangkat = document.getElementsByClassName("container-statLogSet-dasar")[0];
    let url = "http://localhost/IMM/Proyek_Akhir/back-end/getDevStat.php"
    
    // ubah perangkat sesuai dengan data yang diterima dari server
    contPsgnPerangkat.innerHTML = '<p>Status Perangkat</p>';
    xhttp.onreadystatechange = function() {
        if ( this.readyState == 4 && this.status == 200 ) {
            contPsgnPerangkat.innerHTML += this.responseText;
        }
    };
    xhttp.open("GET", url, true);
    xhttp.send();
}

// fungsi untuk load data log pendeteksian burung
function loadLogData() {
    let xhttp = new XMLHttpRequest();
    let contPsgnPerangkat = document.getElementsByClassName("log-table")[1];
    let url = "http://localhost/IMM/Proyek_Akhir/back-end/getDetectionLog.php"
    
    // ubah log pendeteksian sesuai dengan data yang diterima dari server
    xhttp.onreadystatechange = function() {
        if ( this.readyState == 4 && this.status == 200 ) {
            contPsgnPerangkat.innerHTML = this.responseText;
            scrollLog();
        }
    };
    xhttp.open("GET", url, true);
    xhttp.send();
}

// fungsi untuk load data settingan sistem
function loadSysSettings() {
    let xhttp = new XMLHttpRequest();
    let url = "http://localhost/IMM/Proyek_Akhir/back-end/settings.php?procedure=getWebsiteSettings"
    
    // ubah nilai settingan sistem sesuai dengan data yang diterima dari server
    xhttp.onreadystatechange = function() {
        if ( this.readyState == 4 && this.status == 200 ) {
            // response text berupa JSON yang berisi nilai untuk 'jadwal-on' 'jadwal-off' 'sensitivity' 'power-state'
            let sysSettings = JSON.parse(this.responseText);

            document.getElementsByName('jadwal-on')[0].value = sysSettings.jadwal_on;
            document.getElementsByName('jadwal-off')[0].value = sysSettings.jadwal_off;
            document.getElementsByName('sensitivity')[0].value = sysSettings.sensitivitas;
            changeSensiValue();             // ubah tulisan penampil nilai sensitivitas
            
            // ubah bentuk dan tulisan tombol power berdasarkan settingan yang diterima
            let switchButton = document.getElementsByName("OnOffSwitch")[0];
            let powerState = document.getElementById("power-state");
            if ( sysSettings.power == 1 ) {
                switchButton.value = "turnOFF";
                switchButton.textContent = "Matikan Sistem";

                powerState.className = 'sys-pwr-state-ON';
                powerState.innerHTML = 'Sistem<br>Aktif';
            } else if ( sysSettings.power == 0 ) {
                switchButton.value = "turnON";
                switchButton.textContent = "Nyalakan Sistem";
                
                powerState.className = 'sys-pwr-state-OFF';
                powerState.innerHTML = 'Sistem Non-Aktif';
            }
        }
    };
    xhttp.open("GET", url, true);
    xhttp.send();
}


/****** fungsi untuk tombol-tombol di website ******/
// fungsi untuk enable pengubahan jadwal
function enableUbahJadwal() {
    document.getElementsByClassName('schedule-INPUT')[0].disabled = false;      // enable textbox
    document.getElementsByClassName('schedule-INPUT')[1].disabled = false;      // enable textbox
    document.getElementById('button-set-schedule').disabled = false;            // enable terapkan button
}

// fungsi untuk disable pengubahan jadwal
function disableUbahJadwal() {
    document.getElementsByClassName('schedule-INPUT')[0].disabled = true;      // enable textbox
    document.getElementsByClassName('schedule-INPUT')[1].disabled = true;      // enable textbox
    document.getElementById('button-set-schedule').disabled = true;
}

// fungsi untuk enable pengubahan sensitivitas
function enableUbahSensitivitas() {
    document.getElementById('sensitivity-slider').disabled = false;     // enable slider
    document.getElementById('button-set-sensi').disabled = false;       // enable terapkan button
}

// fungsi untuk disable pengubahan sensitivitas
function disableUbahSensitivitas() {
    document.getElementById('sensitivity-slider').disabled = true;      // enable slider
    document.getElementById('button-set-sensi').disabled = true;        // enable terapkan button
}

// fungsi untuk menyalakan atau mematikan sistem
function ONorOFFSystem() {
    let switchButton = document.getElementsByName("OnOffSwitch")[0];
    let powerState = document.getElementById("power-state");
    
    if ( switchButton.value == "turnOFF" ) {        // jika tombol tersebut akan menon-aktifkan sistem
        switchButton.value = "turnON";              // ubah tulisan jadi "Nyalakan Sistem"
        switchButton.textContent = "Nyalakan Sistem";
        
        powerState.className = 'sys-pwr-state-OFF'; // ubah tulisan "Sistem Aktif" menjadi "Sistem Non-Aktif"
        powerState.innerHTML = 'Sistem Non-Aktif';

        let xhttp = new XMLHttpRequest();           // ubah pengaturan di DB
        let url = "http://localhost/IMM/Proyek_Akhir/back-end/settings.php?procedure=turnOFFSys"
        xhttp.open("GET", url, true);
        xhttp.send();
        alert("Sistem pengusir burung telah dinon-aktifkan");

    } else if ( switchButton.value == "turnON") {            // jika tombol tersebut untuk mengaktifkan sistem
        switchButton.value = "turnOFF";
        switchButton.textContent = "Matikan Sistem";

        powerState.className = 'sys-pwr-state-ON';          // ubah tulisan "Sistem Non-Aktif" menjadi "Sistem Aktif"
        powerState.innerHTML = 'Sistem<br>Aktif';

        let xhttp = new XMLHttpRequest();                   // ubah pengaturan di DB
        let url = "http://localhost/IMM/Proyek_Akhir/back-end/settings.php?procedure=turnONSys"
        xhttp.open("GET", url, true);
        xhttp.send();
        alert("Sistem pengusir burung telah diaktifkan");
    }
}

// fungsi untuk mengubah nilai sensitivitas berdasarkan slider
function changeSensiValue() {
    let slider = document.getElementById('sensitivity-slider');
    let sensiValue = document.getElementById('nilai-sensi');
    sensiValue.innerHTML = slider.value;
}

// fungsi untuk autoscroll log table
// fungsi ini akan dipanggil ketika ada update terbaru mengenai log pendeteksian burung
function scrollLog() {
    let table = document.getElementsByClassName("table-content-scroll");
    table[0].scrollBy({
        top: table[0].scrollHeight,
        behavior: 'smooth'
    });
}

/****** Fungsi untuk melakukan submit form tanpa refresh ******/
function submitSchedule() {
    let xhttp = new XMLHttpRequest();
    let waktuON = document.getElementsByClassName("schedule-INPUT")[0].value;
    let waktuOFF = document.getElementsByClassName("schedule-INPUT")[1].value;
    
    if ( waktuON > "24:00" || waktuOFF > "24:00" ) {
        alert('Anda memasukkan waktu yang tidak valid. Mohon masukkan waktu yang kurang dari "24:00"');
        return false;
    }

    let url = 'http://localhost/IMM/Proyek_Akhir/back-end/settings.php?';
    url += 'procedure=changeSchedule&jadwalON=' + waktuON;
    url += '&jadwalOFF=' + waktuOFF;
    
    xhttp.open("GET", url, true);
    xhttp.send();

    alert("Jadwal ON dan OFF berhasil diubah!");

    // disable lagi tombol "terapkan" dan textbox untuk menerima inputan jadwal
    disableUbahJadwal();
    return false;
}

function submitSensitivity() {
    let xhttp = new XMLHttpRequest();
    let sensitivity = document.getElementById("sensitivity-slider");
    
    let url = 'http://localhost/IMM/Proyek_Akhir/back-end/settings.php?procedure=changeSensitivity&sensitivity=' + sensitivity.value;
    xhttp.open("GET", url, true);
    xhttp.send();
    
    alert("Sensitivitas sistem berhasil diubah!");

    // disable lagi tombol "terapkan" dan slider sensitivitas
    disableUbahSensitivitas();
    return false;
}

/****** Wrapping things Up... ******/
function setupWebsite() {
    loadPerangkat();
    loadSysSettings();
    loadLogData();
    setInterval(loadLogData, 60000);        // buat agar loadLogData() dijalankan tiap 1 menit
    setInterval(loadPerangkat, 60000);
}
window.onload = setupWebsite;