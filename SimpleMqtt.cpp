#include"SimpleMqtt.h"
#include<Arduino.h>
#include <EspNowFloodingMesh.h>
#include "base64.h"
#include <safememcpy.h>

SimpleMQTT::SimpleMQTT(int ttl, const char *deviceName) {
  buffer[0] = 0;
  this->ttl = ttl;
  myDeviceName = deviceName;
  static SimpleMQTT *myself = this;
  espNowFloodingMesh_RecvCB([](const uint8_t *data, int len, uint32_t replyPrt) {
    if (len > 0) {
      myself->parse(data, len, replyPrt); //Parse simple Mqtt protocol messages
    }
  });
}

SimpleMQTT::~SimpleMQTT() {
}

bool SimpleMQTT::compareTopic(const char* topic, const char* deviceName, const char* t) {
  String tmp = deviceName;
  tmp += t;
  return strcmp(topic, tmp.c_str()) == 0;
}

bool SimpleMQTT::publish(const char* deviceName, const char* parameterName, const char *value) {
  char *p = buffer;
  p += snprintf(p, sizeof(buffer)-(p-buffer), "MQTT %s\nP:%s%s:%s\n", myDeviceName.c_str(), deviceName, parameterName, value);
  return send(buffer, (int)(p - buffer) + 1, 0);
}

bool SimpleMQTT::subscribeTopic(const char* devName, const char *valName) {
  char *p = buffer;
  snprintf(buffer, sizeof(buffer), "%s%s", devName, valName);
  addtopicToVector(buffer);

  p += snprintf(p, sizeof(buffer), "MQTT %s\nS:%s%s\n", myDeviceName.c_str(), devName, valName);
  bool ret = send(buffer, (int)(p - buffer) + 1, 0);

  return ret;
}

bool SimpleMQTT::listenTopic(const char* devName, const char *valName){
  char *p = buffer;
  snprintf(buffer, sizeof(buffer), "%s%s", devName, valName);
  addtopicToVector(buffer);
}

bool SimpleMQTT::getTopic(const char* devName, const char *valName) {
  snprintf(buffer, sizeof(buffer), "MQTT %s\nG:%s%s\n", myDeviceName.c_str(), devName, valName);

  bool ret = send(buffer, strlen(buffer) + 1, 0);

  snprintf(buffer,sizeof(buffer), "%s%s", devName, valName);
  addtopicToVector(buffer);

  return ret;
}

bool SimpleMQTT::unsubscribeTopic(const char* devName, const char *valName) {
  char *p = buffer;
  snprintf(buffer, sizeof(buffer), "%s%s", devName, valName);
  removeTopicFomVector(buffer);

  p += snprintf(p, sizeof(buffer), "MQTT %s\nU:%s%s\n", myDeviceName.c_str(), devName, valName);
  bool ret = send(buffer, (int)(p - buffer) + 1, 0);

  return ret;
}

