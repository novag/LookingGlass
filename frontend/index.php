<?php require 'LookingGlass/Config.php'; ?>
<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="utf-8">
        <title><?= Config::LG_SERVER ?> - Looking Glass</title>
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <!-- IE6-8 support of HTML elements -->
        <!--[if lt IE 9]>
        <script src="http://html5shim.googlecode.com/svn/trunk/html5.js"></script>
        <![endif]-->
        <link href="assets/css/<?= Config::THEME ?>.min.css" rel="stylesheet">
        <link href="assets/css/bootstrap-responsive.min.css" rel="stylesheet">
    </head>
    <body>
        <div class="container">
            <header class="header nohighlight" id="overview">
                <div class="row">
                    <div class="span12">
                        <h1><a id="title" href="<?= Config::SITE_URL ?>"><?= Config::NODE_NAME ?></a></h1>
                    </div>
                </div>
            </header>
            <section id="information">
                <div class="row">
                    <div class="span12">
                        <div class="well">
                            <span id="legend">Network information</span><!-- IE/Safari dislike <legend> out of context -->
                            <?php if(Config::CONTACT): ?>
                                <p>Contact: <?= Config::CONTACT ?></p>
                            <?php endif; ?>
                            <?php if(Config::LOCATION): ?>
                                <p>Server Location: <b><?= Config::LOCATION ?></b></p>
                            <?php endif; ?>
                            <?php if(Config::TUNNEL_PROTO): ?>
                                <p>Tunnel Type: <?= Config::TUNNEL_PROTO ?></p>
                            <?php endif; ?>
                            <?php if(Config::ASN): ?>
                                <p>AS Number: <?= Config::ASN ?></p>
                            <?php endif; ?>
                            <?php if(Config::BGP_ENDPOINT): ?>
                                <p>BGP Endpoint: <?= Config::BGP_ENDPOINT ?></p>
                            <?php endif; ?>
                            <?php if(Config::REMOTE): ?>
                                <p>Remote: <?= Config::REMOTE ?></p>
                            <?php endif; ?>
                            <?php if(Config::IPV4_TEST || Config::IPV6_TEST): ?>
                                <div style="margin-left: 10px;">
                                    <?php if(Config::IPV4_TEST): ?>
                                        <p>Test IPv4: <?= Config::IPV4_TEST ?></p>
                                    <?php endif; ?>
                                    <?php if(Config::IPV6_TEST): ?>
                                        <p>Test IPv6: <?= Config::IPV6_TEST ?></p>
                                    <?php endif; ?>
                                </div>
                            <?php endif; ?>
                            <p>Your IP Address: <b><a href="#tests" id="userip"><?= $_SERVER['REMOTE_ADDR'] ?></a></b></p>
                        </div>
                    </div>
                </div>
            </section>
            <section id="tests">
                <div class="row">
                    <div class="span12">
                        <form class="well form-inline" id="networktest" action="#results" method="post">
                            <fieldset>
                                <span id="legend">Network tests</span>
                                <div id="argerror" class="control-group">
                                    <div class="controls">
                                        <input id="arg" name="arg" type="text" class="input-large" placeholder="Host, IP, AS or empty for stats">
                                    </div>
                                </div>
                                <select id="cmd" name="cmd" class="input-medium" style="margin-left: 5px;">
                                    <option value="whois">WHOIS</option>
                                    <option value="ping" selected>PING</option>
                                    <option value="ping6">PING6</option>
                                    <option value="traceroute">TRACE</option>
                                    <option value="traceroute6">TRACE6</option>
                                    <option value="summary">SUMMARY4</option>
                                    <option value="summary6">SUMMARY6</option>
                                    <option value="route">ROUTE4</option>
                                    <option value="route6">ROUTE6</option>
                                    <option value="bgpmap">ROUTE (bgpmap)</option>
                                    <option value="bgpmap6">ROUTE6 (bgpmap)</option>
                                    <option value="as">AS</option>
                                    <option value="as6">AS6</option>
                                </select>
                                <button type="submit" id="submit" name="submit" class="btn btn-primary" style="margin-left: 10px;">Run Test</button>
                            </fieldset>
                        </form>
                    </div>
                </div>
            </section>
            <section id="results" style="display:none">
                <div class="row">
                    <div class="span12">
                        <div class="well">
                            <span id="legend">Results</span>
                        <pre id="response" style="display:none"></pre>
                    </div>
                </div>
            </div>
        </section>
        <footer class="footer nohighlight">
            <p class="pull-right">
                <a href="#">Back to top</a>
            </p>
            <p>Powered by <a href="http://github.com/novag/LookingGlass">CLG</a>
            <p>Theme based on <a href="http://github.com/telephone/LookingGlass">LookingGlass</a></p>
        </footer>
    </div>
    <script src="//code.jquery.com/jquery-1.11.3.min.js"></script>
    <script src="assets/js/XMLHttpRequest.min.js"></script>
    <script src="assets/js/LookingGlass.min.js"></script>
</body>
</html>
