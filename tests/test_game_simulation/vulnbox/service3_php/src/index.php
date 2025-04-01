<?php 
    if (isset($_POST['action']) && $_POST['action'] == 'send') {
        $page = 0;
        if (is_numeric($_POST["page"])) {
            $page = intval($_POST["page"]);
        }
        $filename = './pages/page'.$page.'.txt';
        $flagid = trim($_POST["flagid"]);
        if ($flagid === '') {
            header('Location: ?p='.$page);
            exit();
        }
        $flagid = base64_encode($flagid);
        $flag = trim($_POST["flag"]);
        if ($flag === '') {
            header('Location: ?p='.$page);
            exit();
        }
        $flag = base64_encode($flag);
        $newline = "\n".$flagid.' '.$flag;

        $f = fopen($filename, 'aw') or die("Unable to open file! ".$filename);
        fwrite($f, $newline);
        fclose($f);
        header('Location: ?p='.$page);
        exit();
    }
?>
<html>
<head>
    <title>RAD Board</title>
    <link rel="stylesheet" type="text/css" href="index.css?ver=1">
</head>
<body>
<div class="header">
    <div class="logo"></div>
    <div class="pages">

<?php 
    if ($handle = opendir('./pages/')) {
        while (false !== ($entry = readdir($handle))) {
            if ($entry != "." && $entry != "..") {
                if (preg_match('/page(\d+)\.txt/', $entry, $matches, PREG_OFFSET_CAPTURE)) {
                    // print_r($matches);
                    $page = $matches[1][0];
                    echo '<a href="?p='.$page.'" class="page" >'.$page.'</a>';
                }
            }
        }
        closedir($handle);
    }
?>
</div>
</div>

<div class="flags">
<?php
    $p = 0;
    if (isset($_GET["p"])) {
        if (is_numeric( $_GET["p"] )) {
            $p = intval($_GET["p"]);
        }
    }
    echo '<h1>Page '.$p.'</h1>';
?>
<form method="POST" action='?p=<?php echo $p; ?> '>
    <input type="hidden" name="action" value="send">
    <input type="hidden" name="page" value="<?php echo $p; ?>">
    <input type="text" name="flagid" value="" placeholder="Flag ID">
    <input type="text" name="flag" value="" placeholder="Flag">
    <button>Send</button>
</form>

<?php
    $filename = './pages/page'.$p.'.txt';
    if (file_exists($filename)) {
        $handle = fopen($filename, "r");
        if ($handle) {
            while (($line = fgets($handle)) !== false) {
                if (trim($line) == '') {
                    continue;
                }
                $info = explode(' ', $line);
                $flag_id = base64_decode($info[0]);
                $flag_id = trim($flag_id);
                $flag_content = base64_decode($info[1]);
                $flag_content = trim($flag_content);
                echo '<div class="flag" '
                    .' flag="'.htmlspecialchars($flag_content).'" '
                    .' flagid="'.htmlspecialchars($flag_id).'"'
                    .' onclick="showFlag(this)">'
                    .'<div class="flag-id">'.htmlspecialchars($flag_id).'</div>'
                    .'<div class="flag-content">'.htmlspecialchars($flag_content).'</div>'
                    .'</div>';
            }
            fclose($handle);
        }
    }
?>
</div>

<script>
    function showFlag(el) {
        var els = document.getElementsByClassName('flag-content');
        for (var i = 0; i < els.length; i++) {
            els[i].style.display = '';
        }

        var flag = el.getAttribute('flag')
        var flagid = el.getAttribute('flagid');
        console.log(flagid + ' = ' + flag);
        var els = el.getElementsByClassName('flag-content');
        for (var i = 0; i < els.length; i++) {
            els[i].style.display = 'block';
        }
    }
</script>
</body>
</html>