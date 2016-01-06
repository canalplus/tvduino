# TVduino

TVduino is a REST Server implemented on an Arduino. It is used for monitoring the state of a set-top box through its video output. The REST Server can be used as a library to create connected solutions with arduino.

To put in place this solution you will need:

* 1 Arduino
* 2 TinkerKit LDR Sensors (Can be substituted by a normal photoresistor)
* 1 Ethernet Shield

<img src="http://i.imgur.com/eLKfb3X.png" width="300">

You can then fix the sensors to a TV using a box and 
There are three main routes implemented on the arduino REST, they can be accessed through 10.0.2.160:80

| page  | method  | parameters  | return  |
|---|---|---|---|
| /video  | GET | none  |  Readings of both sensors  |
| /getlivestatus  | POST  | {"dt" : ms}  |  Transitions in the TV status |
| /status  |  POST | {"dt" : ms} |  Current TV status |
| /zap  |  POST | {"ip_box" : "STB_IP" , "channel":"NChannel", "threshold":"LOW_THRESHOLD"} |  Measures zapping time for a STB (dev) |
| /wakeup  |  POST | {"nplug" : "N_PLUG", "ip":"Ip_ePowerSwitch"} |  Measures electrical wakeup time for a STB|


Example: */getlivestatus*

Request (POST):
```json
{"dt":5000}
```

Reply:
```json
{
  "result": [
    {
      "plage": 1400,
      "status": "LIVE",
      "avgvalue": 399.93,
      "avgderiv": 0.35
    },
    {
      "plage": 1900,
      "status": "BLACK",
      "avgvalue": 23.45,
      "avgderiv": 0.2
    },
    {
      "plage": 1700,
      "status": "LIVE",
      "avgvalue": 300.19,
      "avgderiv": 17.05
    }
  ]
}
```
Showing us that there were three transitions on the TV signal and the time it stayed in each state.

TVduino can also be easily customized to add different routes to your rest server and be used for different needs.
To create new routes it is really simple:

```c++
void callback(EthernetClient *client, char args[]){
  client->println("{\"hello\":\"world\"}"); 
}

void setup() {
  Serial.begin(9600);
  myServer = new restServer(mac, ip, gateway, subnet,80);
  delay(1000);
  //create routes on our rest server
  myServer->addRoute("/test", GET, &callback);
}

void run(){
  myServer->serve();
}
```

Requires [Timer Library](http://playground.arduino.cc/Code/Timer1) and [Arduino IDE](https://www.arduino.cc/en/Main/Software)
