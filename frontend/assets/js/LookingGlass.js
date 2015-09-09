$(document).ready(function() {
    // onclick, set user IP to input value
    $('#userip').click(function() {
        $('#arg').val($('#userip').text());
    });
    // form submit
    $('#networktest').submit(function() {
        // define vars
        var cmd = $('#cmd').val();
        var arg = $('#arg').val();
        var data = 'cmd=' + cmd + '&arg=' + arg;
        // quick validation
        if (arg == '' && !(cmd == 'summary' || cmd == 'summary6' || cmd == 'route' || cmd == 'route6')) {
            $('#argerror').addClass('error');
            return false;
        } else {
            $('#argerror').removeClass('error');
        }
        // submit form
        // disable submit button + blank response
        $('#submit').attr('disabled', 'true').text('Loading...');
        $('#response').html();

        // call async request
        var xhr = new XMLHttpRequest();
        xhr.open('GET', 'ajax.php?' + data, true);
        xhr.send(null);
        var timer;
        timer = window.setInterval(function() {
            // on completion
            if (xhr.readyState == XMLHttpRequest.DONE) {
                window.clearTimeout(timer);
                if (!cmd.startsWith("bgpmap"))
                    $('#submit').removeAttr('disabled').text('Run Test');
            }

            // show/hide results
            $('#argerror').removeClass('error');
            $('#response, #results').show();

            // output response
            if (xhr.responseText == 'Unauthorized request') {
                $('#results').hide();
                $('#argerror').addClass('error');
            } else {
                $('#response').html(xhr.responseText.replace(/<br \/> +/g, '<br />'));
            }

            if (cmd.startsWith("bgpmap")) {
                var bgpmap = document.getElementById('bgpmap');
                bgpmap.onload = function() {
                    $('#submit').removeAttr('disabled').text('Run Test');      
                };
            }
        }, 500);

        // cancel default behavior
        return false;
    });
    // Preset command
    var cmd = window.location.hash.substring(1).split('/');
    if ($('#cmd option[value="' + cmd[0] + '"]').length != 0) {
        $('#cmd').val(cmd[0]);
        $('#arg').val(cmd[1]);
        $('#networktest').submit();
    }
});