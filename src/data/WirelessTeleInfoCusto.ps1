#Script to prepare Web files
# - Integrate customization of the application
# - GZip of resultant web files
# - and finally convert compressed web files to C++ header in PROGMEM

#List here web files specific to this project/application
$specificFiles="",""

#list here files that need to be merged with Common templates
$applicationName="WirelessTeleInfo"
$shortApplicationName="WTeleInfo"
$templatesWithCustoFiles=@{
    #---Status.html---
    "status.html"=@{
        ApplicationName=$applicationName
        ;
        ShortApplicationName=$shortApplicationName
        ;
        HTMLContent=@'
        <h2 class="content-subhead">TeleInfo<span id="l3"><h6 style="display:inline"><b> Loading...</b></h6></span></h2>
        Connected ADCO : <span id="a"></span><br>
        <dl>
            <dt>Home Automation Upload</dt>
            <dd>Last Request Result : <span id="lrr"></span></dd>
        </dl>
'@
        ;
        HTMLScriptInReady=@'
        $.getJSON("/gs1", function(GS1){

            $.each(GS1,function(k,v){
                $('#'+k).html(v);
            })
            $("#l3").fadeOut();
        })
        .fail(function(){
            $("#l3").html('<h4 style="display:inline;color:red;"><b> Failed</b></h4>');
        });
'@
    }
    ;
    #---config.html---
    "config.html"=@{
        ApplicationName=$applicationName
        ;
        ShortApplicationName=$shortApplicationName
        ;
        HTMLContent=@'
        <h2 class="content-subhead">TeleInfo<span id="l1"><h6 style="display:inline"><b> Loading...</b></h6></span></h2>
        <form class="pure-form pure-form-aligned" id='f1'>
            <fieldset>

        <h3 class="content-subhead">Home Automation  <span id="hai" name="hai" class="infotip">?</span></h3>
        <div class="infotipdiv"  id="haidiv" name="haidiv" style="display:none;">
            HTTP GET request can be send to this device : <br>
            <b>http://$ip$/getAllLabel</b> : Get list of all labels (JSON List)<br>
            <b>http://$ip$/getLabel?name=$labelName$</b> : Get last value of a label, ex : PAPP (JSON)<br>
        </div>

        <div class="pure-control-group">
            <label for="haproto">Type</label>
            <select id='haproto' name='haproto'>
                <option value="0">Disabled</option>
                <option value="1">HTTP</option>
                <option value="2">MQTT</option>
            </select>
        </div>


        <div id='hae' style='display:none'>

            <div class="pure-control-group">
                <label for="hatls">SSL/TLS</label>
                <input type='checkbox' id='hatls' name='hatls'>
            </div>
            <div class="pure-control-group">
                <label for="hahost">Hostname</label>
                <input type='text' id='hahost' name='hahost' maxlength='64' pattern='[A-Za-z0-9-.]+' size='50' placeholder="IP or DNS Name">
                <span class="pure-form-message-inline">(Hostname should match with certificate name if SSL/TLS is enabled)</span>
            </div>


            <div id='hahe'>

                <div class="pure-control-group">
                    <label for="hahtype">Type</label>
                    <select id='hahtype' name='hahtype'>
                        <option value="0">Generic</option>
                        <option value="1">Jeedom TeleInfo Plugin</option>
                    </select>
                </div>

                <div id='hahtlse'>
                    <div class="pure-control-group">
                        <label for="hahfp">TLS FingerPrint</label>
                        <input type='text' id='hahfp' name='hahfp' maxlength='59' pattern='^([0-9A-Fa-f]{2}[ :-]*){19}([0-9A-Fa-f]{2})$' size='65'>
                        <span class="pure-form-message-inline">(separators are : &lt;none&gt;,&lt;space&gt;,:,-)</span>
                    </div>
                </div>

                <div id='hah0'>
                    <div class="pure-control-group">
                        <label for="hahgup">URI Pattern</label>
                        <input type='text' id='hahgup' name='hahgup' maxlength='150' size='65' placeholder="Used to generate requests URI">
                        <span id="hahgupi" name="hahgupi" class="infotip">?</span>
                    </div>

                    <div class="pure-control-group" id="hahgupidiv" name="hahgupidiv" style="display:none;">
                        <label></label>
                        <div class="infotipdiv">
                            URI Pattern placeholders : <br>
                            <b>$tls$</b> : replaced by 's' for HTTPS connection<br>
                            <b>$host$</b> : replaced by hostname<br>
                            <b>$adco$</b> : replaced the counter Serial Number<br>
                            <b>$label$</b> : replaced the current label<br>
                            <b>$val$</b> : replaced the value of the current label<br>
                            Ex : http<b>$tls$</b>://<b>$host$</b>/api/pushValue?id=<b>$label$</b>&amp;value=<b>$val$</b>
                        </div>
                    </div>
                </div>

                <div id='hah1'>
                    <div class="pure-control-group">
                        <label for="hahjak">ApiKey</label>
                        <input type='password' id='hahjak' name='hahjak' maxlength='48' pattern='[A-Za-z0-9-.]+' size=50 title='APIKey from Jeedom configuration webpage'>
                    </div>
                </div>

            </div>

            <div id='hame'>

                <div class="pure-control-group">
                    <label for="hamtype">Type</label>
                    <select id='hamtype' name='hamtype'>
                        <option value="0">Generic</option>
                    </select>
                    <span id="hamtype0i" name="hamtype0i" class="infotip">?</span>
                </div>
                <div class="pure-control-group" id="hamtype0div" name="hamtype0div" style="display:none;">
                    <label></label>
                    <div class="infotipdiv">
                        Published topics : <br>
                        <b>/$label$</b> : all labels received from the counter<br>
                    </div>
                </div>

                <div class="pure-control-group">
                    <label for="hamport">Port</label>
                    <input type='number' id='hamport' name='hamport' min='1' max='65535'>
                </div>
                <div class="pure-control-group">
                    <label for="hamu">Username</label>
                    <input type='text' id='hamu' name='hamu' maxlength='64' placeholder="optional">
                </div>
                <div class="pure-control-group">
                    <label for="hamp">Password</label>
                    <input type='password' id='hamp' name='hamp' maxlength='64' placeholder="optional">
                </div>

                <div id='hamgbte'>
                    <div class="pure-control-group">
                        <label for="hamgbt">Base Topic</label>
                        <input type='text' id='hamgbt' name='hamgbt' maxlength='64'>
                        <span id="hamgbti" name="hamgbti" class="infotip">?</span>
                    </div>

                    <div class="pure-control-group" id="hamgbtidiv" name="hamgbtidiv" style="display:none;">
                        <label></label>
                        <div class="infotipdiv">
                            Base Topic placeholders : <br>
                            <b>$adco$</b> : serial number of the counter<br>
                            <b>$sn$</b> : Serial Number of this device<br>
                            <b>$mac$</b> : WiFi MAC address of this device<br>
                            <b>$model$</b> : Model of this device<br>
                            Ex : DomoChip/<b>$adco$</b>
                        </div>
                    </div>
                </div>
            </div>
        </div>

                <div class="pure-controls">
                    <input type='submit' value='Save' class="pure-button pure-button-primary" disabled>
                </div>
            </fieldset>
        </form>
        <span id='r1'></span>
'@
        ;
        HTMLScript=@'

        $("#hai").click(function(){$("#haidiv").slideToggle(300);});

        $("#haproto").change(function(){
            switch($("#haproto").val()){
                case "0":
                    $("#hae").hide();
                    break;
                case "1":
                    $("#hae").show();
                    $("#hahe").show();
                    $("#hame").hide();
                    break;
                case "2":
                    $("#hae").show();
                    $("#hahe").hide();
                    $("#hame").show();
                    break;
            }
        });

        $("#hahtype").change(function(){
            switch($("#hahtype").val()){
                case "0":
                    $("#hah0").show();
                    $("#hah1").hide();
                    break;
                case "1":
                    $("#hah0").hide();
                    $("#hah1").show();
                    break;
            }
        });
        $("#hahgupi").click(function(){$("#hahgupidiv").slideToggle(300);});

        $("#hamtype").change(function(){
            switch($("#hamtype").val()){
                case "0":
                    $("#hamgbte").show();
                    break;
            }
        });
        $("#hamtype0i").click(function(){$("#hamtype0div").slideToggle(300);});
        $("#hamgbti").click(function(){$("#hamgbtidiv").slideToggle(300);});

        $("#hatls").change(function(){
            if($("#hatls").prop("checked")){
                $("#hahtlse").show();
                $("#hamport").val(8883);
            }
            else{
                $("#hahtlse").hide();
                $("#hamport").val(1883);
            }
        });

        $("#f1").submit(function(event){
            $("#r1").html("Saving Configuration...");
            $.post("/sc1",$("#f1").serialize(),function(){ 
                $("#f1").hide();
                var reload5sec=document.createElement('script');
                reload5sec.text='var count=4;var cdi=setInterval(function(){$("#cd").text(count);if(!count){clearInterval(cdi);location.reload();}count--;},1000);';
                $('#r1').html('<h3><b>Configuration saved <span style="color: green;">successfully</span>. System is restarting now.</b></h3>This page will be reloaded in <span id="cd">5</span>sec.').append(reload5sec);
            }).fail(function(){
                $('#r1').html('<h3><b>Configuration <span style="color: red;">error</span>.</b></h3>');
            });
            event.preventDefault();
        });
'@
        ;
        HTMLScriptInReady=@'
        $.getJSON("/gc1", function(GC1){

            $.each(GC1,function(k,v){

                if($('#'+k).prop('type')!='checkbox') $('#'+k).val(v);
                else $('#'+k).prop("checked",v);

                $('#'+k).trigger("change");
            })

            $("input[type=submit]",$("#f1")).prop("disabled",false);
            $("#l1").fadeOut();
        })
        .fail(function(){
            $("#l1").html('<h6 style="display:inline;color:red;"><b> Failed</b></h6>');
        });
'@
    }
    ;
    #---fw.html---
    "fw.html"=@{
        ApplicationName=$applicationName
        ;
        ShortApplicationName=$shortApplicationName
    }
    ;
    #---discover.html---
    "discover.html"=@{
        ApplicationName=$applicationName
        ;
        ShortApplicationName=$shortApplicationName
    }
}

#call script that prepare Common Web Files and contain compression/Convert/Merge functions
. ..\base\data\_prepareCommonWebFiles.ps1

$path=(Split-Path -Path $MyInvocation.MyCommand.Path)
$templatePath=($path+"\..\base\data")

Write-Host "--- Prepare Application Web Files ---"
Convert-TemplatesWithCustoToCppHeader -templatePath $templatePath -filesAndCusto $templatesWithCustoFiles -destinationPath $path
Convert-FilesToCppHeader -Path $path -FileNames $specificFiles
Write-Host ""