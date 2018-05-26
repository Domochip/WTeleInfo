#include "WirelessTeleInfo.h"

//Please, have a look at Main.h for information and configuration of Arduino project

//------------------------------------------
// TeleInfo CallBack function (when value change occured)
void WebTeleInfo::tinfoUpdatedFrame(ValueList *me)
{

  //if pointer is null
  if (!me)
    return;

  if (!_ADCO[0])
  {
    //Find ADCO
    ValueList *adcoLookup = me;
    while (adcoLookup->next)
    {
      adcoLookup = adcoLookup->next;
      if (!strcmp_P(adcoLookup->name, PSTR("ADCO")))
        strcpy(_ADCO, adcoLookup->value);
    }
  }

  //if Home Automation upload not enabled then return
  if (_ha.protocol == HA_PROTO_DISABLED)
    return;

  //----- HTTP Protocol configured -----
  if (_ha.protocol == HA_PROTO_HTTP)
  {
    String completeURI;

    //Build request
    switch (_ha.http.type)
    {
    case HA_HTTP_GENERIC:
      completeURI = _ha.http.generic.uriPattern;

      //Replace placeholders
      if (completeURI.indexOf(F("$tls$")) != -1)
        completeURI.replace(F("$tls$"), _ha.tls ? "s" : "");

      if (completeURI.indexOf(F("$host$")) != -1)
        completeURI.replace(F("$host$"), _ha.hostname);

      if (completeURI.indexOf(F("$apikey$")) != -1)
        completeURI.replace(F("$apikey$"), _ha.http.jeedom.apiKey);

      if (completeURI.indexOf(F("$adco$")) != -1)
        completeURI.replace(F("$adco$"), _ADCO);

      _haSendResult = true;

      //go through labels to look for new Value
      while (me->next && _haSendResult)
      {
        me = me->next;
        // Add only new or updated values (And name is not ADCO (it will be added))
        if ((me->flags & (TINFO_FLAGS_ADDED | TINFO_FLAGS_UPDATED)) && strcmp_P(me->name, PSTR("ADCO")) != 0 && strcmp_P(me->name, PSTR("MOTDETAT")) != 0)
        {

          String sendURI = completeURI;

          if (sendURI.indexOf(F("$label$")) != -1)
            sendURI.replace(F("$label$"), me->name);

          if (sendURI.indexOf(F("$val$")) != -1)
            sendURI.replace(F("$val$"), me->value);

          //create HTTP request
          HTTPClient http;

          //if tls is enabled or not, we need to provide certificate fingerPrint
          if (!_ha.tls)
            http.begin(sendURI);
          else
          {
            char fpStr[41];
            http.begin(sendURI, Utils::FingerPrintA2S(fpStr, _ha.http.fingerPrint));
          }

          _haSendResult = (http.GET() == 200);
          http.end();
        }
      }

      break;
    case HA_HTTP_JEEDOM_TELEINFO:
      //if we didn't find ADCO then we can't send stuff so return
      if (!_ADCO[0])
        return;

      completeURI = F("http$tls$://$host$/plugins/teleinfo/core/php/jeeTeleinfo.php?api=$apikey$&ADCO=$adco$");

      //initialize data list to send
      _httpJeedomRequest = "";

      //go through labels to look for new Value
      while (me->next)
      {
        me = me->next;
        // Add only new or updated values (And name is not ADCO (it will be added))
        if ((me->flags & (TINFO_FLAGS_ADDED | TINFO_FLAGS_UPDATED)) && strcmp_P(me->name, PSTR("ADCO")) != 0 && strcmp_P(me->name, PSTR("MOTDETAT")) != 0)
        {
          _httpJeedomRequest += String("&") + me->name + '=' + me->value;
        }
      }
      //do we need to send data
      if (_httpJeedomRequest == "")
        return;

      completeURI += _httpJeedomRequest;

      //Replace placeholders
      if (completeURI.indexOf(F("$tls$")) != -1)
        completeURI.replace(F("$tls$"), _ha.tls ? "s" : "");

      if (completeURI.indexOf(F("$host$")) != -1)
        completeURI.replace(F("$host$"), _ha.hostname);

      if (completeURI.indexOf(F("$apikey$")) != -1)
        completeURI.replace(F("$apikey$"), _ha.http.jeedom.apiKey);

      if (completeURI.indexOf(F("$adco$")) != -1)
        completeURI.replace(F("$adco$"), _ADCO);

      //create HTTP request
      HTTPClient http;

      //if tls is enabled or not, we need to provide certificate fingerPrint
      if (!_ha.tls)
        http.begin(completeURI);
      else
      {
        char fpStr[41];
        http.begin(completeURI, Utils::FingerPrintA2S(fpStr, _ha.http.fingerPrint));
      }

      _haSendResult = (http.GET() == 200);
      http.end();

      break;
    }
  }

  //----- MQTT Protocol configured -----
  if (_ha.protocol == HA_PROTO_MQTT)
  { //TODO
    //sn can be used in multiple cases
    char sn[9];
    sprintf_P(sn, PSTR("%08x"), ESP.getChipId());

    //if not connected to MQTT
    if (!_pubSubClient->connected())
    {
      //generate clientID
      String clientID(F(APPLICATION1_NAME));
      clientID += sn;
      //and try to connect
      if (!_ha.mqtt.username[0])
        _pubSubClient->connect(clientID.c_str());
      else
      {
        if (!_ha.mqtt.password[0])
          _pubSubClient->connect(clientID.c_str(), _ha.mqtt.username, NULL);
        else
          _pubSubClient->connect(clientID.c_str(), _ha.mqtt.username, _ha.mqtt.password);
      }
    }

    //if still not connected
    if (!_pubSubClient->connected())
    {
      //return error code minus 10 (result should be negative)
      _haSendResult = _pubSubClient->state();
      _haSendResult -= 10;
    }
    // else we are connected
    else
    {
      //prepare topic
      String completeTopic, thisLabelTopic;
      switch (_ha.mqtt.type)
      {
      case HA_MQTT_GENERIC:
        completeTopic = _ha.mqtt.generic.baseTopic;

        //check for final slash
        if (completeTopic.length() && completeTopic.charAt(completeTopic.length()-1) != '/')
          completeTopic += '/';
        break;
      }

      //Replace placeholders
      if (completeTopic.indexOf(F("$sn$")) != -1)
        completeTopic.replace(F("$sn$"), sn);

      if (completeTopic.indexOf(F("$mac$")) != -1)
        completeTopic.replace(F("$mac$"), WiFi.macAddress());

      if (completeTopic.indexOf(F("$model$")) != -1)
        completeTopic.replace(F("$model$"), APPLICATION1_NAME);

      if (completeTopic.indexOf(F("$adco$")) != -1)
        completeTopic.replace(F("$adco$"), _ADCO);

      _haSendResult = true;

      //go through labels to look for new Value
      while (me->next && _haSendResult)
      {
        me = me->next;
        // Add only new or updated values
        if (me->flags & (TINFO_FLAGS_ADDED | TINFO_FLAGS_UPDATED))
        {

          //copy completeTopic in order to "complete" it ...
          thisLabelTopic = completeTopic;

          //add the label name
          thisLabelTopic += me->name;

          //send
          _haSendResult = _pubSubClient->publish(thisLabelTopic.c_str(), me->value);
        }
      }
    }
  }

  //LOG
  Serial.println(F("Sent"));
}

