void H1_SendCommand(String cmd);
void Status2Msg();

//======================================== M Q T T ==========================================
void mqttpub (String topic, String msg)  // Publish MQTT data
{
  if (!setting[MQTT_ENAB].toInt() || !state_mqtt_init) {return;}
  String s;
  const char *m;
  const char *top;
            
  //s = String(g_wifi_mac) + "/" + topic;
  s = "lyfco/" + topic;
  top = s.c_str(); 
  m = msg.c_str();
  
  trace("MQTT Pub: " + s + ", P:" + msg,"VB SER LF");
  client.publish(top, m, false);
  
 
}

//-------------------------------------------------------------------------------------------

void mqtt_callback(char* topic, byte* payload, unsigned int length) { // Incomming subscribed MQTT messages 
  String top, pay;
  bool Hit=0;
  byte i;

  for (int i = 0; i < length; i++) {pay += (char)payload[i]; }
  top = String(topic);

  if (top =="lyfco/set") 
            {   
              H1_SendCommand(pay);
              Hit=true;
              trace("MQTT set: " + top + ", P:" + pay,"SER LOG LF");
            }
                    
       
   if (!Hit){
      trace("Unknown MQTT message: " + top + ", P:" + pay,"SER LOG LF");
            
     }
 
}

//-------------------------------------------------------------------------------------------

void reconnect() {  // MQTT reconnect if loosing network
  
    
  if (FailedMQTTConnectCounter>10) FailedMQTTConnectCounter++;  // Connecta en gång per min i 10 min sedan en gång per timme.
  if (!setting[MQTT_ENAB].toInt() || (FailedMQTTConnectCounter>10 && FailedMQTTConnectCounter<60)) return;
  
  char m[20];
     
    //trace("Attempting MQTT connection...  ","SER LOG");
    // Attempt to connect
    if (client.connect(g_wifi_mac, setting[MQTT_USER].c_str(), setting[MQTT_PASS].c_str())) {
      //if (client.connect(g_wifi_mac, cfg_MQTT_USER, cfg_MQTT_PASS)) {
   //connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession);
   
        trace("MQTT Connected with user/pass","SER LOG LF");
        FailedMQTTConnectCounter = 0;
           
        // ... and resubscribe
        
        sprintf(m,"lyfco/set",g_wifi_mac); 
        //sprintf(m,"%s/ALARM/SET/#",g_wifi_mac); 
        client.subscribe(m);
        
        state_mqtt_init = true;
            
        } 
         else 
        {
        //Serial.print("failed, rc="); //Serial.print(client.state());
       
        state_mqtt_init = false;
        trace("Failed MQTT connect attempt: " + String(FailedMQTTConnectCounter), "SER LOG LF");
        FailedMQTTConnectCounter++;
        if (FailedMQTTConnectCounter>60) FailedMQTTConnectCounter=11;  // Försök igen om 50 min
        
    }
 
}

//=====================================================================================================

void mqtt_pub_all()   // Send all values on request DUMP
{
  if (!setting[MQTT_ENAB].toInt() && state_mqtt_init) return;
   
  mqttpub ("battery", String(float(l_Volt)/100)) ;
  mqttpub ("battprocent", String(l_BattProcent)) ;
  mqttpub ("status", l_Status) ;
  mqttpub ("statustxt", l_StatusTxt) ;
  mqttpub ("chargetime", String(l_ChargeTime)) ;
  mqttpub ("runtime", String(l_RunTime)) ;
  mqttpub ("rssi", String(g_wifi_rssi)) ;
  mqttpub ("rssinum", g_wifi_rssi_num) ;
  mqttpub ("mode", l_Mode) ;
  mqttpub ("alarm", String(l_Alarm)) ;


  
    

}
