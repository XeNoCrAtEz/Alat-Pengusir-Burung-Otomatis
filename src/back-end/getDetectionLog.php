<!-- Kode yang bisa dipanggil oleh Javascript pada website sehingga
     data logging di DB dapat ditampilkan di website.
     
     Ketika file ini dipanggil oleh JS, maka ia akan mengambil data
     log pendeteksian, memprosesnya menjadi HTML dan mengirimnya ke
     JS sesuai dengan format HTML sebelumnya -->

<tbody id="isi-log">

<?php
    // lakukan koneksi ke database
    $PA_IMM_DB = new mysqli("localhost", "root", "", "imm_proyek_akhir");
    if ( $PA_IMM_DB->connect_error ) {
        die ("Connection Failed : " . $PA_IMM_DB->connect_error);
    }

    // ambil log deteksi
    $sql_get_log = "SELECT * FROM log_pendeteksian";
    $result = $PA_IMM_DB->query($sql_get_log);
    if ( $result->num_rows > 0 ) {
        while ( $row = $result->fetch_assoc() ) {       // ambil baris satu per satu
            $date = strtotime($row['waktu']);
            $status_deteksi = $row['status_deteksi'];
            if ( $status_deteksi == 1 ) {           // jika status deteksinya 1...
                echo '<tr class="log-ada">';        // maka tr nya hrs menggunakan class "log-ada"
            } else {
                echo '<tr>';                        // jika tidak, maka tr tidak menggunakan class apapun
            }
            echo '<td>' . date('d-m-Y', $date) . ' &emsp; ' . date('H:i:s', $date) . '</td>';
            if ( $status_deteksi == 1 ) {
                echo '<td>Ada</td>';
            } else {
                echo '<td>Tidak Ada</td>';
            }
            echo '</tr>';
            /* 
               Hasilnya adalah seperti ini :
               <tr class="log-ada">
                    <td>20-12-2020 &emsp; 22:02:45</td>
                    <td>Ada</td>
               <tr>
            */
        }
    }
?>

</tbody>