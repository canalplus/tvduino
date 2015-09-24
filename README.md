# TVduino

TVduino is a REST API interface used to monitor the state of a set-top box using its video output.

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

Output example of */getlivestatus*

Body:
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

TVduino can also be easily customized to add different routes to your rest server. And even be used for different needs.
