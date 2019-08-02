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
#### Multiple MQTT commands in the same message
```
"MQTT myDeviceName"
G:device1/switch/led/value
S:device1/switch/led/set
G:device1/switch/led1/value
S:device1/switch/led1/set
G:device1/temp/dallas1/value
G:device1/temp/dallas2/value
G:device2/switch/led/value
S:device2/switch/led/set
"
```
##### "Compressed" message
```
"MQTT myDeviceName"
G:device1/switch/led/value     -->Topic is device1/switch/led/value
S:.../set                      -->Topic is device1/switch/led/set
G:../led1/value                -->Topic is device1/switch/led1/value
S:.../set                      -->Topic is device1/switch/led1/set
G:./temp/dallas1/value         -->Topic is device1/temp/dallas1/value
G:../dallas2/value             -->Topic is device1/temp/dallas2/value
G:device2/switch/led/value     -->Topic is device2/switch/led/value
S:.../set                      -->Topic is device2/switch/led/set
"
```
