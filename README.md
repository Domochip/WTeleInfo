# WirelessTeleInfo
This project use a D1 Mini and some few components to push French TeleInfo to Jeedom.
Any other Home automation system can pull TeleInfo using HTTP GET Request too.
This project is related to French energy metering system.
MQTT publish has been added to improve compatibility and performance.

# WirelessTeleInfo
Ce projet utilise un D1 mini et quelques composants pour faire remonter les compteurs EDF au plugin Jeedom teleInfo.
Il peut aussi être utilisé avec n'importe quel systeme domotique pourvu qu'il puisse faire des requetes GET HTTP et interpréter le JSON.
Le protocole MQTT a été ajouté afin d'améliorer les performances et la compatibilité avec d'autres systèmes.


## Construisez votre WirelessTeleInfo

Tous les fichiers necessaires sont dans le sous-dossier schematic et ont été conçu avec KiCAD.

### Schematic

![WirelessTeleInfo schematic](https://raw.github.com/Domochip/WirelessTeleInfo/master/img/schematic.jpg)

### PCB

![WirelessTeleInfo PCB](https://raw.github.com/Domochip/WirelessTeleInfo/master/img/pcb.jpg)![WirelessTeleInfo PCB2](https://raw.github.com/Domochip/WirelessTeleInfo/master/img/pcb2.jpg)

### Code/Compile
Pour compiler ce sketch, vous devez utiliser PlatformIO

### Imprimez votre boitier

Le projet du boitier (Fusion 360) se trouve dans le dossier `box` 

![WirelessTeleInfo Box](https://raw.github.com/Domochip/WirelessTeleInfo/master/img/box.jpg)


## Démarrage

### Premier Boot
Durant le premier boot, l'ESP démarre en mode Point d'Accès afin de le configurer

 - SSID : `WTeleInfoXXXX`
 - Password : `PasswordTeleInfo`
 - IP : `192.168.4.1`

Connectez vous à ce réseau Wifi puis passer à la configuration.

### Configuration

WirelessTeleInfo possède plusieurs pages web vous permettant de l'administrer/configurer : 

 - `Status` vous retourne l'état du module :

![status screenshot](https://raw.github.com/Domochip/WirelessTeleInfo/master/img/status.png)

 - `Config` vous permet de modifier la configuration : 

![config screenshot](https://raw.github.com/Domochip/WirelessTeleInfo/master/img/config.png)

- **ssid & password** : Informations Wifi
- **hostname** : nom de l'ESP sur le réseau
- **IP,GW,NetMask,DNS1&2** : configuration IP fixe 

![config2 screenshot](https://raw.github.com/Domochip/WirelessTeleInfo/master/img/config2.png)

- **Type** : Disabled (votre domotique vient requêter le module) / HTTP (le module execute des requêtes HTTP GET) / MQTT (le module publie les compteurs sur un topic MQTT)
- **SSL/TLS** : à cocher si votre serveur HTTP ou broker MQTT utilise du TLS
- **Hostname** : IP ou nom DNS du serveur
- **Port** : Port MQTT
- **Username/Password** : nom d'utilisateur/mot de passe MQTT (tous deux optionnels)
- **Base Topic** : Préfix du topic MQTT


 - `Firmware` vous permet de flasher une nouvelle version du firmware :

![firmware screenshot](https://raw.github.com/Domochip/WirelessTeleInfo/master/img/fw.png)

- `Discover` vous permet de découvrir tous les modules Domochip sur votre réseau :

![discover screenshot](https://raw.github.com/Domochip/Wireless-DS18B20-Bus/master/img/discover.png)


## Utilisation

### Base

De Base, Le module peut-être requêté en HTTP GET

Usage (les réponses sont au format JSON): 

 - `http://IP/getAllLabel` retourne la liste de toutes les étiquettes reçu du compteur
 - `http://IP/getLabel?name=PAPP` retourne l'étiquette souhaitée (ex : PAPP Puissance Apparente)

### Envoi HTTP

Le module peut envoyer les informations en faisant des requêtes HTTP GET à votre solution domotique
Sur la page de configuration, choisir le mode HTTP et le type Generic.
Il vous faudra construire un pattern qui corresponde aux attentes de votre solution domotique
ex : `http$tls$://$host$/api/pushValue?id=$label$&value=$val$`

### Avec Jeedom (Plugin TeleInfo)

Sur la page de configuration, choisir le mode HTTP et Jeedom Teleinfo Plugin en Type

### Envoi MQTT

Sur la page de configuration, choisir le mode MQTT et renseigner un topic de base
ex : $model$

Seront ainsi publié :
- WTeleInfo/XXXXXXXXX/ADCO
- WTeleInfo/XXXXXXXXX/HCHC
- WTeleInfo/XXXXXXXXX/PAPP
- WTeleInfo/XXXXXXXXX/HCHP
- ...


## Other Sources / Autres Sources
This project uses a Library from hallard/LibTeleinfo repository.
This is a generic Teleinfo French Meter Measure Library
Charles Hallard made a very great job with this one.

I recommend you his blog : https://hallard.me/  
And his company website : https://ch2i.eu/en

Ce projet utilise la librairie libTeleInfo provenant du dépôt hallard/LibTeleinfo.
C'est une librairie générique de gestion du protocole TeleInfo (ERDF - Enedis)
Charles Hallard a fait un excellent travail.

je vous recommande son blog : https://hallard.me/  
et son site professionnel : https://ch2i.eu/

## License
This project is placed under terms of Licence Creative Commons Attribution - Pas d’Utilisation Commerciale - Partage dans les Mêmes Conditions 4.0 International.

Ce Projet est placé sous les termes Licence Creative Commons Attribution - Pas d’Utilisation Commerciale - Partage dans les Mêmes Conditions 4.0 International.
