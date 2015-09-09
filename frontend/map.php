<?php
/**
 * CLG - LookingGlass - BGP Map
 *
 * Modified PHP version of https://github.com/sileht/bird-lg/blob/6a7b/lg.py#L396
 *
 * @package     CLG - LookingGlass
 * @author      Hendrik Hagendorn <w0h@w0h.de>
 * @copyright   2015 Hendrik Hagendorn
 * @license     http://opensource.org/licenses/GPL-3.0 GPL-3.0 License
 * @version     1.0.0
 */

require_once 'RateLimit.php';
require_once 'Config.php';
require_once 'Image/GraphViz.php';

if(isset($_GET['json'])) {
    // instantiate RateLimit
    $limit = new RateLimit(Config::RATE_LIMIT);

    // check IP against database
    $limit->rateLimit(Config::RATE_LIMIT);

    $data = json_decode($_GET['json'], true);
    show_bgpmap($data);

    exit();
}

exit('Unauthorized request');

function show_bgpmap($data) {
    // return a bgp map in a png file
    global $graph, $nodes, $edges;
    $graph = new Image_GraphViz(true, array(), "BGPMAP");

    $nodes = array();
    $edges = array();

    function escape($label) {
        $label = str_replace("&", "&amp;", $label);
        $label = str_replace(">", "&gt;", $label);
        $label = str_replace("<", "&lt;", $label);

        return $label;
    }

    function add_node($_as, $fillcolor = NULL, $label = NULL, $shape = NULL) {
        global $nodes, $graph;

        if(!array_key_exists($_as, $nodes)) {
            if(!$label) {
                $label = "AS".$_as;
            }

            $label = '<table cellborder="0" border="0" cellpadding="0" cellspacing="0"><tr><td align="center">'
                    .str_replace("\r", "<br/>", escape($label)).'</td></tr></table>';
            $attrs = array('style' => 'filled', 'fontsize' => '10', 'label' => $label);
            
            if($shape) {
                $attrs['shape'] = $shape;
            }

            if($fillcolor) {
                $attrs['fillcolor'] = $fillcolor;
            }

            $nodes[$_as] = $attrs;
            $graph->addNode($_as, $nodes[$_as]);
        }
        return $nodes[$_as];
    }

    function add_edge($_previous_as, $_as, $attrs = array()) {
        global $edges, $graph;

        $edge_array = array($_previous_as => $_as);
        if(!array_key_exists(gek($_previous_as, $_as), $edges)) {
            $attrs['splines'] = "true";
            $edge = array($edge_array, $attrs);
            $graph->addEdge($edge_array, $attrs);
            $edges[gek($_previous_as, $_as)] = $edge;
        } else if(array_key_exists('label', $attrs)) {
            $e = &$edges[gek($_previous_as, $_as)];

            $label_without_star = str_replace("*", "", $attrs['label']);
            $labels = explode("\r", $e[1]['label']); 
            if(!in_array($label_without_star."*", $labels)) {
                $tmp_labels = array();
                foreach($labels as $l) {
                    if(!startsWith($l, $label_without_star)) {
                        $labels[] = $l;
                    }
                }
                $labels = array_merge(array($attrs['label']), $tmp_labels);

                $cmp = function($a, $b) {
                    return endsWith($a, "*") ? -1 : 1;
                };
                usort($labels, $cmp);
                
                $label = escape(implode("\r", $labels));
                $e[1]['label'] = $label;
            }
        }
        return $edges[gek($_previous_as, $_as)];
    }

    function gek($_previous_as, $_as) { // Generate Edge Key
        return $_previous_as.'_'.$_as;
    }

    foreach($data as $host => $asmaps) {
        add_node($host, "#F5A9A9", strtoupper($host), "box");

        if($host == Config::NODE_NAME) {
            $node = add_node(Config::ASN, "#F5A9A9");
            $edge = add_edge(Config::ASN, $host, array('color' => "red", 'style' => "bold"));
        }
    }
    
    $previous_as = NULL;
    $hosts = array_keys($data);
    foreach($data as $host => $asmaps) {
        $first = true;

        foreach($asmaps as $asmap) {
            $previous_as = $host;
            $color = "#".dechex(rand(0, 16777215));

            $hop = false;
            $hop_label = "";
            foreach($asmap as $_as) {
                if($_as == $previous_as) {
                    continue;
                }

                if(!$hop) {
                    $hop = true;
                    if(!in_array($_as, $hosts)) {
                        $hop_label = $_as;
                        if($first) {
                            $hop_label = $hop_label."*";
                        }
                        continue;
                    } else {
                        $hop_label = "";
                    }
                }
                
                if(!strpos($_as, '.')) {
                    add_node($_as, ($first ? "#F5A9A9" : "white"));
                } else {
                    add_node($_as, ($first ? "#F5A9A9" : "white"), NULL, "box");
                }

                if($hop_label) {
                    $attrs = array('fontsize' => "7", 'label' => $hop_label);
                } else {
                    $attrs = array('fontsize' => "7");
                }

                $hop_label = "";

                if($first) {
                    $attrs['style'] = "bold";
                    $attrs['color'] = "red";
                } else if(!array_key_exists(gek($previous_as, $_as), $edges)
                        || $edges[gek($previous_as, $_as)][1]['color'] != "red") {
                    $attrs['style'] = "dashed";
                    $attrs['color'] = $color;
                }
                add_edge($previous_as, $_as, $attrs);

                $previous_as = $_as;
            }
            $first = false;
        }
    }

    return $graph->image("jpeg");
}

function startsWith($haystack, $needle) {
    // search backwards starting from haystack length characters from the end
    return $needle === "" || strrpos($haystack, $needle, -strlen($haystack)) !== FALSE;
}

function endsWith($haystack, $needle) {
    // search forward starting from end minus needle length characters
    return $needle === "" || (($temp = strlen($haystack) - strlen($needle)) >= 0
            && strpos($haystack, $needle, $temp) !== FALSE);
}