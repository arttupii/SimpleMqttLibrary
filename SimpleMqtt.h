#ifndef ___SIMPLE_MQTT_H_
#define ___SIMPLE_MQTT_H_
#include<map>
#include<list>
#include<Arduino.h>
typedef enum {
  SUBSCRIBE,
  UNSUBSCRIBE,
  GET,
  PUBLISH
} Mqtt_cmd;
typedef enum {
  SWITCH_ON,
  SWITCH_OFF
} MQTT_switch;
typedef enum {
  TRIGGERED
} MQTT_trigger;
typedef enum {
  CONTACT_OPEN,
  CONTACT_CLOSED
} MQTT_contact;
typedef enum {
  SHUTTER_OPEN,
  SHUTTER_CLOSE,
  SHUTTER_STOP
} MQTT_shutter;

typedef enum {
  SET,
  VALUE
} MQTT_IF;

class SimpleMQTT {
    public:
        SimpleMQTT(int ttl, const char *myDeviceName);
        ~SimpleMQTT();
        bool publish(const char* deviceName, const char* parameterName, const char *value);

        bool getTopic(const char* devName, const char *valName);
        bool subscribeTopic(const char* devName, const char *valName);
        bool listenTopic(const char* devName, const char *valName); //Listen topics without subscribing
        bool unsubscribeTopic(const char* devName, const char *valName);

        void parse(const unsigned char *data, int size, uint32_t replyId, bool subscribeSequance=false);
        const char* getBuffer();

        void handleEvents(void (*cb)(const char *, const char*));

        bool compareTopic(const char* topic, const char* deviceName, const char* t);

        bool _switch(Mqtt_cmd cmd, const char* name, MQTT_switch value=SWITCH_ON);
        bool _temp(Mqtt_cmd cmd, const char* name, float value=0);
        bool _humidity(Mqtt_cmd cmd, const char* name, float value=0);
        bool _trigger(Mqtt_cmd cmd, const char* name, MQTT_trigger value=TRIGGERED);
        bool _contact(Mqtt_cmd cmd, const char* name, MQTT_contact value=CONTACT_OPEN);
        bool _dimmer(Mqtt_cmd cmd, const char* name, uint8_t value=0);
        bool _string(Mqtt_cmd cmd, const char* name, const char *value=NULL);
        bool _number(Mqtt_cmd cmd, const char* name, int min=0, int max=0, int step=0);
        bool _float(Mqtt_cmd cmd, const char* name, float value=0);
        bool _int(Mqtt_cmd cmd, const char* name, int value=0);
        bool _shutter(Mqtt_cmd cmd, const char* name, MQTT_shutter value=SHUTTER_OPEN);
        bool _counter(Mqtt_cmd cmd, const char* name, int value=0);
        bool _bin(Mqtt_cmd cmd,  const char* name, const uint8_t* data=0, int len=0);


        bool _ifSwitch(MQTT_IF ifType, const char* name, void (*cb)(MQTT_switch /*value*/));
        bool _ifTemp(MQTT_IF ifType, const char* name, void (*cb)(float /*value*/));
        bool _ifHumidity(MQTT_IF ifType, const char* name, void (*cb)(float /*value*/));
        bool _ifTrigger(MQTT_IF ifType, const char* name, void (*cb)(MQTT_trigger /*value*/));
        bool _ifContact(MQTT_IF ifType, const char* name, void (*cb)(MQTT_contact /*value*/));
        bool _ifDimmer(MQTT_IF ifType, const char* name, void (*cb)(uint8_t /*value*/));
        bool _ifString(MQTT_IF ifType, const char* name, void (*cb)(const char */*value*/));
        bool _ifNumber(MQTT_IF ifType, const char* name, void (*cb)(int /*min*/, int /*max*/, int /*step*/));
        bool _ifFloat(MQTT_IF ifType, const char* name, void (*cb)(float /*value*/));
        bool _ifInt(MQTT_IF ifType, const char* name, void (*cb)(int /*value*/));
        bool _ifShutter(MQTT_IF ifType, const char* name, void (*cb)(MQTT_shutter /*value*/));
        bool _ifCounter(MQTT_IF ifType, const char* name, void (*cb)(int /*value*/));
        bool _ifBin(MQTT_IF ifType, const char* name, void (*cb)(const uint8_t */*bin*/,int /*length*/));
    private:
        String myDeviceName;
        char buffer[250];
        uint32_t replyId;
        bool _raw(Mqtt_cmd cmd, const char* type, const char* name, const char *value);
        bool _rawIf(MQTT_IF ifType,const char* type, const char* name);
        void (*publishCallBack)(const char *topic, const char* value);

        void parse2(const char *c,int l, bool subscribeSequance);
        bool send(const char *mqttMsg, int len, uint32_t replyId);

        std::list<char*> topicVector;
        void addtopicToVector(const char *topic);
        void removeTopicFomVector(const char *topic);
        int ttl;

        const char *_topic;
        const char* _value;
};
#endif
