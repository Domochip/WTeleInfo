#include "WirelessTeleInfo.h"

//Please, have a look at Main.h for information and configuration of Arduino project

//------------------------------------------
// Execute code to upload Infos
void WebTeleInfo::publishTick(bool publishAll = true)
{
  //if Home Automation upload not enabled then return
  if (_ha.protocol == HA_PROTO_DISABLED)
    return;

  //Get Labels list
  ValueList *me = _tinfo.getList();

  if (!me)
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
        completeURI.replace(F("$tls$"), _ha.http.tls ? "s" : "");

      if (completeURI.indexOf(F("$host$")) != -1)
        completeURI.replace(F("$host$"), _ha.hostname);

      if (completeURI.indexOf(F("$apikey$")) != -1)
        completeURI.replace(F("$apikey$"), _ha.http.jeedom.apiKey);

      if (completeURI.indexOf(F("$adco$")) != -1)
        completeURI.replace(F("$adco$"), _SN);
      if (completeURI.indexOf(F("$adsc$")) != -1)
        completeURI.replace(F("$adsc$"), _SN);

      _haSendResult = true;

      //go through labels to look for new Value
      while (me->next && _haSendResult)
      {
        me = me->next;
        //publish All labels (publishAll passed) Or new/updated ones only (And name is not ADCO/ADSC (it will be added))
        if ((publishAll || (me->flags & (TINFO_FLAGS_ADDED | TINFO_FLAGS_UPDATED))) && strcmp_P(me->name, PSTR("ADCO")) != 0 && strcmp_P(me->name, PSTR("ADSC")) != 0)
        {

          String sendURI = completeURI;

          if (sendURI.indexOf(F("$label$")) != -1)
            sendURI.replace(F("$label$"), me->name);

          if (sendURI.indexOf(F("$val$")) != -1)
            sendURI.replace(F("$val$"), me->value);

          //create HTTP request objects
          HTTPClient http;

          //if tls is enabled or not, we need to provide certificate fingerPrint
          if (!_ha.http.tls)
            http.begin(_wifiClient, sendURI);
          else
          {
            if (Utils::isFingerPrintEmpty(_ha.http.fingerPrint))
              _wifiClientSecure.setInsecure();
            else
              _wifiClientSecure.setFingerprint(_ha.http.fingerPrint);
            http.begin(_wifiClientSecure, sendURI);
          }

          _haSendResult = (http.GET() == 200);
          http.end();

          //Send published value to Web clients through EventSource
          _statusEventSource.send((String("{\"") + (char *)me->name + "\":\"" + (char *)me->value + "\"}").c_str());
        }
      }

      break;
    case HA_HTTP_JEEDOM_TELEINFO:
      //if we didn't find counter SN then we can't send stuff so return
      if (!_SN[0])
        return;

      completeURI = F("http$tls$://$host$/plugins/teleinfo/core/php/jeeTeleinfo.php?apikey=$apikey$");

      //initialize data list to send
      _httpJeedomRequest = String(F("{\"device\":{\"")) + _SN + F("\":{\"device\":\"") + _SN + '"';

      bool sendNeeded = false;

      //go through labels to look for new Value
      while (me->next)
      {
        me = me->next;
        //publish All labels (publishAll passed) Or new/updated ones only (And name is not ADCO/ADSC)
        if ((publishAll || (me->flags & (TINFO_FLAGS_ADDED | TINFO_FLAGS_UPDATED))) && strcmp_P(me->name, PSTR("ADCO")) != 0 && strcmp_P(me->name, PSTR("ADSC")) != 0)
        {
          _httpJeedomRequest += String(",\"") + (char *)me->name + F("\":\"") + (char *)me->value + '"';
          sendNeeded = true;
        }
      }

      //close data list to send
      _httpJeedomRequest += F("}}}");

      //do we need to send data
      if (!sendNeeded)
        return;

      //Replace placeholders
      if (completeURI.indexOf(F("$tls$")) != -1)
        completeURI.replace(F("$tls$"), _ha.http.tls ? "s" : "");

      if (completeURI.indexOf(F("$host$")) != -1)
        completeURI.replace(F("$host$"), _ha.hostname);

      if (completeURI.indexOf(F("$apikey$")) != -1)
        completeURI.replace(F("$apikey$"), _ha.http.jeedom.apiKey);

      //create HTTP request objects
      HTTPClient http;

      //if tls is enabled or not, we need to provide certificate fingerPrint
      if (!_ha.http.tls)
        http.begin(_wifiClient, completeURI);
      else
      {
        if (Utils::isFingerPrintEmpty(_ha.http.fingerPrint))
          _wifiClientSecure.setInsecure();
        else
          _wifiClientSecure.setFingerprint(_ha.http.fingerPrint);
        http.begin(_wifiClientSecure, completeURI);
      }

      http.addHeader("Content-Type", "application/json");

      _haSendResult = (http.POST(_httpJeedomRequest.c_str()) == 200);
      http.end();

      //Send published values to Web clients through EventSource
      _statusEventSource.send(getAllLabel().c_str());

      break;
    }
  }

  //----- MQTT Protocol configured -----
  if (_ha.protocol == HA_PROTO_MQTT)
  {
    //if we are connected
    if (_mqttMan.connected())
    {
      //prepare topic
      String completeTopic = _ha.mqtt.generic.baseTopic;

      //Replace placeholders
      MQTTMan::prepareTopic(completeTopic);

      switch (_ha.mqtt.type)
      {
      case HA_MQTT_GENERIC1:
        completeTopic += F("$adco$/");
        break;
      case HA_MQTT_GENERIC2:
        //no adco/adsc added in this configuration
        break;
      }

      if (completeTopic.indexOf(F("$adco$")) != -1)
        completeTopic.replace(F("$adco$"), _SN);
      if (completeTopic.indexOf(F("$adsc$")) != -1)
        completeTopic.replace(F("$adsc$"), _SN);

      _haSendResult = true;

      //go through labels to look for values
      while (me->next && _haSendResult)
      {
        me = me->next;

        //publish All labels (publishAll passed) Or new/updated ones only
        if (publishAll || (me->flags & (TINFO_FLAGS_ADDED | TINFO_FLAGS_UPDATED)))
        {
          //copy completeTopic in order to "complete" it ...
          String thisLabelTopic = completeTopic;

          //add the label name
          thisLabelTopic += (char *)me->name;

          //send
          _haSendResult = _mqttMan.publish(thisLabelTopic.c_str(), me->value);

          //Send published value to Web clients through EventSource
          _statusEventSource.send((String("{\"") + (char *)me->name + "\":\"" + (char *)me->value + "\"}").c_str());
        }
      }
    }
  }
}

