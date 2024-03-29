<?php slot::start('head') ?>
  <title>Memcache Status</title>
<?php slot::end() ?>

<h1>Memcache Status</h1>
<?php 
if (is_array($cache_stats)) {
    foreach ($cache_stats  as $server=>$stats) {
        echo "<h3>{$server}</h3>";    
        if (!empty($stats) && is_array($stats)) {
            echo '<ul>';
            echo '<li>Gets: '.$stats['get_hits'].'</li>';
            echo '<li>Misses: '.$stats['get_misses'].'</li>';
            echo '<li>Total Gets: '.($stats['get_hits']+$stats['get_misses']).'</li>';
            echo '<li>Hit %: '.$stats['get_hits']*100/($stats['get_hits']+$stats['get_misses']).'</li>';
            echo '<li>Quota %: '.$stats['bytes']*100/$stats['limit_maxbytes'].'</li>';
            echo '</ul>';
            echo '<ul>';
            foreach ($stats as $key=>$val) {
                echo "<li>{$key}: {$val}</li>";
            }
            echo '</ul>';
        } else {
            echo "<ul><li>Failed to connect to {$server}</li></ul>";
        }
    }
} else {
    echo '<p>Memcache is not connected or stats are unavailable.</p>';
}
?>
