<?php 
$dir = './ppfx_firmwares/';
$dfu_files = array();

if ($dh = opendir($dir)) {
    while (($file = readdir($dh)) !== false) {
        if (str_ends_with($file,".dfu"))
        {
            $filetime = filectime($dir.$file);
            $dfu_entry = array("fname" => $file,"createdDate" => date("d-m-Y H:i:s",$filetime),"timestamp" => $filetime  );
            $dfu_files[] = $dfu_entry;
        }

    }
    closedir($dh);
}
if ($_GET["dload"]!==NULL)
{
    $found = FALSE;
    foreach($dfu_files as $dfu_entry)
    {
        if ($dfu_entry["fname"]===$_GET["dload"])
        {
            $found = TRUE;
        }
    }
    if ($found===TRUE)
    {
        $fname = $dir.$_GET["dload"];
        $fp = fopen($fname, 'rb');
        header('Content-Type: application/octet_stream');
        header("Content-Length: " . filesize($fname));
        header('Content-Disposition: attachment; filename="' . $_GET["dload"] . '"');
        while ($chunk = fread($fp,256))
        {
            echo $chunk;
        }
        return;
    }
}
elseif ($_GET["fmt"]!==NULL)
{
    if ($_GET["fmt"]==="json")
    {
        header('Content-Type: application/json; charset=utf-8');
        echo json_encode($dfu_files);
        return;
    }
    elseif ($_GET["fmt"]==="xml")
    {
        header('Content-Type: text/xml; charset=utf-8');
        
        echo '<?xml version="1.0" encoding="UTF-8"?>'."\n<firmwares>\n";
        foreach($dfu_files as $dfu_entry)
        {
            echo "\t<dfuFile fname='".$dfu_entry["fname"]."' createdDate='".$dfu_entry["createdDate"]."' timestamp='".$dfu_entry["timestamp"]."'/>\n";

        }
        echo "</firmwares>\n";
        return;
    }
    else
    {
        http_response_code(501);
        echo "wrong format, choose either 'xml' or 'json'";
        return;
    }
}
echo "<html><head></head><body><h3>DFU Files for PiPicoFX, DaisySeed-based version</h3>";
foreach($dfu_files as $dfu_entry)
{
    echo "<li><a href='".$_SERVER['REQUEST_URI']."?dload=".$dfu_entry["fname"]."'>".$dfu_entry["fname"]." created: ".$dfu_entry["createdDate"]."</a></li>";
}
echo <<<BDY
<br>
the following parameters are valid:
<ul>
<li>fmt: "json" or "xml"; this returns the firmware files list as json or xml
<li>dload: filename of the dfu-file to download; download the firmware version defined,if available
</ul>
</body></html>
BDY;
?>