bool SimpleMQTT::_raw(Mqtt_cmd cmd, const char* type, const std::list<const char*> & names, const char *value){
  char *p = buffer;

  bool removeFromVector=false;
  const char * name = names.front();

  bool addToVector=false;
  bool ret = true;


  int c = 0;
  bool first = true;
  p = buffer;
  p += snprintf(p, sizeof(buffer)-(p-buffer), "MQTT %s\n",  myDeviceName.c_str());

  for (auto const& name : names) {
    if(c>2) {
      if(!send(buffer, (int)(p - buffer) + 1, 0)) {
        ret=false;
      }
      p = buffer;
      p += snprintf(p, sizeof(buffer)-(p-buffer), "MQTT %s\n",  myDeviceName.c_str());
      c=0;
      first=true;
    }

    if(cmd==SUBSCRIBE) {
      if(first) {
        p += snprintf(p, sizeof(buffer)-(p-buffer), "S:%s/%s/%s/set\n", myDeviceName.c_str(),type, name);
        p += snprintf(p, sizeof(buffer)-(p-buffer), "G:.../value\n", myDeviceName.c_str(), type, name);
        first = false;
      } else {
        p += snprintf(p, sizeof(buffer)-(p-buffer), "S:../%s/set\n", name);
        p += snprintf(p, sizeof(buffer)-(p-buffer), "G:.../value\n");
      }
      addToVector = true;
    } else if(cmd==UNSUBSCRIBE) {
      if(first) {
        p += snprintf(p, sizeof(buffer)-(p-buffer),"U:%s/%s/%s/set\n", myDeviceName.c_str(),type, name);
        first=false;
      } else {
        p += snprintf(p, sizeof(buffer)-(p-buffer),"U:../%s/set\n", name);
      }
      removeFromVector = true;
    }
    else if(cmd==GET){
      if(first) {
        p += snprintf(p, sizeof(buffer)-(p-buffer),"G:%s/%s/%s/value\n", myDeviceName.c_str(), type, name);
        p += snprintf(p, sizeof(buffer)-(p-buffer),"G:.../set\n");
        first=false;
      } else {
        p += snprintf(p, sizeof(buffer)-(p-buffer),"G:../%s/value\n", name);
        p += snprintf(p, sizeof(buffer)-(p-buffer),"G:.../set\n", myDeviceName.c_str(), type,name);
      }
      addToVector = true;
    }
    else if(cmd==PUBLISH){
      if(first) {
      p += snprintf(p, sizeof(buffer)-(p-buffer),"P:%s/%s/%s/value %s\n", myDeviceName.c_str(), type, name, value);
      first=false;
      } else {
        p += snprintf(p, sizeof(buffer)-(p-buffer),"P:../%s/value %s\n", name, value);
      }
    } else {
      return false;
    }
    c++;
  }
  if(!send(buffer, (int)(p - buffer) + 1, 0)) {
      ret=false;
  }
  if(addToVector) {
    for (auto const& name : names) {
      snprintf(buffer, sizeof(buffer), "%s/%s/%s/set", myDeviceName.c_str(), type, name);
      addtopicToVector(buffer);
      snprintf(buffer, sizeof(buffer), "%s/%s/%s/value", myDeviceName.c_str(), type, name);
      addtopicToVector(buffer);
    }
  }
  if(removeFromVector) {
    for (auto const& name : names) {
      snprintf(buffer, sizeof(buffer), "%s/%s/%s/set", myDeviceName.c_str(), type, name);
      removeTopicFomVector(buffer);
      snprintf(buffer, sizeof(buffer), "%s/%s/%s/value", myDeviceName.c_str(), type, name);
      removeTopicFomVector(buffer);
    }
  }
  return ret;
}

bool SimpleMQTT::_switch(Mqtt_cmd cmd, const std::list<const char*> & names, MQTT_switch value){
  return _raw(cmd, "switch", names, value==SWITCH_ON?"on":"off");
}
bool SimpleMQTT::_temp(Mqtt_cmd cmd, const std::list<const char*> & names, float value){
  char v[20];
  snprintf(v,sizeof(v),"%f", value);
  return _raw(cmd, "temp", names, v);
}
bool SimpleMQTT::_humidity(Mqtt_cmd cmd, const std::list<const char*> & names, float value){
  char v[20];
  snprintf(v,sizeof(v),"%f", value);
  return _raw(cmd, "humidity", names, v);
}
bool SimpleMQTT::_trigger(Mqtt_cmd cmd, const std::list<const char*> & names, MQTT_trigger value){
  return _raw(cmd, "trigger", names, "triggered");
}
bool SimpleMQTT::_contact(Mqtt_cmd cmd, const std::list<const char*> & names, MQTT_contact value){
   _raw(cmd, "contact", names, value==CONTACT_OPEN?"open":"closed");
}
bool SimpleMQTT::_dimmer(Mqtt_cmd cmd, const std::list<const char*> & names, uint8_t value){
  char v[20];
  snprintf(v,sizeof(v),"%d", value);
  return _raw(cmd, "dimmer", names, v);
}
bool SimpleMQTT::_string(Mqtt_cmd cmd, const std::list<const char*> & names, const char *value){
  return _raw(cmd, "string", names, value);
}
bool SimpleMQTT::_number(Mqtt_cmd cmd, const std::list<const char*> & names, int min, int max, int step){
  char v[50];
  snprintf(v,sizeof(v),"%d,%d,%d", min,max,step);
  return _raw(cmd, "number", names, v);
}
bool SimpleMQTT::_float(Mqtt_cmd cmd, const std::list<const char*> & names, float value){
  char v[20];
  snprintf(v,sizeof(v),"%f", value);
  return _raw(cmd, "float", names, v);
}
bool SimpleMQTT::_int(Mqtt_cmd cmd, const std::list<const char*> & names, int value){
  char v[20];
  snprintf(v,sizeof(v),"%d", value);
  return _raw(cmd, "int", names, v);
}
bool SimpleMQTT::_shutter(Mqtt_cmd cmd, const std::list<const char*> & names, MQTT_shutter value){
  const char *type="shutter";
   switch (value) {
     case SHUTTER_OPEN: return _raw(cmd, type, names, "open");
     case SHUTTER_CLOSE: return _raw(cmd, type, names, "close");
     case SHUTTER_STOP: return _raw(cmd, type, names, "stop");
   }
   return false;
}
bool SimpleMQTT::_bin(Mqtt_cmd cmd,  const std::list<const char*> & names, const uint8_t* data, int len) {
  if(data!=NULL){
    char b[250];
    if(!toBase64(b, sizeof(b), data, len)){
      return false;
    }
    return _raw(cmd, "bin", names, b);
  }
  return _raw(cmd, "bin", names, NULL);
}
bool SimpleMQTT::_counter(Mqtt_cmd cmd, const std::list<const char*> & names, int value){
  char v[20];
  snprintf(v,sizeof(v),"%d", value);
  return _raw(cmd, "counter", names, v);
}
/********************************************************************************************************/

