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
        <h2 class="content-subhead">TeleInfo <span id="l2"><h6 style="display:inline"><b> Loading...</b></h6></span></h2>
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
            $("#l2").fadeOut();
        })
        .fail(function(){
            $("#l2").html('<h4 style="display:inline;color:red;"><b> Failed</b></h4>');
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
'@
        ;
        HTMLFillinConfigForm=@'
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