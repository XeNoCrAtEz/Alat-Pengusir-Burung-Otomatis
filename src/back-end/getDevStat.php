<!-- Kode yang dapat dipanggil oleh JavaScript pada website sehingga
     data perangkat dalam database dapat ditampilkan di website
     
     Ketika file ini dipanggil oleh JS, maka ia akan mengambil data
     perangkat dari DB, memprosesnya, mengubahnya dalam bentuk batt$battHtml
     sesuai dengan format di website, lalu mengirimnya ke JS -->
<?php
    // fungsi untuk membuat HTML baterai
    function generateBattHTML( $baterai ) {
        $battHtml = '';             // buat string kosong
        if ( $baterai <= 100 && $baterai > 75 ) {
            $battHtml .= '<td><img src="icons/battery-100.png" alt="battery less than 100%"></td>';
            $battHtml .= '<td>Baterai</td>';
            $battHtml .= '<td class="bg-bat-75">' . $baterai . '%</td>';
        } else if ( $baterai <= 75 && $baterai > 50 ) {
            $battHtml .= '<td><img src="icons/battery-75.png" alt="battery less than 75%"></td>';
            $battHtml .= '<td>Baterai</td>';
            $battHtml .= '<td class="bg-bat-50">' . $baterai . '%</td>';
        } else if ( $baterai <= 50 && $baterai > 25 ) {
            $battHtml .= '<td><img src="icons/battery-50.png" alt="battery less than 50%"></td>';
            $battHtml .= '<td>Baterai</td>';
            $battHtml .= '<td class="bg-bat-25">' . $baterai . '%</td>';
        } else if ( $baterai <= 25 && $baterai >= 0 ) {
            $battHtml .= '<td><img src="icons/battery-25.png" alt="battery less than 25%"></td>';
            $battHtml .= '<td>Baterai</td>';
            $battHtml .= '<td class="bg-bat-0">' . $baterai . '%</td>';
        }
        return $battHtml;
    }

    // connect ke database
    $PA_IMM_DB = new mysqli("localhost", "root", "", "imm_proyek_akhir");
    if ( $PA_IMM_DB->connect_error ) {
        die (" Connection Failed : ". $PA_IMM_DB->connect_error);
    }
?>




<?php
    // ambil data dari DB
    $sql = 'SELECT * FROM status_alat';
    $result = $PA_IMM_DB->query($sql);
    $rowsCount = $result->num_rows;

    // karena diambil berpasang"an, berarti jumlah looping yang dilakukan adalah setengahnya saja
    for ( $i = 0; $i < $rowsCount/2; $i++ ) {
        echo '<section class="container-psgn">';
        // simpan kedua pasangan alat di dalam sebuah array
        $pasanganAlat = array( $result->fetch_assoc(), $result->fetch_assoc() );
        // buat HTML tiap alat
        $count = 1;
        foreach ( $pasanganAlat as $alat ) {
            $nomorAlat = $alat['nomor_alat'];
            $baterai = $alat['baterai'];
            $status = $alat['status'];

            echo '<div class="status-box">';
            echo '<p>Perangkat ' . $nomorAlat . '</p>';
            echo '<table><tr>';
            echo generateBattHTML($baterai);
            echo '</tr><tr>';
            echo '<td><img src="icons/power.png" alt="power-status-logo"></td>';
            echo '<td>Status</td>';
            if ( $status == 1 ) {
                echo '<td class="bg-stat-ON">ON</td>';
            } else {
                echo '<td class="bg-stat-OFF">OFF</td>';
            }
            echo '</tr></table></div>';

            if ( $count % 2 == 0 ) break;       // prevent inserting <p><b>Terhubung dengan</b></p> the second time
            else {
                $count += 1;
                echo '<p><b>Terhubung dengan</b></p>';
            }
            /*
                Hasilnya harusnya seperti ini :
                <div class="status-box">
                    <p>Perangkat 1</p>  <!-- nomor perangkat -->
                    <table>     <!-- tabel status perangkat -->
                        <tr>    <!-- status baterainya -->
                            <td><img src="icons/battery-50.png" alt="battery-lt-50%"></td>
                            <td>Battery</td>
                            <td class="bg-bat-50">50%</td>
                        </tr>
                        <tr>    <!-- status keaktifannya -->
                            <td><img src="icons/power.png" alt="power-status-logo"></td>
                            <td>Status</td>
                            <td class="bg-stat-ON">ON</td>
                        </tr>
                    </table>
                </div>
            */
        }
        echo '</section>';
    }
?>