# WirelessTeleInfo
This project use an ESP8266 and some few components to push French TeleInfo to Jeedom.
Any other Home automation system can pull TeleInfo using HTTP GET Request too.
This project is related to French energy metering system.

# WirelessTeleInfo
Ce projet utilise un ESP8266 et quelques composants pour faire remonter les compteurs EDF au plugin Jeedom teleInfo.
Il peut aussi être utilisé avec n'importe quel systeme domotique pourvu qu'il puisse faire des requetes GET HTTP et interpréter le JSON.


## Construisez votre WirelessTeleInfo

Tous les fichiers necessaires sont dans le sous-dossier schematic et ont été conçu avec KiCAD.

### Schematic

![WirelessTeleInfo schematic](https://raw.github.com/J6B/Wireless-TeleInfo-ESP8266/master/img/schematic.jpg)

### PCB

![WirelessTeleInfo PCB](https://raw.github.com/J6B/Wireless-TeleInfo-ESP8266/master/img/pcb.jpg)

### Code/Compile
Les fichiers sources se trouve dans ce dépôt...
Pour compiler ce sketch, vous devez d'abord executer le script powerShell du dossier htmlToCppHeader.
Celui-ci compresse et converti en code (variables PROGMEM) les pages web se trouvant dans le dossier data



## Démarrage

### Premier Boot
Durant le premier boot, l'ESP démarre en mode Point d'Accès afin de le configurer

 - SSID : `WirelessTeleInfo`
 - Password : `PasswordTeleInfo`
 - IP : `192.168.4.1`

Connectez vous à ce réseau Wifi puis passer à la configuration.

### Configuration

WirelessTeleInfo possède plusieurs pages web vous permettant de l'administrer/configurer : 

 - `http://IP/config` vous permet de modifier la configuration : 

![config screenshot](https://raw.github.com/J6B/Wireless-TeleInfo-ESP8266/master/img/config.png)

	 **ssid & password** : Informations Wifi
	 **hostname** : nom de l'ESP sur le réseau

	 **Jeedom TeleInfo Plugin** : A cocher si vous souhaitez que ce module pousse les données à Jeedom (Sinon vous pourrez utiliser du requetage JSON)
	 **SSL/TLS** : à cocher si votre Jeedom utilise de l'HTTPS
	 **Hostname** : IP ou nom DNS de votre Jeedom
	 **ApiKey** : API Key de Jeedom
	 **TLS Fingerprint** : Si HTTPS, vous devez fournir l'empreinte du certificat Jeedom


 - `http://IP/status` vous retourne l'état du module :

![status screenshot](https://raw.github.com/J6B/Wireless-TeleInfo-ESP8266/master/img/status.png)


 - `http://IP/fw` vous permet de flasher une nouvelle version du firmware (OTA toujours disponible aussi) :

![firmware screenshot](https://raw.github.com/J6B/Wireless-TeleInfo-ESP8266/master/img/fw.png)

### Rescue Mode
Si vous perdez l'accès à votre WirelessTeleInfo, vous pouvez `le redémarrer` (couper puis remettre l'alimentation) ET durant les 5 premières secondes, `appuyer sur le bouton "Rescue Mode"`.
Celui-ci va demarrer avec la configuration par défaut (comme au Premier Boot).




## Utilisation

Pour un Jeedom avec le plugin TeleInfo, la configuration suffit!
Pour une autre solution domotique, vous trouverez dans ce chapitre les infos pour requeter les "étiquettes" de votre compteur EDF

### Base

Usage (les réponses sont au format JSON): 

 - `http://IP/getAllLabel` retourne la liste de toutes les étiquettes reçu du compteur
 - `http://IP/getLabel?name=PAPP` retourne l'étiquette souhaitée (ex : PAPP Puissance Apparente)

### Avec Jeedom (mode "universel")

TODO


## Other Sources / Autres Sources
This project uses a Library from hallard/LibTeleinfo repository.
This is a generic Teleinfo French Meter Measure Library
Charles Hallard made a very great job with this one.

I recommend you his blog too : https://hallard.me/

Ce projet utilise la librairie libTeleInfo provenant du dépôt hallard/LibTeleinfo.
C'est une librairie générique de gestion du protocole TeleInfo (ERDF - Enedis)
Charles Hallard a fait un excellent travail.

je vous recommande son blog : https://hallard.me/

## License
This project is placed under terms of Licence Creative Commons Attribution - Pas d’Utilisation Commerciale - Partage dans les Mêmes Conditions 4.0 International.

Ce Projet est placé sous les termes Licence Creative Commons Attribution - Pas d’Utilisation Commerciale - Partage dans les Mêmes Conditions 4.0 International.
