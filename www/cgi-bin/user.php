#!/usr/bin/php
<?php
header("Content-Type: text/plain");

$user = getenv('USER');
if (!$user) {
    $user = "Unknown User";
}

echo "Hello " . $user . "!\n";
?>