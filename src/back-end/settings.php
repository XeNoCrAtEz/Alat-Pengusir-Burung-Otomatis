<?php
    /* kode untuk menghandle pengaturan sistem */

    // connect to database
    $PA_IMM_DB = new mysqli("localhost", "root", "", "imm_proyek_akhir");
    if ( $PA_IMM_DB->connect_error ) {
        die (" Connection Failed : ". $PA_IMM_DB->connect_error);
    }
    
    $handler = curl_init();
    
    // fungsi untuk mengambil data pengaturan sistem yang disimpan di DB
    // fungsi ini mengembalikan array yang berisi nilai 'jadwal_on', 'jadwal_off', 'sensitivitas', dan 'power'
    function getSettingsDB() {
        // lakukan sql untuk mengambil data dari DB
        $sql = 'SELECT * FROM system_settings';
        $PA_IMM_DB = $GLOBALS['PA_IMM_DB'];
        $result = $PA_IMM_DB->query($sql);
        $settings = $result->fetch_assoc();
        return $settings;
    }

    // fungsi untuk mengirim pengaturan ke main controller
    function sendSettingsToESP() {
        // ambil settingan sistem dari DB
        $settings = getSettingsDB();
        // hitung selang waktu yang dibutuhkan sblm ON atau OFF kembali
        $now = date('H:i');
        $secsTillTurnOFF = strtotime($settings['jadwal_off']) - strtotime($now);
        if ( $secsTillTurnOFF < 0 ) {
            $secsTillTurnOFF += 24*3600;      // tambahkan 24 jam klo hasilnya negatif
        }
        $secsTillTurnON = strtotime($settings['jadwal_on']) - strtotime($now);
        if ( $secsTillTurnON < 0 ) {
            $secsTillTurnON += 24*3600;       // tambahkan 24 jam klo hasilnya negatif
        }

        // masukkan data ke URL
        $link = "http://192.168.43.27/changeSettings?sysStatus=" . $settings['power'];
        $link .= "&secsTillTurnOFF=" . $secsTillTurnOFF;
        $link .= "&secsTillTurnON=" . $secsTillTurnON;
        $link .= "&sysSensitivity=" . $settings['sensitivitas'];

        // lalu kirim ke ESP
        $handler = $GLOBALS['handler'];
        curl_setopt($handler, CURLOPT_URL, $link);
        curl_exec($handler);
    }
    
    // kode ini dipanggil oleh main ctrl untuk mengambil pengaturan sistem dari server
    if ( $_GET['procedure'] == 'getSettingsESP' ) {
        sendSettingsToESP();
    }

    // kode ini untuk menerima perubahan nilai sensitivitas dari website
    if ( $_GET['procedure'] == 'changeSensitivity' ) {
        // masukkan pengaturan ke DB
        $sensitivityValue = $_GET['sensitivity'];
        $sql = 'UPDATE system_settings SET sensitivitas = '. $sensitivityValue;
        if ( $PA_IMM_DB->query($sql) === FALSE ) {
            die("SQL error : " . $PA_IMM_DB->error);
        }
        sendSettingsToESP();    // kirim pengaturan ke ESP
    }

    // kode ini untuk menerima perubahan jadwal ON dan OFF dari website
    if ( $_GET['procedure'] == 'changeSchedule' ) {
        // masukkan pengaturan ke DB
        $jadwalON = $_GET['jadwalON'];
        $jadwalOFF = $_GET['jadwalOFF'];
        $sql = 'UPDATE system_settings SET jadwal_on="' . $jadwalON . '", jadwal_off="' . $jadwalOFF . '";';
        if ( $PA_IMM_DB->query($sql) === FALSE ) {
            die("SQL error : " . $PA_IMM_DB->error);
        }
        sendSettingsToESP();    // kirim pengaturan ke ESP
    }

    // jika sistem diminta untuk non-aktif
    if ( $_GET['procedure'] == 'turnOFFSys' ) {
        $sql = 'UPDATE system_settings SET power=0';
        if ( $PA_IMM_DB->query($sql) === FALSE ) {
            die("SQL error : " . $PA_IMM_DB->error);
        }
        sendSettingsToESP();
    }

    // jika sistem diminta untuk aktif
    if ( $_GET['procedure'] == 'turnONSys' ) {
        $sql = 'UPDATE system_settings SET power=1';
        if ( $PA_IMM_DB->query($sql) === FALSE ) {
            die("SQL error : " . $PA_IMM_DB->error);
        }
        sendSettingsToESP();
    }

    // jika website meminta data settingan sistem
    if ( $_GET['procedure'] == 'getWebsiteSettings' ) {
        $settings = getSettingsDB();
        // ubah format waktu "07:00:00" menjadi "07:00"
        $settings['jadwal_on'] = date('H:i', strtotime($settings['jadwal_on']));
        $settings['jadwal_off'] = date('H:i', strtotime($settings['jadwal_off']));
        echo json_encode($settings);
    }
?>