#define SLAVESENSOR A5
#define MASTERSENSOR A4
#define BLACK_THRESHOLD 30

/*
 * Declares possible MODES for the TV
 */
enum IMG_MODE {
  BLACK,
  LIVE,
  FREEZE
};


enum SSENSITIVITY{
  NORMAL,
  EXTREME  //needs bigger changes to return LIVE
};

struct __livestatus {
  IMG_MODE status;
  int  delta;
  float average;
  float derivate;
};


/*
 * Analyses the TV state for twait ms
 * and returns a __livestatus object
 */
__livestatus getTVStatus(int twait, SSENSITIVITY sensitivity) {
  unsigned int dt = twait;
  __livestatus rstatus;
  rstatus.delta = twait;
  unsigned int nloop = dt / 10;
  float sum1 = 0;
  float sum2 = 0;
  int maxDeriv1 = 0;
  int maxDeriv2 = 0;
  int v1 = 0, v2 = 0;
  int oldv1 = 0, oldv2 = 0;
  float deriv1 = 0, deriv2 = 0;
  int minV1 = 1024, minV2 = 1024, maxV1 = 0, maxV2 = 0;
  for (unsigned int i = 0; i <= nloop; i++) {
    v1 = analogRead(MASTERSENSOR); //master
    v2 = analogRead(SLAVESENSOR);
    if (i != 0) {
      maxDeriv1 = (abs(oldv1 - v1) > maxDeriv1) ? abs(oldv1 - v1) : maxDeriv1;
      maxDeriv2 = (abs(oldv2 - v2) > maxDeriv2) ? abs(oldv2 - v2) : maxDeriv2;
      
      maxV1 = (v1 > maxV1) ? v1 : maxV1;
      maxV2 = (v2 > maxV2) ? v2 : maxV2;
      
      minV1 = (v1 < minV1) ? v1 : minV1;
      minV2 = (v2 < minV2) ? v2 : minV2;
      
      deriv1 += abs(oldv1 - v1);
      deriv2 += abs(oldv2 - v2);
    } else {
      minV1 = v1;
      maxV1 = v1;
      minV2 = v2;
      maxV2 = v2;
    }
    sum1 += v1;
    sum2 += v2;
    oldv1 = v1;
    oldv2 = v2;
    delay(10);
  }
  sum1 /= nloop;
  sum2 /= nloop;
  deriv1 /= nloop;
  deriv2 /= nloop;
  IMG_MODE state;
//  Serial.println(sensitivity);
//  Serial.println("sum");
//  Serial.println(sum1);
//  Serial.println(sum2);
//  Serial.println("deriv");
//  Serial.println(deriv1);
//  Serial.println(deriv2);
//  Serial.println("maxderiv");
//  Serial.println(maxDeriv1);
//  Serial.println(maxDeriv2);
//  Serial.println("maxDiff");
//  Serial.println(maxV1 - minV1);
//  Serial.println(maxV2 - minV2);
  if(sensitivity == NORMAL){
    if (deriv1 <= 1.2 && sum1 <= BLACK_THRESHOLD && deriv2 <= 1.2 && sum2 <= BLACK_THRESHOLD) {
      rstatus.status = BLACK;
    } else if (deriv1 <= 2.0 && maxDeriv1 <= 5 &&
               deriv2 <= 2.0 && maxDeriv2 <= 5 &&
               maxV1 - minV1 <= 6 && maxV2 - minV2 <= 6) {
      rstatus.status = FREEZE;
    } else {
      rstatus.status = LIVE;
    }  
  //measure with different criteria to return movement
  }else if(sensitivity == EXTREME){
    if (deriv1 <= 1.2 && sum1 <= BLACK_THRESHOLD*2 && deriv2 <= 1.2 && sum2 <= BLACK_THRESHOLD*2) {
      rstatus.status = BLACK;
    } else if ( (deriv1 <= 3.0 && maxDeriv1 <= 10 && maxV1 - minV1 < 60) ||
                (deriv2 <= 2.0 && maxDeriv2 <= 2 && maxV2 - minV2 < 5)) 
    {
      rstatus.status = FREEZE;
    } else {
      rstatus.status = LIVE;
    }
  }
  rstatus.average = (sum1 + sum2) / 2;
  rstatus.derivate = (deriv1 + deriv2) / 2;
  return rstatus;
}