//------------------------------------------
//Parse GET request to get a Counter Label
String WebTeleInfo::GetLabel(const String &labelName)
{
  //name : 9char max
  //value : 12char max
  char value[12];
  if (!_tinfo.valueGet((char *)labelName.c_str(), value))
    return String();
  else
    return String(F("{\"")) + labelName + F("\":\"") + value + F("\"}");
}
//------------------------------------------
//Parse GET request to get ALL Counter Label
String WebTeleInfo::GetAllLabel()
{
  String galvJSON('{');

  ValueList *vl = _tinfo.getList();
  bool first = true;

  while (vl->next)
  {
    vl = vl->next;

    if (!first)
      galvJSON += ',';
    else
      first = false;
    galvJSON = galvJSON + '\"' + vl->name + F("\":\"") + vl->value + F("\"\r\n");
  }

  galvJSON += '}';

  return galvJSON;
}

//------------------------------------------
//Used to initialize configuration properties to default values
void WebTeleInfo::SetConfigDefaultValues()
{
  _ha.protocol = HA_PROTO_DISABLED;
  _ha.tls = false;
  _ha.hostname[0] = 0;

  _ha.http.type = HA_HTTP_GENERIC;
  memset(_ha.http.fingerPrint, 0, 20);
  _ha.http.generic.uriPattern[0] = 0;
  _ha.http.jeedom.apiKey[0] = 0;

  _ha.mqtt.type = HA_MQTT_GENERIC;
  _ha.mqtt.port = 1883;
  _ha.mqtt.username[0] = 0;
  _ha.mqtt.password[0] = 0;
  _ha.mqtt.generic.baseTopic[0] = 0;
}
//------------------------------------------
//Parse JSON object into configuration properties
void WebTeleInfo::ParseConfigJSON(JsonObject &root)
{

  if (root[F("haproto")].success())
    _ha.protocol = root[F("haproto")];
  if (root[F("hatls")].success())
    _ha.tls = root[F("hatls")];
  if (root[F("hahost")].success())
    strlcpy(_ha.hostname, root[F("hahost")], sizeof(_ha.hostname));

  if (root[F("hahtype")].success())
    _ha.http.type = root[F("hahtype")];
  if (root[F("hahfp")].success())
    Utils::FingerPrintS2A(_ha.http.fingerPrint, root[F("hahfp")]);

  if (root[F("hahgup")].success())
    strlcpy(_ha.http.generic.uriPattern, root[F("hahgup")], sizeof(_ha.http.generic.uriPattern));

  if (root[F("hahjak")].success())
    strlcpy(_ha.http.jeedom.apiKey, root[F("hahjak")], sizeof(_ha.http.jeedom.apiKey));

  if (root[F("hamtype")].success())
    _ha.mqtt.type = root[F("hamtype")];
  if (root[F("hamport")].success())
    _ha.mqtt.port = root[F("hamport")];
  if (root[F("hamu")].success())
    strlcpy(_ha.mqtt.username, root[F("hamu")], sizeof(_ha.mqtt.username));
  if (root[F("hamp")].success())
    strlcpy(_ha.mqtt.password, root[F("hamp")], sizeof(_ha.mqtt.password));

  if (root[F("hamgbt")].success())
    strlcpy(_ha.mqtt.generic.baseTopic, root[F("hamgbt")], sizeof(_ha.mqtt.generic.baseTopic));
}
//------------------------------------------
//Parse HTTP POST parameters in request into configuration properties
bool WebTeleInfo::ParseConfigWebRequest(AsyncWebServerRequest *request)
{
  //Parse HA protocol
  if (request->hasParam(F("haproto"), true))
    _ha.protocol = request->getParam(F("haproto"), true)->value().toInt();

  //if an home Automation protocol has been selected then get common param
  if (_ha.protocol != HA_PROTO_DISABLED)
  {
    if (request->hasParam(F("hatls"), true))
      _ha.tls = (request->getParam(F("hatls"), true)->value() == F("on"));
    else
      _ha.tls = false;
    if (request->hasParam(F("hahost"), true) && request->getParam(F("hahost"), true)->value().length() < sizeof(_ha.hostname))
      strcpy(_ha.hostname, request->getParam(F("hahost"), true)->value().c_str());
  }

  //Now get specific param
  switch (_ha.protocol)
  {
  case HA_PROTO_HTTP:

    if (request->hasParam(F("hahtype"), true))
      _ha.http.type = request->getParam(F("hahtype"), true)->value().toInt();
    if (request->hasParam(F("hahfp"), true))
      Utils::FingerPrintS2A(_ha.http.fingerPrint, request->getParam(F("hahfp"), true)->value().c_str());

    switch (_ha.http.type)
    {
    case HA_HTTP_GENERIC:
      if (request->hasParam(F("hahgup"), true) && request->getParam(F("hahgup"), true)->value().length() < sizeof(_ha.http.generic.uriPattern))
        strcpy(_ha.http.generic.uriPattern, request->getParam(F("hahgup"), true)->value().c_str());
      if (!_ha.hostname[0] || !_ha.http.generic.uriPattern[0])
        _ha.protocol = HA_PROTO_DISABLED;
      break;
    case HA_HTTP_JEEDOM_TELEINFO:
      char tempApiKey[48 + 1];
      //put apiKey into temporary one for predefpassword
      if (request->hasParam(F("hahjak"), true) && request->getParam(F("hahjak"), true)->value().length() < sizeof(tempApiKey))
        strcpy(tempApiKey, request->getParam(F("hahjak"), true)->value().c_str());
      //check for previous apiKey (there is a predefined special password that mean to keep already saved one)
      if (strcmp_P(tempApiKey, appDataPredefPassword))
        strcpy(_ha.http.jeedom.apiKey, tempApiKey);
      if (!_ha.hostname[0] || !_ha.http.jeedom.apiKey[0])
        _ha.protocol = HA_PROTO_DISABLED;
      break;
    }
    break;

  case HA_PROTO_MQTT:

    if (request->hasParam(F("hamtype"), true))
      _ha.mqtt.type = request->getParam(F("hamtype"), true)->value().toInt();
    if (request->hasParam(F("hamport"), true))
      _ha.mqtt.port = request->getParam(F("hamport"), true)->value().toInt();
    if (request->hasParam(F("hamu"), true) && request->getParam(F("hamu"), true)->value().length() < sizeof(_ha.mqtt.username))
      strcpy(_ha.mqtt.username, request->getParam(F("hamu"), true)->value().c_str());
    char tempPassword[64 + 1] = {0};
    //put MQTT password into temporary one for predefpassword
    if (request->hasParam(F("hamp"), true) && request->getParam(F("hamp"), true)->value().length() < sizeof(tempPassword))
      strcpy(tempPassword, request->getParam(F("hamp"), true)->value().c_str());
    //check for previous password (there is a predefined special password that mean to keep already saved one)
    if (strcmp_P(tempPassword, appDataPredefPassword))
      strcpy(_ha.mqtt.password, tempPassword);

    switch (_ha.mqtt.type)
    {
    case HA_MQTT_GENERIC:
      if (request->hasParam(F("hamgbt"), true) && request->getParam(F("hamgbt"), true)->value().length() < sizeof(_ha.mqtt.generic.baseTopic))
        strcpy(_ha.mqtt.generic.baseTopic, request->getParam(F("hamgbt"), true)->value().c_str());

      if (!_ha.hostname[0] || !_ha.mqtt.generic.baseTopic[0])
        _ha.protocol = HA_PROTO_DISABLED;
      break;
    }
    break;
  }

  return true;
}
//------------------------------------------
//Generate JSON from configuration properties
String WebTeleInfo::GenerateConfigJSON(bool forSaveFile)
{
  char fpStr[60];

  String gc('{');

  gc = gc + F("\"haproto\":") + _ha.protocol;
  gc = gc + F(",\"hatls\":") + _ha.tls;
  gc = gc + F(",\"hahost\":\"") + _ha.hostname + '"';

  //if for WebPage or protocol selected is HTTP
  if (!forSaveFile || _ha.protocol == HA_PROTO_HTTP)
  {
    gc = gc + F(",\"hahtype\":") + _ha.http.type;
    gc = gc + F(",\"hahfp\":\"") + Utils::FingerPrintA2S(fpStr, _ha.http.fingerPrint, forSaveFile ? 0 : ':') + '"';

    gc = gc + F(",\"hahgup\":\"") + _ha.http.generic.uriPattern + '"';

    if (forSaveFile)
      gc = gc + F(",\"hahjak\":\"") + _ha.http.jeedom.apiKey + '"';
    else
      gc = gc + F(",\"hahjak\":\"") + (__FlashStringHelper *)appDataPredefPassword + '"'; //predefined special password (mean to keep already saved one)
  }

  //if for WebPage or protocol selected is MQTT
  if (!forSaveFile || _ha.protocol == HA_PROTO_MQTT)
  {
    gc = gc + F(",\"hamtype\":") + _ha.mqtt.type;
    gc = gc + F(",\"hamport\":") + _ha.mqtt.port;
    gc = gc + F(",\"hamu\":\"") + _ha.mqtt.username + '"';
    if (forSaveFile)
      gc = gc + F(",\"hamp\":\"") + _ha.mqtt.password + '"';
    else
      gc = gc + F(",\"hamp\":\"") + (__FlashStringHelper *)appDataPredefPassword + '"'; //predefined special password (mean to keep already saved one)

    gc = gc + F(",\"hamgbt\":\"") + _ha.mqtt.generic.baseTopic + '"';
  }

  gc = gc + '}';

  return gc;
}
//------------------------------------------
//Generate JSON of application status
String WebTeleInfo::GenerateStatusJSON()
{
  String gs('{');

  gs = gs + F("\"a\":\"") + _ADCO + '"';
  if (_ha.protocol != HA_PROTO_DISABLED)
  {
    gs = gs + F(",\"lrr\":") + _haSendResult;
  }

  gs = gs + '}';

  return gs;
}
//------------------------------------------
//code to execute during initialization and reinitialization of the app
bool WebTeleInfo::AppInit(bool reInit = false)
{

  //Clean up MQTT variables
  if (_pubSubClient)
  {
    if (_pubSubClient->connected())
      _pubSubClient->disconnect();
    delete _pubSubClient;
    _pubSubClient = NULL;
  }
  if (_wifiClient)
  {
    delete _wifiClient;
    _wifiClient = NULL;
  }
  if (_wifiClientSecure)
  {
    delete _wifiClientSecure;
    _wifiClientSecure = NULL;
  }

  //if MQTT used so build MQTT variables
  if (_ha.protocol == HA_PROTO_MQTT)
  {

    if (!_ha.tls)
    {
      _wifiClient = new WiFiClient();
      _pubSubClient = new PubSubClient(_ha.hostname, _ha.mqtt.port, *_wifiClient);
    }
    else
    {
      _wifiClientSecure = new WiFiClientSecure();
      _pubSubClient = new PubSubClient(_ha.hostname, _ha.mqtt.port, *_wifiClientSecure);
    }
  }

  if (!reInit)
  {

//If a Pin is defined to control TeleInfo signal arrival
#ifdef TELEINFO_CONTROL_PIN
    pinMode(TELEINFO_CONTROL_PIN, OUTPUT);
    digitalWrite(TELEINFO_CONTROL_PIN, LOW);
#endif

    //try to consume Serial Data until end of TeleInfo frame
    char c = 0;
    while (c != 0x03)
    {
      if (!Serial.available())
        delay(100);
      if (Serial.available())
        c = Serial.read() & 0x7f;
      else
        break;
    }
    return c == 0x03;
  }
  return true;
}
//------------------------------------------
//code to register web request answer to the web server
void WebTeleInfo::AppInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication)
{
  server.on("/getLabel", HTTP_GET, [this](AsyncWebServerRequest *request) {
    //if no name passed
    if (!request->hasParam(F("name")))
    {
      //return error
      request->send(400, F("text/html"), F("Missing parameter"));
      return;
    }
    //if name passed is empty
    if (request->getParam(F("name"))->value().length() == 0)
    {
      //return error
      request->send(400, F("text/html"), F("Incorrect label name"));
      return;
    }
    String labelResponse = GetLabel(request->getParam(F("name"))->value());
    //if return value is empty, label were not found
    if (!labelResponse.length())
      request->send(500, F("text/html"), F("Label cannot be found"));
    else
      request->send(200, F("text/json"), labelResponse); //value returned to client
  });

  server.on("/getAllLabel", HTTP_GET, [this](AsyncWebServerRequest *request) {
    request->send(200, F("text/json"), GetAllLabel());
  });
}

//------------------------------------------
//Run for timer
void WebTeleInfo::AppRun()
{
  if (_pubSubClient)
    _pubSubClient->loop();
  if (Serial.available())
    _tinfo.process(Serial.read() & 0x7f);
}

//------------------------------------------
//WebTeleInfo Constructor
WebTeleInfo::WebTeleInfo(char appId, String appName) : Application(appId, appName)
{
  // Init and configure TeleInfo
  _ADCO[0] = 0;
  _tinfo.init();
  _tinfo.attachUpdatedFrame([this](ValueList *vl) {
    this->tinfoUpdatedFrame(vl);
  });
}
