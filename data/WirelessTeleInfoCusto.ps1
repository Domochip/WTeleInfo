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
            <dt>Jeedom TeleInfo Plugin</dt>
            <dd>Last Request : <span id="lr"></span></dd>
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
        <h2 class="content-subhead">Jeedom<span id="l1"><h6 style="display:inline"><b> Loading...</b></h6></span></h2>
        <form class="pure-form pure-form-aligned" id='f1'>
            <fieldset>

    <div class="pure-control-group">
        <label for="je" class="pure-checkbox">Jeedom Upload</label>
        <input id='je' name='je' type="checkbox">
    </div>
    <div id='j' style='display:none'>
        <div class="pure-control-group">
            <label for="jt">SSL/TLS</label>
            <input type='checkbox' id='jt' name='jt'>
        </div>
        <div class="pure-control-group">
            <label for="jh">Hostname</label>
            <input type='text' id='jh' name='jh' maxlength='64' pattern='[A-Za-z0-9-.]+' size='50' title='DNS name or IP of the Jeedom server'>
            <span class="pure-form-message-inline">(Jeedom Hostname should match with certificate name if SSL/TLS is enabled)</span>
        </div>
        <div class="pure-control-group">
            <label for="ja">ApiKey</label>
            <input type='password' id='ja' name='ja' maxlength='48' pattern='[A-Za-z0-9-.]+' size=50 title='APIKey from Jeedom configuration webpage'>
        </div>
        <div id='jf'>
            <div class="pure-control-group">
                <label for="jfp">TLS FingerPrint</label>
                <input type='text' id='jfp' name='jfp' maxlength='59' pattern='^([0-9A-Fa-f]{2}[ :-]*){19}([0-9A-Fa-f]{2})$' size='65'>
                <span class="pure-form-message-inline">(separators are : &lt;none&gt;,&lt;space&gt;,:,-)</span>
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
        function onJEChange(){
            if($("#je").prop("checked")) $("#j").show();
            else $("#j").hide();
        };

        $("#je").change(onJEChange);

        function onJTChange(){
            if($("#jt").prop("checked")) $("#jf").show();
            else $("#jf").hide();
        };
        $("#jt").change(onJTChange);

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
. ..\src\data\_prepareCommonWebFiles.ps1

$path=(Split-Path -Path $MyInvocation.MyCommand.Path)
$templatePath=($path+"\..\src\data")

Write-Host "--- Prepare Application Web Files ---"
Convert-TemplatesWithCustoToCppHeader -templatePath $templatePath -filesAndCusto $templatesWithCustoFiles -destinationPath $path
Convert-FilesToCppHeader -Path $path -FileNames $specificFiles
Write-Host ""