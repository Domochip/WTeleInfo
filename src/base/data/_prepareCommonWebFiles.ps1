#Script to prepare Common Web files
# - GZip of Common web files
# - Convert compressed web files to C++ header in PROGMEM

$listOfFiles="index.html","config0.html","configw.html","discover0.html","fw0.html","jquery-3.3.1-custo.min.js","pure-min.css","side-menu.css","side-menu.js","status0.html","statusw.html"


Import-Module -Name ((Split-Path -Path $MyInvocation.MyCommand.Path)+"\7Zip4Powershell-1.8.0\7Zip4PowerShell")

#function used in Application folder by _prepareWebFiles.ps1

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

#----- Real Execution of this script -----

$path=(Split-Path -Path $MyInvocation.MyCommand.Path)

Write-Host "--- Prepare Common Web Files ---"
Convert-FilesToCppHeader -Path $path -FileNames $listOfFiles
Write-Host ""