bool SimpleMQTT::_switch(Mqtt_cmd cmd, const char* name, MQTT_switch value){
    std::list<const char*> t= {name};
  return _switch(cmd, t, value);
}
bool SimpleMQTT::_temp(Mqtt_cmd cmd, const char* name, float value){
    std::list<const char*> t= {name};
return _temp(cmd, t, value);
}
bool SimpleMQTT::_humidity(Mqtt_cmd cmd, const char* name, float value){
    std::list<const char*> t= {name};
return _humidity(cmd, t, value);
}

bool SimpleMQTT::_trigger(Mqtt_cmd cmd, const char* name, MQTT_trigger value){
  std::list<const char*> t= {name};
  return _trigger(cmd, t, value);
}
bool SimpleMQTT::_contact(Mqtt_cmd cmd, const char* name, MQTT_contact value){
    std::list<const char*> t= {name};
return _contact(cmd, t, value);}
bool SimpleMQTT::_dimmer(Mqtt_cmd cmd, const char* name, uint8_t value){
    std::list<const char*> t= {name};
return _dimmer(cmd, t, value);
}
bool SimpleMQTT::_string(Mqtt_cmd cmd, const char* name, const char *value){
    std::list<const char*> t= {name};
return _string(cmd, t, value);
}
bool SimpleMQTT::_number(Mqtt_cmd cmd, const char* name, int min, int max, int step){
    std::list<const char*> t= {name};
return _number(cmd, t, min,max,step);
}
bool SimpleMQTT::_float(Mqtt_cmd cmd, const char* name, float value){
    std::list<const char*> t= {name};
return _float(cmd, t, value);
}
bool SimpleMQTT::_int(Mqtt_cmd cmd, const char* name, int value){
    std::list<const char*> t= {name};
return _int(cmd, t, value);
}
bool SimpleMQTT::_shutter(Mqtt_cmd cmd, const char* name, MQTT_shutter value){
    std::list<const char*> t= {name};
return _shutter(cmd, t, value);
}
bool SimpleMQTT::_bin(Mqtt_cmd cmd,  const char* name, const uint8_t* data, int len) {
    std::list<const char*> t= {name};
return _bin(cmd, t, data,len);
}
bool SimpleMQTT::_counter(Mqtt_cmd cmd, const char* name, int value){
    std::list<const char*> t= {name};
  return _counter(cmd, t, value);
}
/********************************************************************************************************/

float toFloat(const char* v) {
  float ret;
  sscanf(v, "%f", &ret);
  return ret;
}
int toInt(const char* v) {
  int ret;
  sscanf(v, "%d", &ret);
  return ret;
}