//------------------------------------------
// TeleInfo CallBack function (when value change occured)
void WebTeleInfo::tinfoUpdatedFrame(ValueList *me)
{
  //if pointer is null
  if (!me)
    return;

  if (!_SN[0])
  {
    //Find ADCO/ADSC
    ValueList *snLookup = me;
    while (snLookup->next)
    {
      snLookup = snLookup->next;
      if (!strcmp_P(snLookup->name, PSTR("ADCO")) || !strcmp_P(snLookup->name, PSTR("ADSC")))
      {
        strcpy(_SN, snLookup->value);
        Serial.print(snLookup->name);
        Serial.print(F(" : "));
        Serial.println(_SN);
      }
    }
  }

  //if Upload period is not 0 then we don't send values in real time
  if (_ha.uploadPeriod != 0)
    return;

  //Publish but not all labels for real time
  publishTick(false);

  //LOG
  Serial.println(F("Sent"));
}

//------------------------------------------
//Parse GET request to get a Counter Label
String WebTeleInfo::getLabel(const String &labelName)
{
  //value : 12char max
  char value[13];
  if (!_tinfo.valueGet((char *)labelName.c_str(), value))
    return String();
  else
    return String(F("{\"")) + labelName + F("\":\"") + value + F("\"}");
}
//------------------------------------------
//Parse GET request to get ALL Counter Label
String WebTeleInfo::getAllLabel()
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
    galvJSON = galvJSON + '\"' + (char *)vl->name + F("\":\"") + (char *)vl->value + F("\"\r\n");
  }

  galvJSON += '}';

  return galvJSON;
}

