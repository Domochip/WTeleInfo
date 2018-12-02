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

![WirelessTeleInfo schematic](https://raw.github.com/Domochip/Wireless-TeleInfo-ESP8266/master/img/schematic.jpg)

### PCB

![WirelessTeleInfo PCB](https://raw.github.com/Domochip/Wireless-TeleInfo-ESP8266/master/img/pcb.jpg)

### Code/Compile
Pour compiler ce sketch, vous devez utiliser PlatformIO


## Démarrage

### Premier Boot
Durant le premier boot, l'ESP démarre en mode Point d'Accès afin de le configurer

 - SSID : `WirelessTeleInfoXXXX`
 - Password : `PasswordTeleInfo`
 - IP : `192.168.4.1`

Connectez vous à ce réseau Wifi puis passer à la configuration.

### Configuration

WirelessTeleInfo possède plusieurs pages web vous permettant de l'administrer/configurer : 

 - `http://IP/status` vous retourne l'état du module :

![status screenshot](https://raw.github.com/Domochip/Wireless-TeleInfo-ESP8266/master/img/status.png)

 - `Config` vous permet de modifier la configuration : 

![config screenshot](https://raw.github.com/Domochip/Wireless-TeleInfo-ESP8266/master/img/config.png)

- **ssid & password** : Informations Wifi
- **hostname** : nom de l'ESP sur le réseau
- **IP,GW,NetMask,DNS1&2** : configuration IP fixe 

![config2 screenshot](https://raw.github.com/Domochip/Wireless-TeleInfo-ESP8266/master/img/config2.png)

- **HA Type** : None (HA make request to the device) or HTTP (device make GET request to HA) or MQTT
- **SSL/TLS** : check if your MQTT server enforce SSL/TLS
- **Hostname** : IP or DNS name of your MQTT server
- **Port** : MQTT Port
- **Username/Password** : MQTT Username/Password (both are optionnal)
- **Base Topic** : Prefix of the topic


 - `Firmware` vous permet de flasher une nouvelle version du firmware :

![firmware screenshot](https://raw.github.com/Domochip/Wireless-TeleInfo-ESP8266/master/img/fw.png)

- `Discover` vous permet de découvrir tous les device Domochip sur votre réseau :

![discover screenshot](https://raw.github.com/Domochip/Wireless-DS18B20-Bus/master/img/discover.png)


### Rescue Mode
Si vous perdez l'accès à votre WirelessTeleInfo, vous pouvez `le redémarrer` (couper puis remettre l'alimentation) ET durant les 5 premières secondes, `appuyer une fois sur le bouton "Rescue Mode"`.
Celui-ci va demarrer avec la configuration par défaut (comme au Premier Boot).




## Utilisation

### Base

De Base, Le module peut-être requeté en HTTP GET
Usage (les réponses sont au format JSON): 

 - `http://IP/getAllLabel` retourne la liste de toutes les étiquettes reçu du compteur
 - `http://IP/getLabel?name=PAPP` retourne l'étiquette souhaitée (ex : PAPP Puissance Apparente)

### Envoi HTTP

Le module peut envoyer les informations en faisant des requetes HTTP GET à votre solution domotique
Sur la page de configuration, choisir le mode HTTP et le type Generic.
Il vous faudra construire un pattern qui corresponde au attente de votre solution domotique
ex : `http$tls$://$host$/api/pushValue?id=$label$&value=$val$`

### Avec Jeedom (Plugin TeleInfo)

Sur la page de configuration, choisir le mode HTTP et Jeedom Teleinfo Plugin en Type!

### Envoi MQTT

Le module peut "publier" les informations à serveur MQTT


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
