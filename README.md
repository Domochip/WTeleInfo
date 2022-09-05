# WTeleInfo
Ce projet utilise un D1 mini et quelques composants pour faire remonter les compteurs EDF à votre domotique.


## Construisez votre WTeleInfo

Tous les fichiers necessaires sont dans le sous-dossier schematic et ont été conçu avec KiCAD.

### Schematic

![WTeleInfo schematic](https://raw.github.com/Domochip/WTeleInfo/master/img/schematic.jpg) 

**Spécial Linky** : <u>Il faut remplacer **R2** par une **1.5K** et **C3** n'est **pas necessaire**</u>

### PCB

![WTeleInfo PCB](https://raw.github.com/Domochip/WTeleInfo/master/img/pcb.jpg)![WTeleInfo PCB2](https://raw.github.com/Domochip/WTeleInfo/master/img/pcb2.jpg)

### Imprimez votre boitier

Le projet du boitier (Fusion 360) se trouve dans le dossier `box` 
(Ainsi que les fichiers STL à imprimer directement) 

![WTeleInfo Box](https://raw.github.com/Domochip/WTeleInfo/master/img/box.jpg)

### Flash

Le firmware Tasmota-téléinfo est utilisé pour animer le D1Mini  
Vous pouvez le flasher directement depuis la page web "Install tasmota"
https://tasmota.github.io/install/

![WTeleInfo Flash](https://raw.github.com/Domochip/WTeleInfo/master/img/flash.png)

## Démarrage

### Configuration Tasmota

Vous devez appliquer ce Template :  
`{"NAME":"WTeleInfo","GPIO":[0,0,0,0,0,0,0,0,0,5152,0,0,0,0],"FLAG":0,"BASE":18,"CMND":"Module 0|TelePeriod 60"}`

Commande à executer, si vous possedez un compteur Linky en Mode Standard (nouveau protocole):  
`EnergyConfig Standard`

Pour plus de détails, veuillez vous référer à la documentation officielle :  
https://tasmota.github.io/docs/Teleinfo/