bool SimpleMQTT::compare(MQTT_IF ifType, const char* type, const char* name) {
  if(ifType==SET) {
    snprintf(buffer, sizeof(buffer), "%s/%s/%s/set", myDeviceName.c_str(), type, name);
  } else { //VALUE
    snprintf(buffer, sizeof(buffer), "%s/%s/%s/value", myDeviceName.c_str(), type, name);
  }
  return strcmp(_topic, buffer) == 0;
}

bool SimpleMQTT::_rawIf(MQTT_IF ifType, const char* type, const char* name){
  if(ifType==SET||ifType==VALUE) {
    return compare(ifType, type, name);
  } else {
    return compare(SET, type, name) || compare(VALUE, type, name);
  }
}

bool SimpleMQTT::_ifSwitch(MQTT_IF ifType, const char* name, void (*cb)(MQTT_switch /*value*/)){
  if(!_rawIf( ifType, "switch", name)) return false;
  if(strcmp(_value, "on")==0) cb(SWITCH_ON);
  else cb(SWITCH_OFF);
  return false;
}

bool SimpleMQTT::_ifTemp(MQTT_IF ifType, const char* name, void (*cb)(float /*value*/)){
  if(!_rawIf( ifType, "temp", name)) return false;
  return toFloat(_value);
}
bool SimpleMQTT::_ifHumidity(MQTT_IF ifType, const char* name, void (*cb)(float /*value*/)){
  if(!_rawIf( ifType, "humidity", name)) return false;
  return toFloat(_value);
}
bool SimpleMQTT::_ifTrigger(MQTT_IF ifType, const char* name, void (*cb)(MQTT_trigger /*value*/)){
  if(!_rawIf( ifType, "trigger", name)) return false;
  cb(TRIGGERED);
}
bool SimpleMQTT::_ifContact(MQTT_IF ifType, const char* name, void (*cb)(MQTT_contact /*value*/)){
  if(!_rawIf( ifType, "contact", name)) return false;
  if(strcmp(_value, "open")==0) cb(CONTACT_OPEN);
  else if(strcmp(_value, "closed")==0) cb(CONTACT_CLOSED);
  else return false;
  return true;
}
bool SimpleMQTT::_ifDimmer(MQTT_IF ifType, const char* name, void (*cb)(uint8_t /*value*/)){
  if(!_rawIf( ifType, "dimmer", name)) return false;
  cb(toInt(_value));
  return true;
}
bool SimpleMQTT::_ifString(MQTT_IF ifType, const char* name, void (*cb)(const char */*value*/)){
  if(!_rawIf( ifType, "string", name)) return false;
  cb(_value);
  return true;
}
bool SimpleMQTT::_ifNumber(MQTT_IF ifType, const char* name, void (*cb)(int /*min*/, int /*max*/, int /*step*/)){
  if(!_rawIf( ifType, "number", name)) return false;
  int min,max,step;
  sscanf(_value,"%d,%d,%d"),&min,&max,&step;
  cb(min,max,step);
  return true;
}
bool SimpleMQTT::_ifFloat(MQTT_IF ifType, const char* name, void (*cb)(float /*value*/)){
  if(!_rawIf( ifType, "float", name)) return false;
  cb(toFloat(_value));
  return true;
}
bool SimpleMQTT::_ifInt(MQTT_IF ifType, const char* name, void (*cb)(int /*value*/)){
  if(!_rawIf( ifType, "int", name)) return false;
  cb(toInt(_value));
  return true;
}
bool SimpleMQTT::_ifShutter(MQTT_IF ifType, const char* name, void (*cb)(MQTT_shutter /*value*/)){
  if(!_rawIf( ifType, "shutter", name)) return false;
  if(strcmp(_value, "open")==0) cb(SHUTTER_OPEN);
  else if(strcmp(_value, "close")==0) cb(SHUTTER_OPEN);
  else if(strcmp(_value, "stop")==0) cb(SHUTTER_OPEN);
  else return false;
  return true;
}
bool SimpleMQTT::_ifCounter(MQTT_IF ifType, const char* name, void (*cb)(int /*value*/)){
  if(!_rawIf( ifType, "counter", name)) return false;
  cb(toInt(_value));
  return true;
}
bool SimpleMQTT::_ifBin(MQTT_IF ifType, const char* name, void (*cb)(const uint8_t */*bin*/,int /*length*/)){
  if(!_rawIf( ifType, "bin", name)) return false;
  else {
    uint8_t b[250];
    int len = fromBase64(b, sizeof(b), _value);
    cb(b, len);
    return true;
  }
}




