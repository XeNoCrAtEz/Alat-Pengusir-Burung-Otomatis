<?php
    // kode PHP agar server dapat menerima status dari perangkat

    // server harus bisa menerima data kapan terdeteksi burung,
    // serta data status masing-masing perangkat
    
    // buat hubungan ke database
    $PA_IMM_DB = new mysqli("localhost", "root", "", "imm_proyek_akhir");
    if ( $PA_IMM_DB->connect_error ) {
        die ("Connection Failed : " . $PA_IMM_DB->connect_error);
    }
    // bagian ini menghandle data yang dikirimkan dan melakukan update
    // pada database sesuai dengan nilai 'procedure'
    if ( $_POST['procedure'] == "updateLog" ) {
        $isDetected = $_POST['isDetected'];
        $sql = "INSERT INTO log_pendeteksian (status_deteksi, waktu) VALUES ('" . $isDetected . "', NOW());";
        if ( $PA_IMM_DB->query($sql) === FALSE ) {
            die("SQL error : " . $PA_IMM_DB->error);
        }
    }
    if ( $_POST['procedure'] == "updateDevStat") {
        $deviceNum = $_POST['devNum'];
        $deviceBatt = $_POST['battPercent'];
        $deviceStat = $_POST['stat'];
        $sql = "UPDATE status_alat SET baterai=" . $deviceBatt . ", status=" . $deviceStat . " WHERE nomor_alat=" . $deviceNum;
        $sql .= ";INSERT INTO log_pendeteksian (status_deteksi, waktu) VALUES ('0', NOW());";
        if ( $PA_IMM_DB->multi_query($sql) === FALSE ) {
            die("SQL error : " . $PA_IMM_DB->error);
        }
    }
?>