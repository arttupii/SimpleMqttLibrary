#ifndef ___SIMPLE_MQTT_H_
#define ___SIMPLE_MQTT_H_
#include<map>
#include<list>
#include<Arduino.h>
class SimpleMQTT {
    public:
        SimpleMQTT(int ttl, const char *myDeviceName);
        ~SimpleMQTT();
        bool publish(const char* deviceName, const char* parameterName, const char *value);

        bool getTopic(const char* devName, const char *valName);
        bool subscribeTopic(const char* devName, const char *valName);
        bool unsubscribeTopic(const char* devName, const char *valName);

        void parse(const unsigned char *data, int size, uint32_t replyId, bool subscribeSequance=false);
        const char* getBuffer();

        void handleSubscribeAndGetEvents(void (*cb)(const char *, const char*));
        void handlePublishEvents(void (*cb)(const char *, const char*));

        bool compareTopic(const char* topic, const char* deviceName, const char* t);

    private:
        String myDeviceName;
        char buffer[100];
        uint32_t replyId;

        void (*subscribeCallBack)(const char *topic, const char* value);
        void (*publishCallBack)(const char *topic, const char* value);

        void parse2(const char *c,int l, bool subscribeSequance);
        bool send(const char *mqttMsg, int len, uint32_t replyId);

        std::list<char*> topicVector;
        void addtopicToVector(const char *topic);
        void removeTopicFomVector(const char *topic);
        int ttl;
};
#endif