void SimpleMQTT::addtopicToVector(const char *topic) {
  for (char* subscribed_topic : topicVector) {
    if (strcmp(subscribed_topic, topic) == 0) {
      return;
    }
  }
  int l = strlen(topic) + 1;
  char *b = (char*)malloc(l);
  memcpy(b, topic, l);
  topicVector.push_back(b);
}

void SimpleMQTT::removeTopicFomVector(const char *topic) {
  topicVector.remove_if([topic](char *t)->bool{
    if (strcmp(t, topic) == 0) {
      free(t);
      return true;
    }
    return false;
  });
}

void SimpleMQTT::parse(const unsigned char *data, int size, uint32_t replyId, bool subscribeSequance) {
  this->replyId = replyId;
  if ( size>5 &&data[0] == 'M' && data[1] == 'Q' && data[2] == 'T' && data[3] == 'T' && (data[4] == '\n'||data[4] == ' ')) {
    int i = 0;
    int s = 0;
    Serial.print("PARSE:");
    Serial.println((const char*)data);
    while (i < size) {
      for (; i < size; i++) {
        if (data[i] == '\n') {
          parse2((const char*)data + s, i - s, subscribeSequance);
          s = i + 1;
          i++;
        }
      }
    }
  }
}

void SimpleMQTT::handleEvents(void (cb)(const char *, const char*)) {
  publishCallBack = cb;
}

bool SimpleMQTT::send(const char *mqttMsg, int len, uint32_t replyId) {
  static SimpleMQTT *myself = this;
  Serial.print("Send:\"");
  Serial.print(mqttMsg);
  Serial.println("\"");


  if (replyId == 0) {
    bool status = espNowFloodingMesh_sendAndWaitReply((uint8_t*)mqttMsg, len, ttl, 3, [](const uint8_t *data, int size) {
      if (size > 0) {
        myself->parse(data, size, 0, true); //Parse simple Mqtt protocol messages
      }
    }); //Send MQTT commands via mesh network
    if (!status) {
      //Send failed, no connection to master??? Reboot ESP???
      Serial.println("Timeout");
      return false;
    }
    return status;
  } else {
    espNowFloodingMesh_sendReply((uint8_t*)mqttMsg, len, ttl, replyId);
  }
}

const char* SimpleMQTT::decompressTopic(const char*topic) {
  static char t[100]={0};
  static char b[100];
  if(topic[0]!='.') {
      memcpyS(t,sizeof(t),topic,strlen(topic)+2);
      return t;
  }
  int c = 0;
  for(c=0;c<strlen(topic)&&topic[c]=='.';c++);

  int index=0;
  for(int i=0;i<strlen(t);i++) {
     if(t[i]=='/') {
          index++;
          if(index==c) {
              index=i;
              break;
          }
      }
  }
  memcpyS(b,sizeof(b),t,index);
  memcpyS(b+index,sizeof(t)-index, topic+c, strlen(topic)+1);
  memcpyS(t,sizeof(t),b,strlen(b)+1);
  return b;
}


void SimpleMQTT::parse2(const char *c, int l, bool subscribeSequance) {
  if (l>4&&c[0] == 'P' && c[1] == ':') { //publish
    char topic[100];
    char value[100];
    int i = 2;
    for (; (i < l) && c[i] != ' '; i++); //find :
    if (i != l) { //found
      if (i > sizeof(topic)) {
        Serial.print("Invalid Topic length:");
        Serial.println(l);
        return;
      };
      memcpyS(topic,sizeof(topic), c + 2, l - 2);
      topic[i - 2] = 0;
      memcpyS(value, sizeof(value),c + i + 1, l - i);
      value[l - i - 1] = 0;
      const char *decompressedTopic = decompressTopic(topic);
      for (char* subscribed_topic : topicVector) {
        if (strcmp(subscribed_topic, decompressedTopic) == 0) {
            this->_topic = decompressedTopic;
            this->_value = value;
            publishCallBack(decompressedTopic, value);
            this->_topic = NULL;
            this->_value = NULL;
          if (replyId) {
            //Reply/Ack requested
            send("ACK", 4, replyId);
          }
          break;
        }
      }
    }
  }
}
