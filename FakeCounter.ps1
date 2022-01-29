[CmdletBinding()]
param (
    [Parameter(Mandatory)]
    [ValidateSet('Historique', 'Standard')]
    [string]
    $Mode,

    [Parameter(Mandatory)]
    [byte]
    $ComPortNumber
)

$STX = [char]0x02
$ETX = [char]0x03
$EOT = [char]0x04
$CR = [char]0x0D
$LF = [char]0x0A
$SP = [char]0x20
$HT = [char]0x09


function New-HistoriqueInfoGroupe {
    param (
        [Parameter(Mandatory)]
        [string]
        $Label,
        [Parameter(Mandatory)]
        [string]
        $Data
    )

    # return is {LF}LABEL{SP}DATA{SP}{CheckSum}{CR}
    # ie : {LF}PTEC{SP}TH..{SP}${CR}

    # calculate checksum on "LABEL{SP}DATA" part only
    $CheckSum = 0;
    foreach ($c in "$Label$SP$Data".ToCharArray()) {
        $CheckSum += [byte]$c
    }
    $CheckSum = [char](($CheckSum -band 0x3F) + 0x20)

    return "$LF$Label$SP$Data$SP$CheckSum$CR"
}

function New-HistoriqueTrame {
    param (
        [Parameter(Mandatory)]
        [Int]
        $CounterValue
    )

    $HistoriqueTrame = "$STX"
    
    $HistoriqueTrame += New-HistoriqueInfoGroupe -Label 'ADCO' -Data '041764312345'
    $HistoriqueTrame += New-HistoriqueInfoGroupe -Label 'OPTARIF' -Data 'BASE'
    $HistoriqueTrame += New-HistoriqueInfoGroupe -Label 'ISOUSC' -Data '60'
    $HistoriqueTrame += New-HistoriqueInfoGroupe -Label 'BASE' -Data "$CounterValue"
    $HistoriqueTrame += New-HistoriqueInfoGroupe -Label 'PTEC' -Data 'TH..'
    $HistoriqueTrame += New-HistoriqueInfoGroupe -Label 'IINST' -Data '003'
    $HistoriqueTrame += New-HistoriqueInfoGroupe -Label 'IMAX' -Data '090'
    $HistoriqueTrame += New-HistoriqueInfoGroupe -Label 'PAPP' -Data '00720'
    $HistoriqueTrame += New-HistoriqueInfoGroupe -Label 'HHPHC' -Data 'A'
    $HistoriqueTrame += New-HistoriqueInfoGroupe -Label 'MOTDETAT' -Data '000000'

    $HistoriqueTrame += "$ETX"

    return $HistoriqueTrame
}

function New-StandardInfoGroupe {
    param (
        [Parameter(Mandatory)]
        [string]
        $Label,
        [string]
        $Horodate,
        [Parameter(Mandatory)]
        [string]
        $Data
    )

    # return is {LF}LABEL{HT}DATA{HT}{CheckSum}{CR}
    # ie : {LF}LTARF{HT}BASE{HT}F{CR}
    # OR
    # return with Horodate is {LF}LABEL{HT}HORODATE{HT}DATA{HT}{CheckSum}{CR}
    # ie : {LF}SMAXSN{HT}H210310094942{HT}02058{HT}?{CR}

    # calculate checksum on "LABEL{HT}DATA{HT}" part only
    $CheckSum = 0;
    if ($Horodate -eq '') {
        foreach ($c in "$Label$HT$Data$HT".ToCharArray()) {
            $CheckSum += [byte]$c
        }
    }
    # calculate checksum on "LABEL{HT}HORODATE{HT}DATA{HT}" part only
    else {
        foreach ($c in "$Label$HT$Horodate$HT$Data$HT".ToCharArray()) {
            $CheckSum += [byte]$c
        }
    }
    $CheckSum = [char](($CheckSum -band 0x3F) + 0x20)

    if ($Horodate -eq '') {
        return "$LF$Label$HT$Data$HT$CheckSum$CR"
    }
    else {
        return "$LF$Label$HT$Horodate$HT$Data$HT$CheckSum$CR"
    }
}

