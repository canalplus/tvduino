
bool setPower(int gate, bool isOn, byte switchIP[]){
  EthernetClient switchCl;
  switchCl.stop();
  int code = switchCl.connect(switchIP, 80);
  Serial.println(code);
  if (switchCl.connected()) {
    Serial.println("connected");
    switchCl.println("POST /hidden.htm HTTP/1.1");
    switchCl.println("Connection: keep-alive");
    switchCl.println("Content-Type: text/plain;charset=UTF-8");
    switchCl.print("Content-Length: ");
    if(isOn){
      switchCl.println('8');
    }else{
      switchCl.println('9');
    }
    switchCl.println("Accept: */*");
    switchCl.println("Content-Type: application/json");
    switchCl.println("");
    switchCl.print("M0:O"); 
    switchCl.print(gate);//1-4
    switchCl.print("=");
    if(isOn){
      switchCl.print("On");
    }else{
      switchCl.print("Off");
    }
    switchCl.flush();
    switchCl.stop();
    return true;
  }else{
    return false;
  }
}



void power(EthernetClient *client, char args[]) {
  //int switchIn = JSONParser::getInt(args,"\"nplug\"");
  int switchIn = JSONParser::getIntFromStr(args, "\"nplug\"");
  byte sip[4];
  JSONParser::getIP(args, "\"ip\"", sip);

  //turn off!
  bool reply = setPower(switchIn, false, sip);
  delay(2000);
  reply = reply && setPower(switchIn, true, sip);
  unsigned long twake = wakeupTime();
  if(switchIn > 0 && switchIn < 5){
    if(reply){
      if(twake != 0){
        client->print("{\"done\":1, \"ms\":");
        client->print(twake);
        client->println("}");  
      }else{
      
      }
    }else{
      client->println("{\"error\": 1,\"msg\":\"cannot connect\"}");  
    }
  }else{
    client->println("{\"error\": 1,\"msg\":\"Invalid plug (1-4)\"}");  
  }
  
}