/*
 * Callback used to retrieve the TV status after analyzing it
 * for dt miliseconds
 */
void stbState(EthernetClient *client, char args[]) {
  unsigned int dt = JSONParser::getInt(args, "\"dt\"");
  //this will be used as a circular buffer with size = 10
  if (dt > 10000) {
    client->println("{\"error\":1,\"message\":\"Max time exceeded\"}");
    return;
  }
  __livestatus rstatus = getTVStatus(dt,NORMAL);

  client->print("{\"status\":");
  if (rstatus.status == BLACK)
    client->print("\"BLACK\"");
  else if (rstatus.status == LIVE)
    client->print("\"LIVE\"");
  else if (rstatus.status == FREEZE)
    client->print("\"FREEZE\"");
  client->print("}");
}


/*
 * Returns transitions in the state of the set top box
 */
void getLiveStatus(EthernetClient *client, char args[]) {
  unsigned int dt = JSONParser::getInt(args, "\"dt\"");

  if (dt > 10000) {
    client->println("{\"error\":1,\"message\":\"Max time exceeded\"}");
    return;
  }
  //number of 100ms intervals
  int nPlages = dt / 100;
  vector<__livestatus> vstatus;

  for (int j = 0; j < nPlages; j++) {
    vstatus.push_back(getTVStatus(100, NORMAL));
    
    //check if we should merge
    if(j > 0){
       int pos1 = vstatus.size()-2;
       int pos2 = vstatus.size()-1;
       if (vstatus[pos1].status == LIVE && vstatus[pos2].status == LIVE) {
         vstatus[pos1].average = (vstatus[pos1].average * vstatus[pos1].delta + vstatus[pos2].average * vstatus[pos2].delta) / (vstatus[pos1].delta + vstatus[pos2].delta);
         vstatus[pos1].delta += vstatus[pos2].delta;
         vstatus.remove(pos2);
      } else if ( (vstatus[pos1].status == LIVE && vstatus[pos2].status == FREEZE) ||
                  (vstatus[pos1].status == FREEZE && vstatus[pos2].status == LIVE)) {
         vstatus[pos1].average = (vstatus[pos1].average * vstatus[pos1].delta + vstatus[pos2].average * vstatus[pos2].delta) / (vstatus[pos1].delta + vstatus[pos2].delta);
         vstatus[pos1].status = LIVE;
         vstatus[pos1].delta += vstatus[pos2].delta;
         vstatus.remove(pos2);
      } else if (vstatus[pos1].status == FREEZE && vstatus[pos2].status == FREEZE) {
        if (abs(vstatus[pos1].average - vstatus[pos2].average) > 2)
          vstatus[pos1].status == LIVE;
        vstatus[pos1].average = (vstatus[pos1].average * vstatus[pos1].delta + vstatus[pos2].average * vstatus[pos2].delta) / (vstatus[pos1].delta + vstatus[pos2].delta);
        vstatus[pos1].delta += vstatus[pos2].delta;
        vstatus.remove(pos2);
      } else if (vstatus[pos1].status == BLACK && vstatus[pos2].status == BLACK) {
        vstatus[pos1].average = (vstatus[pos1].average * vstatus[pos1].delta + vstatus[pos2].average * vstatus[pos2].delta) / (vstatus[pos1].delta + vstatus[pos2].delta);
        vstatus[pos1].delta += vstatus[pos2].delta;
        vstatus.remove(pos2);
      }
    }    
  }

  //create JSON
  char str[10];
  client->print("{");
  client->print("\"result\":[");
  for (int i = 0; i < vstatus.size(); i++) {
    client->print("{\"plage\":");
    client->print(vstatus[i].delta);
    client->print(",\"status\":");
    if (vstatus[i].status == BLACK)
      client->print("\"BLACK\",");
    else if (vstatus[i].status == LIVE)
      client->print("\"LIVE\",");
    else if (vstatus[i].status == FREEZE)
      client->print("\"FREEZE\",");
    client->print("\"avgvalue\":");
    dtostrf(vstatus[i].average, 1, 2, str);
    client->print(str);
    client->print(",\"avgderiv\":");
    dtostrf(vstatus[i].derivate, 1, 2, str);
    client->print(str);
    client->print("}");
    if (i != vstatus.size() - 1)
      client->print(",");
  }
  client->print("]}");
}