function New-StandardTrame {
    param (
        [Parameter(Mandatory)]
        [Int]
        $CounterValue
    )

    $StandardTrame = "$STX"
    
    $StandardTrame += New-StandardInfoGroupe -Label 'ADSC' -Data '039999999999'
    $StandardTrame += New-StandardInfoGroupe -Label 'VTIC' -Data '02'
    $StandardTrame += New-StandardInfoGroupe -Label 'DATE' -Data 'H210310115551'
    $StandardTrame += New-StandardInfoGroupe -Label 'NGTF' -Data 'BASE'
    $StandardTrame += New-StandardInfoGroupe -Label 'LTARF' -Data 'BASE'
    $StandardTrame += New-StandardInfoGroupe -Label 'EAST' -Data "$CounterValue"
    $StandardTrame += New-StandardInfoGroupe -Label 'EASF01' -Data '022719001'
    $StandardTrame += New-StandardInfoGroupe -Label 'EASF02' -Data '000000000'
    $StandardTrame += New-StandardInfoGroupe -Label 'EASF03' -Data '000000000'
    $StandardTrame += New-StandardInfoGroupe -Label 'EASF04' -Data '000000000'
    $StandardTrame += New-StandardInfoGroupe -Label 'EASF05' -Data '000000000'
    $StandardTrame += New-StandardInfoGroupe -Label 'EASF06' -Data '000000000'
    $StandardTrame += New-StandardInfoGroupe -Label 'EASF07' -Data '000000000'
    $StandardTrame += New-StandardInfoGroupe -Label 'EASF08' -Data '000000000'
    $StandardTrame += New-StandardInfoGroupe -Label 'EASF09' -Data '000000000'
    $StandardTrame += New-StandardInfoGroupe -Label 'EASF10' -Data '000000000'
    $StandardTrame += New-StandardInfoGroupe -Label 'EASD01' -Data '012970758'
    $StandardTrame += New-StandardInfoGroupe -Label 'EASD02' -Data '004786755'
    $StandardTrame += New-StandardInfoGroupe -Label 'EASD03' -Data '001424492'
    $StandardTrame += New-StandardInfoGroupe -Label 'EASD04' -Data '003536996'
    $StandardTrame += New-StandardInfoGroupe -Label 'EAIT' -Data '000108022'
    $StandardTrame += New-StandardInfoGroupe -Label 'ERQ1' -Data '000026605'
    $StandardTrame += New-StandardInfoGroupe -Label 'ERQ2' -Data '000000105'
    $StandardTrame += New-StandardInfoGroupe -Label 'ERQ3' -Data '000070468'
    $StandardTrame += New-StandardInfoGroupe -Label 'ERQ4' -Data '010376751'
    $StandardTrame += New-StandardInfoGroupe -Label 'IRMS1' -Data '003'
    $StandardTrame += New-StandardInfoGroupe -Label 'URMS1' -Data '229'
    $StandardTrame += New-StandardInfoGroupe -Label 'PREF' -Data '06'
    $StandardTrame += New-StandardInfoGroupe -Label 'PCOUP' -Data '06'
    $StandardTrame += New-StandardInfoGroupe -Label 'SINSTS' -Data '00000'
    $StandardTrame += New-StandardInfoGroupe -Label 'SMAXSN' -Horodate 'H210310094942' -Data '02058'
    $StandardTrame += New-StandardInfoGroupe -Label 'SMAXSN-1' -Horodate 'H210309192738' -Data '04507'
    $StandardTrame += New-StandardInfoGroupe -Label 'SINSTI' -Data '00623'
    $StandardTrame += New-StandardInfoGroupe -Label 'SMAXIN' -Horodate 'H210310111438' -Data '00753'
    $StandardTrame += New-StandardInfoGroupe -Label 'SMAXIN-1' -Horodate 'H210309142429' -Data '02149'
    $StandardTrame += New-StandardInfoGroupe -Label 'CCASN' -Horodate ' H210310113000' -Data '00000'
    $StandardTrame += New-StandardInfoGroupe -Label 'CCASN-1' -Horodate 'H210310110000' -Data '00000'
    $StandardTrame += New-StandardInfoGroupe -Label 'CCAIN' -Horodate ' H210310113000' -Data '00308'
    $StandardTrame += New-StandardInfoGroupe -Label 'CCAIN-1' -Horodate 'H210310110000' -Data '00214'
    $StandardTrame += New-StandardInfoGroupe -Label 'UMOY1' -Horodate ' H210310115000' -Data '226'
    $StandardTrame += New-StandardInfoGroupe -Label 'STGE' -Data '003AC301'
    $StandardTrame += New-StandardInfoGroupe -Label 'MSG1' -Data 'PAS DE MESSAGE'
    $StandardTrame += New-StandardInfoGroupe -Label 'PRM' -Data '99999999999999'
    $StandardTrame += New-StandardInfoGroupe -Label 'RELAIS' -Data '000'
    $StandardTrame += New-StandardInfoGroupe -Label 'NTARF' -Data '01'
    $StandardTrame += New-StandardInfoGroupe -Label 'NJOURF' -Data '00'
    $StandardTrame += New-StandardInfoGroupe -Label 'NJOURF+1' -Data '00'
    $StandardTrame += New-StandardInfoGroupe -Label 'PJOURF+1' -Data '00008001 NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE'

    $StandardTrame += "$ETX"

    return $StandardTrame
}


$ComSpeed = if ($Mode -eq 'Historique') { 1200 } else { 9600 }

try {
    $port = new-Object System.IO.Ports.SerialPort "COM$ComPortNumber", $ComSpeed, Even, 7, one
    $port.open()
}
catch {
    Write-Error "We got an issue to open the Serial port : $($Error[0])"
    Exit
}


$lastSend = Get-Date

try {
    $Counter = 0
    while ( $true ) {
        if (((Get-Date) - $lastSend).TotalSeconds -gt 1.0) {
    
            Write-Host "Action $Counter"
            $Counter += 1
            $Trame = ''
            if ($Mode -eq 'Historique') {
                $Trame = New-HistoriqueTrame -CounterValue $Counter
            }
            else {
                $Trame = New-StandardTrame -CounterValue $Counter
            }

            $port.Write($Trame)

            $lastSend = Get-Date
        }
    }
}
catch {
    Write-Error "We got an issue while writing to the Serial port : $($Error[0])"
    $port.Close();
    Exit
}
finally {
    Write-Host 'You did a Ctrl-C?!'
    $port.Close();
}