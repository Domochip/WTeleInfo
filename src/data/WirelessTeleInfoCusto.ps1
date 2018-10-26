#Script to prepare Web files
# - GZip of web files
# - and convert compressed web files to C++ header in PROGMEM

#List here web files specific to this project/application
$specificFiles="config1.html","discover1.html","fw1.html","status1.html"

#call script that prepare Common Web Files and contain compression/Convert/Merge functions
. ..\base\data\_prepareCommonWebFiles.ps1

$path=(Split-Path -Path $MyInvocation.MyCommand.Path)

Write-Host "--- Prepare Application Web Files ---"
Convert-FilesToCppHeader -Path $path -FileNames $specificFiles
Write-Host ""