struct dataZap{
  bool isZapping;
  unsigned long nFramesLow;
  unsigned int nFramesHigh;
};

void zapTCallback(){
  dataZap *data = (dataZap *) TimeInterruption::data;
  int value = analogRead(A4);
  TimeInterruption::nTimes++;
  //will search for a high luminosity level
  if(data->isZapping){
      if(value < BLACK_THRESHOLD){
        data->nFramesLow++;
        data->nFramesHigh = 0;
      }else{
        data->nFramesHigh++;
      }
  }else{
      if(value < BLACK_THRESHOLD){
        data->nFramesLow++;
      }else{
        data->nFramesLow = 0;
      }
      if(data->nFramesLow > 3){
        data->isZapping = true;
      }
  }
  //finished zapping!
  if(data->nFramesHigh > 3){
    TimeInterruption::removeInterruption();  
  }
  //15 seconds is the limit
  if(TimeInterruption::nTimes > 300){
    TimeInterruption::nTimes = -1;
    TimeInterruption::removeInterruption();
  }
}

/* ZapTime measurement routine,
 *  this will execute a zap and then measure the
 *  time it was taken to execute it.
 */
void dozap(EthernetClient *client, char args[]){
  EthernetClient zapclient;
  byte dec_ip[4];
  int channel;
  //get IP
  JSONParser::getIP(args, "\"ip_box\"", dec_ip );
  //get channel number
  //channel = JSONParser::getInt(args, "\"channel\"");
  channel = JSONParser::getIntFromStr(args, "\"channel\"");
  zapclient.stop();
  int code = zapclient.connect(dec_ip, 8080);
  if (code) {
    Serial.println("connected");
    zapclient.println("POST /message HTTP/1.1");
    zapclient.print("Content-Length: ");
    zapclient.println(45 + channel/10);
    zapclient.println("Accept: */*");
    zapclient.println("Content-Type: application/json");
    zapclient.println("");
    zapclient.print("{\"action\":\"zap\",\"params\":{\"channelNumber\":");
    zapclient.print(channel);
    zapclient.println("}}");
    zapclient.flush();
    zapclient.stop();
    //wait for the end of zapping
    dataZap dataLive;
    dataLive.isZapping = false;
    dataLive.nFramesLow = 0;
    dataLive.nFramesHigh = 0;
    TimeInterruption::data = (void *) &dataLive;
    TimeInterruption::startCount(&zapTCallback);
    Serial.println("Starting timer!");    
    TimeInterruption::wait();
    if(TimeInterruption::nTimes != -1){
      client->print("{\"done\":1,\"ms\":");
      char buf[20];
      TimeInterruption::getMs(buf);
      client->print(buf);
      client->println("}");
    }else{
      client->println("{\"error\":1,\"msg\":\"time limit exceeded\"}");
    }
  } else {
    client->println("{\"error\":1,\"msg\":\"cannot connect\"}");
  }
}

/*
 * Measures WAKEUP time for a decoder
 * 
 * This method only uses the slave sensor to avoid having problems
 * when OFF screen in the TV has a message moving in the center (Samsung)
 */
unsigned long wakeupTime(){
  byte dec_ip[4];
  unsigned long tStart = millis();
  bool finished = false;
  bool error = false;
  int livecount = 0;
  while(finished == false){
    __livestatus state = getTVStatus(500, EXTREME);
    if(state.status == LIVE && 
       millis() - tStart > (unsigned long)30000){
        livecount++;
        if(livecount == 4){
          finished = true;
        }
    }else if(state.status == FREEZE && state.average > 400){
        livecount++;
        if(livecount == 4){
          finished = true;
        }
    }else{
        livecount = 0;
    }
    //time limit is 6 minutes
    if(millis() - tStart > (unsigned long)6*60000){
      error = true;
      finished = true;
    }
  }
  tStart = millis() - tStart;
  if(error){
    return 0;  
  }else{
    return tStart;  
  }
}