//------------------------------------------
// subscribe to MQTT topic after connection
void WebTeleInfo::mqttConnectedCallback(MQTTMan *mqttMan, bool firstConnection) {}

//------------------------------------------
//Callback used when an MQTT message arrived
void WebTeleInfo::mqttCallback(char *topic, uint8_t *payload, unsigned int length) {}

//------------------------------------------
//Used to initialize configuration properties to default values
void WebTeleInfo::setConfigDefaultValues()
{
  _tinfoMode = TINFO_MODE_HISTORIQUE;

  _ha.protocol = HA_PROTO_DISABLED;
  _ha.hostname[0] = 0;
  _ha.uploadPeriod = 60;

  _ha.http.type = HA_HTTP_GENERIC;
  _ha.http.tls = false;
  memset(_ha.http.fingerPrint, 0, 20);
  _ha.http.generic.uriPattern[0] = 0;
  _ha.http.jeedom.apiKey[0] = 0;

  _ha.mqtt.type = HA_MQTT_GENERIC1;
  _ha.mqtt.port = 1883;
  _ha.mqtt.username[0] = 0;
  _ha.mqtt.password[0] = 0;
  _ha.mqtt.generic.baseTopic[0] = 0;
}
//------------------------------------------
//Parse JSON object into configuration properties
void WebTeleInfo::parseConfigJSON(DynamicJsonDocument &doc)
{
  if (!doc[F("tim")].isNull())
    _tinfoMode = doc[F("tim")];

  if (!doc[F("haproto")].isNull())
    _ha.protocol = doc[F("haproto")];
  if (!doc[F("hahost")].isNull())
    strlcpy(_ha.hostname, doc[F("hahost")], sizeof(_ha.hostname));
  if (!doc[F("haupperiod")].isNull())
    _ha.uploadPeriod = doc[F("haupperiod")];

  if (!doc[F("hahtype")].isNull())
    _ha.http.type = doc[F("hahtype")];
  if (!doc[F("hahtls")].isNull())
    _ha.http.tls = doc[F("hahtls")];
  if (!doc[F("hahfp")].isNull())
    Utils::fingerPrintS2A(_ha.http.fingerPrint, doc[F("hahfp")]);

  if (!doc[F("hahgup")].isNull())
    strlcpy(_ha.http.generic.uriPattern, doc[F("hahgup")], sizeof(_ha.http.generic.uriPattern));

  if (!doc[F("hahjak")].isNull())
    strlcpy(_ha.http.jeedom.apiKey, doc[F("hahjak")], sizeof(_ha.http.jeedom.apiKey));

  if (!doc[F("hamtype")].isNull())
    _ha.mqtt.type = doc[F("hamtype")];
  if (!doc[F("hamport")].isNull())
    _ha.mqtt.port = doc[F("hamport")];
  if (!doc[F("hamu")].isNull())
    strlcpy(_ha.mqtt.username, doc[F("hamu")], sizeof(_ha.mqtt.username));
  if (!doc[F("hamp")].isNull())
    strlcpy(_ha.mqtt.password, doc[F("hamp")], sizeof(_ha.mqtt.password));

  if (!doc[F("hamgbt")].isNull())
    strlcpy(_ha.mqtt.generic.baseTopic, doc[F("hamgbt")], sizeof(_ha.mqtt.generic.baseTopic));
}
//------------------------------------------
//Parse HTTP POST parameters in request into configuration properties
bool WebTeleInfo::parseConfigWebRequest(AsyncWebServerRequest *request)
{
  //Parse TeleInfo mode
  if (request->hasParam(F("tim"), true))
    _tinfoMode = (_Mode_e)request->getParam(F("tim"), true)->value().toInt();

  //Parse HA protocol
  if (request->hasParam(F("haproto"), true))
    _ha.protocol = request->getParam(F("haproto"), true)->value().toInt();

  //if an home Automation protocol has been selected then get common param
  if (_ha.protocol != HA_PROTO_DISABLED)
  {
    if (request->hasParam(F("hahost"), true) && request->getParam(F("hahost"), true)->value().length() < sizeof(_ha.hostname))
      strcpy(_ha.hostname, request->getParam(F("hahost"), true)->value().c_str());
    if (request->hasParam(F("haupperiod"), true))
      _ha.uploadPeriod = request->getParam(F("haupperiod"), true)->value().toInt();
  }

  //Now get specific param
  switch (_ha.protocol)
  {
  case HA_PROTO_HTTP:

    if (request->hasParam(F("hahtype"), true))
      _ha.http.type = request->getParam(F("hahtype"), true)->value().toInt();
    if (request->hasParam(F("hahtls"), true))
      _ha.http.tls = (request->getParam(F("hahtls"), true)->value() == F("on"));
    else
      _ha.http.tls = false;
    if (request->hasParam(F("hahfp"), true))
      Utils::fingerPrintS2A(_ha.http.fingerPrint, request->getParam(F("hahfp"), true)->value().c_str());

    switch (_ha.http.type)
    {
    case HA_HTTP_GENERIC:
      if (request->hasParam(F("hahgup"), true) && request->getParam(F("hahgup"), true)->value().length() < sizeof(_ha.http.generic.uriPattern))
        strcpy(_ha.http.generic.uriPattern, request->getParam(F("hahgup"), true)->value().c_str());
      if (!_ha.hostname[0] || !_ha.http.generic.uriPattern[0])
        _ha.protocol = HA_PROTO_DISABLED;
      break;
    case HA_HTTP_JEEDOM_TELEINFO:
      char tempApiKey[64 + 1];
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
    case HA_MQTT_GENERIC1:
    case HA_MQTT_GENERIC2:
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
String WebTeleInfo::generateConfigJSON(bool forSaveFile)
{
  char fpStr[60];

  String gc('{');

  gc = gc + F("\"tim\":") + _tinfoMode;

  gc = gc + F(",\"haproto\":") + _ha.protocol;
  gc = gc + F(",\"hahost\":\"") + _ha.hostname + '"';
  gc = gc + F(",\"haupperiod\":") + _ha.uploadPeriod;

  //if for WebPage or protocol selected is HTTP
  if (!forSaveFile || _ha.protocol == HA_PROTO_HTTP)
  {
    gc = gc + F(",\"hahtype\":") + _ha.http.type;
    gc = gc + F(",\"hahtls\":") + _ha.http.tls;
    gc = gc + F(",\"hahfp\":\"") + Utils::fingerPrintA2S(fpStr, _ha.http.fingerPrint, forSaveFile ? 0 : ':') + '"';

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
String WebTeleInfo::generateStatusJSON()
{
  String gs('{');

  gs = gs + F("\"liveData\":") + getAllLabel();
  gs = gs + F(",\"has1\":\"");
  switch (_ha.protocol)
  {
  case HA_PROTO_DISABLED:
    gs = gs + F("Disabled");
    break;
  case HA_PROTO_HTTP:
    gs = gs + F("Last HTTP request result : ") + (_haSendResult ? F("OK") : F("Failed"));
    break;
  case HA_PROTO_MQTT:
    gs = gs + F("MQTT Connection State : ");
    switch (_mqttMan.state())
    {
    case MQTT_CONNECTION_TIMEOUT:
      gs = gs + F("Timed Out");
      break;
    case MQTT_CONNECTION_LOST:
      gs = gs + F("Lost");
      break;
    case MQTT_CONNECT_FAILED:
      gs = gs + F("Failed");
      break;
    case MQTT_CONNECTED:
      gs = gs + F("Connected");
      break;
    case MQTT_CONNECT_BAD_PROTOCOL:
      gs = gs + F("Bad Protocol Version");
      break;
    case MQTT_CONNECT_BAD_CLIENT_ID:
      gs = gs + F("Incorrect ClientID ");
      break;
    case MQTT_CONNECT_UNAVAILABLE:
      gs = gs + F("Server Unavailable");
      break;
    case MQTT_CONNECT_BAD_CREDENTIALS:
      gs = gs + F("Bad Credentials");
      break;
    case MQTT_CONNECT_UNAUTHORIZED:
      gs = gs + F("Connection Unauthorized");
      break;
    }

    if (_mqttMan.state() == MQTT_CONNECTED)
      gs = gs + F("\",\"has2\":\"Last Publish Result : ") + (_haSendResult ? F("OK") : F("Failed"));

    break;
  }
  gs += '"';
  gs += '}';

  return gs;
}
//------------------------------------------
//code to execute during initialization and reinitialization of the app
bool WebTeleInfo::appInit(bool reInit = false)
{
  //Prepare Serial port according to the tinfoMode
  Serial.flush();
  Serial.end();
  //begin Serial to the right speed
  Serial.begin(_tinfoMode == TINFO_MODE_HISTORIQUE ? 1200 : 9600);
  Serial.pins(15, 13); //swap ESP8266 pins to alternative positions (D7(GPIO13)(RX)/D8(GPIO15)(TX))

  //Initialize _tinfo (clear all labels, clear buffer, update mode)
  _tinfo.init(_tinfoMode);


  //Stop Publish
  _publishTicker.detach();

  //Stop MQTT
  _mqttMan.disconnect();

  //if MQTT used so configure it
  if (_ha.protocol == HA_PROTO_MQTT)
  {
    //prepare will topic
    String willTopic = _ha.mqtt.generic.baseTopic;
    MQTTMan::prepareTopic(willTopic);
    willTopic += F("connected");

    //setup MQTT
    _mqttMan.setClient(_wifiClient).setServer(_ha.hostname, _ha.mqtt.port);
    _mqttMan.setConnectedAndWillTopic(willTopic.c_str());
    _mqttMan.setConnectedCallback(std::bind(&WebTeleInfo::mqttConnectedCallback, this, std::placeholders::_1, std::placeholders::_2));
    _mqttMan.setCallback(std::bind(&WebTeleInfo::mqttCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    //Connect
    _mqttMan.connect(_ha.mqtt.username, _ha.mqtt.password);
  }

  //if HA and upload period != 0, then start ticker
  if (_ha.protocol != HA_PROTO_DISABLED && _ha.uploadPeriod != 0)
  {
    publishTick(); //if configuration changed, publish immediately
    _publishTicker.attach(_ha.uploadPeriod, [this]() { this->_needPublish = true; });
  }

  return true;
}
//------------------------------------------
//Return HTML Code to insert into Status Web page
const uint8_t *WebTeleInfo::getHTMLContent(WebPageForPlaceHolder wp)
{
  switch (wp)
  {
  case status:
    return (const uint8_t *)status1htmlgz;
    break;
  case config:
    return (const uint8_t *)config1htmlgz;
    break;
  default:
    return nullptr;
    break;
  };
  return nullptr;
};
//and his Size
size_t WebTeleInfo::getHTMLContentSize(WebPageForPlaceHolder wp)
{
  switch (wp)
  {
  case status:
    return sizeof(status1htmlgz);
    break;
  case config:
    return sizeof(config1htmlgz);
    break;
  default:
    return 0;
    break;
  };
  return 0;
};
//------------------------------------------
//code to register web request answer to the web server
void WebTeleInfo::appInitWebServer(AsyncWebServer &server, bool &shouldReboot, bool &pauseApplication)
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
    String labelResponse = getLabel(request->getParam(F("name"))->value());
    //if return value is empty, label were not found
    if (!labelResponse.length())
      request->send(500, F("text/html"), F("Label cannot be found"));
    else
      request->send(200, F("text/json"), labelResponse); //value returned to client
  });

  server.on("/getAllLabel", HTTP_GET, [this](AsyncWebServerRequest *request) {
    request->send(200, F("text/json"), getAllLabel());
  });
}

//------------------------------------------
//Run for timer
void WebTeleInfo::appRun()
{
  if (_ha.protocol == HA_PROTO_MQTT)
    _mqttMan.loop();

  if (Serial.available())
    _tinfo.process(Serial.read() & 0x7f);

  if (_needPublish)
  {
    _needPublish = false;
    LOG_SERIAL.println(F("PublishTick"));
    publishTick();
  }
}

//------------------------------------------
//WebTeleInfo Constructor
WebTeleInfo::WebTeleInfo(char appId, String appName) : Application(appId, appName)
{
  // Init and configure TeleInfo
  _SN[0] = 0;
  _tinfo.init(_tinfoMode);
  _tinfo.attachUpdatedFrame([this](ValueList *vl) {
    this->tinfoUpdatedFrame(vl);
  });
}
