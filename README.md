## MQTT library to flooding mesh network

example project: https://github.com/arttupii/EspNowFloodingMesh/tree/master/arduinoSlaveNode/main


### Protocol messages:
#### Subscribe topic device1/led/value
```
"MQTT myDeviceName"
S:device1/led/value
"
```

#### Unsubscribe topic device1/led/value
```
"MQTT myDeviceName"
U:device1/led/value
"
```

#### Get topic device1/led/value without subscribing
```
"MQTT myDeviceName"
G:device1/led/value
"
```

#### Publish topic device1/led/value
```
"MQTT myDeviceName"
P:device1/led/value on
"
```
