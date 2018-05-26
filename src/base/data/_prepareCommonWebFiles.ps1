#Script to prepare Common Web files
# - GZip of Common web files
# - Convert compressed web files to C++ header in PROGMEM

$listOfFiles="jquery-3.2.1.min.js","pure-min.css","side-menu.css","side-menu.js"


Import-Module -Name ((Split-Path -Path $MyInvocation.MyCommand.Path)+"\7Zip4Powershell-1.8.0\7Zip4PowerShell")

#function used in Application folder by _prepareWebFiles.ps1
#It uses html file in base\data as template and then replace special tag with content in ps1 fil in application data folder
#then resulting webpage is stored in application data folder
function Merge-CustoWithTemplate
{
    PARAM (
        [string]$Path,
        [string]$FileName
    )

}


function Remove-StringLatinCharacters
{
    PARAM (
        [string]$String
    )
    [Text.Encoding]::ASCII.GetString([Text.Encoding]::GetEncoding("Cyrillic").GetBytes($String))
}
function Convert-BinaryToCppHeader
{
    PARAM (
        [string]$Path,
        [string]$FileName
    )

    $binaryPath=$Path+"\"+$FileName

    #if file already exists
    if(Test-Path ($binaryPath+".h")) {Remove-Item ($binaryPath+".h");}
    
    #open files
    [System.IO.FileStream] $binaryFile = [System.IO.File]::OpenRead($binaryPath);
    [System.IO.FileStream] $cppFile = [System.IO.File]::OpenWrite($binaryPath+".h");

    #Prepare start of code and write it
    $text+="const PROGMEM char "+ (Remove-StringLatinCharacters -String $FileName.Replace(' ','').Replace('.','').Replace('-','')) + "[] = {";
    $cppFile.Write([System.Text.UTF8Encoding]::new().GetBytes($text),0,$text.Length);


    $first=$true;

    while($binaryFile.Position -ne $binaryFile.Length){

        $text = if($first){""}else{","};
        $first=$false;
        $text+= "0x"+[System.BitConverter]::ToString($binaryFile.ReadByte());
        $cppFile.Write([System.Text.UTF8Encoding]::new().GetBytes($text),0,$text.Length);
    }

    $text="};";
    $cppFile.Write([System.Text.UTF8Encoding]::new().GetBytes($text),0,$text.Length);

    $binaryFile.Close();
    $cppFile.Close();
}

#Convert one file to header
#file is first GZipped then convert to header file (hex in PROGMEM)
function Convert-FileToCppHeader
{
    PARAM(
        [string]$Path,
        [string]$FileName
    )
    if($FileName -ne $null -and $FileName -ne "")
    {
        Compress-7Zip -Path $Path -Filter $FileName -ArchiveFileName ($Path+"\"+$FileName+".gz") -CompressionLevel Ultra -DisableRecursion
        Convert-BinaryToCppHeader -Path $Path -FileName ($FileName+".gz")
        Remove-Item ($Path+"\"+$FileName+".gz")
    }
}

#Convert multiple files into the same folder to cpp header
function Convert-FilesToCppHeader
{
    PARAM (
        [string]$Path,
        $FileNames
    )
    foreach($FileName in $FileNames)
    {
        if($FileName -ne $null -and $FileName -ne "")
        {
            Write-Host $FileName
            Convert-FileToCppHeader -Path $Path -FileName $FileName
        }
    }

}

#function that generate Custom web files from templates and customizations and then compresss and convert those to cpp header
#$templatePath : path of the folder conatining templates
#$filesAndCusto : structure that contains list of files to customize and for each of those contains customizations (replacement)
#$destinationPath : path of target folder for generated files
function Convert-TemplatesWithCustoToCppHeader{
    PARAM(
        [string]$templatePath,
        $filesAndCusto,
        [string]$destinationPath
    )

    #foreach file listed in $filesAndCusto
    foreach($file in $filesAndCusto.Keys)
    {
        #if filename is not empty
        if($file -ne $null -and $file -ne "")
        {
            Write-Host ($file+" (customized from template)")

            #load template
            $templateFile=Get-Content ($templatePath+"\"+$file)

            #foreach line of this template
            $templateFile | Foreach-Object {

                #copy line
                $line=$_
        
                #foreach replacement that need to be done for this file, apply it on this line
                foreach($replace in $filesAndCusto[$file].Keys)
                {
                    $line=$line.Replace("$"+$replace+"$",$filesAndCusto[$file][$replace])
                }
                
                #output line
                $line
            } | Set-Content ($destinationPath+"\"+$file) #save it to destination file

            #use existing function to convert generated customized file into cpp header
            Convert-FileToCppHeader -Path $destinationPath -FileName $file
        }
    }
}


#----- Real Execution of this script -----

$path=(Split-Path -Path $MyInvocation.MyCommand.Path)

Write-Host "--- Prepare Common Web Files ---"
Convert-FilesToCppHeader -Path $path -FileNames $listOfFiles
Write-Host ""