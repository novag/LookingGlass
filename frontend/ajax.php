<?php
/**
 * CLG - LookingGlass - User friendly PHP frontend
 *
 * @package     CLG - LookingGlass
 * @author      Hendrik Hagendorn <w0h@w0h.de>
 * @copyright   2015 Hendrik Hagendorn
 * @license     http://opensource.org/licenses/GPL-3.0 GPL-3.0 License
 * @version     1.3.0
 */

require_once 'LookingGlass/RateLimit.php';
require_once 'LookingGlass/Config.php';

function generateJsonV4($lg_sock) {
    fwrite($lg_sock, 'route '.$_GET['arg']);
    $best = NULL;
    $path = NULL;
    $paths = array();
    $net_dest = NULL;
    while(($line = fgets($lg_sock)) !== false) {
        $line = trim($line);

        $expr = array();
        if(preg_match("/(.*)via\s+([0-9a-fA-F:\.]+)\s+on.*\[(\w+)\s+/", $line, $expr)) {
            if($path) {
                $path[] = $net_dest;
                $paths[] = $path;
                $path = NULL;
            }

            if(trim($expr[1])) {
                $net_dest = trim($expr[1]);
            }

            $peer_ip = trim($expr[2]);
            $peer_protocol_name = trim($expr[3]);
            # Check if via line is a internal route
            if($peer_ip == Config::BGP_ENDPOINT) {
                $path = array(Config::NODE_NAME);
            } else {
                # This makes it nicer
                $path = array($peer_protocol_name);
            }
        } else if(preg_match("/for ([0-9a-fA-F:\.\/]+)$/", $line, $expr)) {
            if(trim($expr[1])) {
                $net_dest = trim($expr[1]);
            }
        } else if(preg_match("/best #(\d+)/", $line, $expr)) {
            $best = intval($expr[1]) - 1;
        } else if(preg_match("/\(([0-9a-fA-F:\.\/]+)\)/", $line, $expr)) {
            $peer_ip = trim($expr[1]);

            # Check if via line is a internal route
            if($peer_ip == Config::BGP_ENDPOINT) {
                if($path) {
                    array_unshift($path, Config::NODE_NAME);
                } else {
                    $path[] = Config::NODE_NAME;
                }
            } else {
                if($path) {
                    array_unshift($path, "");
                } else {
                    $path[] = "";
                }
            }
            if($path) {
                $path[] = $net_dest;
                $paths[] = $path;
                $path = NULL;
            }
        }
        $expr2 = array();
        preg_match("/(.*)unreachable\s+\[(\w+)\s+/", $line, $expr2);
        if($expr2) {
            if($path) {
                $path[] = $net_dest;
                $paths[] = $path;
                $path = NULL;
            }

            if(trim($expr2[1])) {
                $net_dest = trim($expr2[1]);
            }
        }
        if(substr($line, 0, 12) === "BGP.as_path:") {
            $asns = explode(" ", trim(str_replace("BGP.as_path:", "", $line)));
            $path = array_merge($path, $asns);
        } else {
            $expr3 = array();
            if(preg_match("/^(\d+\s?)+$/", $line, $expr3)) {
                $ass = explode(" ", trim($expr3[0]));
                $path = $ass;
            }
        }
    }

    if($path) {
        $path[] = $net_dest;
        $paths[] = $path;
    }

    if($best != NULL) {
        $bestpath = $paths[$best];
        $paths[$best] = $paths[0];
        $paths[0] = $bestpath;
    }

    $data = array(Config::NODE_NAME => $paths);
    return htmlspecialchars(json_encode($data));
}

if(isset($_GET['cmd']) && isset($_GET['arg'])) {
    // available commands
    $cmds = array('whois',
                  'ping',
                  'ping6',
                  'traceroute',
                  'traceroute6',
                  'summary',
                  'summary6',
                  'route',
                  'route6',
                  'bgpmap',
                  'bgpmap6',
                  'as',
                  'as6');

    if(in_array($_GET['cmd'], $cmds)) {
        // instantiate RateLimit
        $limit = new RateLimit(Config::RATE_LIMIT);

        // check IP against database
        $limit->rateLimit(Config::RATE_LIMIT);

        // connect to remote lg daemon
        $addr = gethostbyname(Config::LG_SERVER);
        $lg_sock = stream_socket_client("tcp://".$addr.":".Config::LG_PORT, $errno, $errorMessage);
        if($lg_sock === false) {
            throw new Exception("Internal error");
        }

        if($_GET['cmd'] == "bgpmap") {
            $json = generateJsonV4($lg_sock);
            print('<img id="bgpmap" src="map.php?json='.$json.'">');
        } else if($_GET['cmd'] == "bgpmap6") {
            // TODO
            print(".");
        } else {
            if(isset($_GET['arg'])) {
                fwrite($lg_sock, $_GET['cmd'].' '.$_GET['arg']);
            } else {
                fwrite($lg_sock, $_GET['cmd']);
            }

            if(ob_get_level() == 0) ob_start();
            while(($line = fgets($lg_sock)) !== false) {
                print($line);

                ob_flush();
                flush();
            }
        }
        fclose($lg_sock);
        exit();
    }
}

exit('Unauthorized request');