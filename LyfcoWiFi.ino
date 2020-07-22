#include <ESP8266WiFi.h>       
#include <WiFiClient.h>         // Webserver
#include <ESP8266WebServer.h>   // Webserver
#include <ESP8266HTTPClient.h>  // Web Client
#include <time.h>               // time() ctime()
#include <PubSubClient.h>       // MQTT Library
#include <ESP8266mDNS.h>        // Include the mDNS library
#include <ArduinoJson.h>        // Json parser support
#include <ESP8266httpUpdate.h>  // HTTP Remote firmware update
#include <FS.h>                 // SPIFFS SPI-Flash Filsystem, används för att lagra/läsa H1 images för uppdateringar
#include "HD_Globals.h"         // Globara variabler och konstanter

#include <SoftwareSerial.h>     // Serial port for Lyfco

os_timer_t myTimer;                      // Init soft interrupt timer
WiFiClient Online_Mqtt_Client;           // Init MQTT 
PubSubClient client(Online_Mqtt_Client); // Init MQTT 

SoftwareSerial H1(13, 12, false, 1024);  // Init software serial  Pin (RX D7)13, (TX D6)12, non inv, buff for lyfco serial
SoftwareSerial H2(14, 16, false, 1024);  // Init software serial  Pin (RX D5)14, (TX D0)16, non inv, buff for lyfco from Module/APP

#include "HD_Utilities.h"       // Diverse funktioner: Eprom, Oled
#include "HD_Online.h"          // online calls / logging
#include "HD_MQTT.h"            // MQTT 
#include "HD_Webserver.h"       // Intern webserver
#include "HD_LyfcoComm.h"       // Serial Lyfco
#include "Ultra.h"

//===============================================================================
void setup() { // ESP Boot  
  String t;
  digitalWrite(LED_BUILTIN, LOW);   // Lys under init, blinka när Wifi Conn
  SPIFFS.begin();                   // Initiera filsystem
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  Serial.begin(115200);             // Debug serial usb setup
  H1.begin(115200);                  // H1 soft serial setup
  H2.begin(115200);                  // H1 soft serial setup
  //init_eeprom(); read_eeprom();     // Initiera Epromrutiner och läs in settings
  settings_rw(0);                   // Läs in settings
  
  Serial.println("");  
  Serial.println("-------------------------------------------");  
  Serial.println("    LYFCO WIFI  (C) Peter Hansson 2020 ");
  Serial.println("-------------------------------------------");  
  Serial.println("\n-- Startup --");


 /* crc("##18F40004111110ccEE");
  crc("##35W00012301762863000000000000001195");
  crc("##35W00012301762863000000000000001194");
*/
 // crc("##35W0001230176260900000000000000F996");
 
  
  // ----------- WIFI CONNECT --------------
  trace("Wifi connecting to: " + setting[WIFI_SSID],"SER LOG LF");
  
  if (setting[DHCP_ENAB]=="0") {  // OM FIXED IP 
    IPAddress ip, gateway, subnet, dns;
    ip.fromString(setting[FIXED_IP]);
    gateway.fromString(setting[FIXED_GW]);
    subnet.fromString(setting[FIXED_SUB]);
    dns.fromString(setting[FIXED_DNS]);
    WiFi.config(ip, dns, gateway, subnet);
    }
    
  WiFi.mode(WIFI_STA);
  byte i=0;
  const char *ssid, *pass;
  ssid = setting[WIFI_SSID].c_str(); pass = setting[WIFI_PASS].c_str();
  WiFi.begin(ssid, pass); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (i++>25) break;   // Försök 25ggr
  }
  //WiFi.softAP("ESP", "12345678");  // WiFi hotspot, funkar men Gateway blir fel så då dör vanlig access utifrån branvägg
  if (WiFi.status() == WL_CONNECTED) {trace("Wifi connected!","SER LOG LF");state_Wifi=1;} else trace("Giving up, no Wifi connection!","SER LOG LF");
  digitalWrite(LED_BUILTIN, HIGH);   // Släck nät Wifi Int process är över
     // ----------- GET IP, RSSI, MAC --------------
  rssi_update(); 
  trace("Signal strength: " + String(g_wifi_rssi),"SER LOG LF");
  sprintf(g_wifi_ip,"%d.%d.%d.%d", WiFi.localIP()[0],WiFi.localIP()[1],WiFi.localIP()[2],WiFi.localIP()[3]);  
   if (setting[DHCP_ENAB]=="0")  trace("Fixed IP address: " + String(g_wifi_ip), "SER LOG LF");  else trace("DHCP IP address: " + String(g_wifi_ip), "SER LOG LF");
  byte mac[8];
  WiFi.macAddress(mac); 
  sprintf(g_wifi_mac,"%02X%02X%02X%02X%02X%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);   tolow(g_wifi_mac); trace("MAC address: " + String(g_wifi_mac),"SER LOG LF");
  

// ----------- Enable multicast DNS (mDNS-SD) for Athom Homey --------------

      
      if (state_Wifi)
            
            if (!MDNS.begin("KLIPP")) { // Start the mDNS responder for H60.local
                   Serial.println("Error setting up MDNS responder!");
                  }
                  else
                  {
                    Serial.println("mDNS-SD responder started");
                    MDNS.addService("http", "tcp", 80);  // Extend to mDNS-SD for Athom Homey
                  }




  // ----------- NTP CLOCK INIT --------------
  configTime(TZ_SEC, DST_SEC, "pool.ntp.org");
  tid(2); // Test NTP connection and report
  
  
  // ----------- INIT MQTT --------------
  if (setting[MQTT_ENAB].toInt())
     {
      client.setServer(setting[MQTT_SRVR].c_str(), setting[MQTT_PORT].toInt());
      client.setCallback(mqtt_callback); //Info connect to dual MQTT brokers: 
      //reconnect() ;
      }
  

  // ---------------- WEBSERVER ------------------
  if (setting[WEB_ENABLED].toInt()) setup_webserver();      

  // --------- INTERUPT TIMER INIT ------------------
  os_timer_setfn(&myTimer, timerCallback, NULL);    
  os_timer_arm(&myTimer, 1000, true);

  // --------- MISQ ------------
  init_eeprom();
  read_eeprom();
  
  t = "ESP Start reason: " + ESP.getResetReason();trace (t,"SER LOG LF");

  // --------- PORTAR ------------
      
   //ultrasetup();

   H1_SendCommand("lstat"); 
   
  

}
//---------------------------------------------------------------------------------------

void timerCallback(void *pArg) {

  if (state_Wifi==true) blinkLed();
  if (secondsCount++>=1) every1seconds = true;
  if (mincount++>=60)  onemin=true;
  web_session_loop();
  


} // End of timerCallback

//---------------------------------------------------------------------------------------


void loop() {
  int i;
  bool Hit;
  // --- Fast loop  --

 //handle_RC();      // Radio data in
  
  H1_Recv();                                   // Receive H1 data
  H1_HandleData();                             // Handle new data from interface
  H2_Recv();                                   // Receive H2 (RX from module app)
  H2_HandleData();                             // Handle new data from interface
  

  SerialDebugRX();         // Läs av data in på serial debug port
  if (setting[WEB_ENABLED].toInt()) webserver_loop();     // Handle Webserver requests
  if (state_mqtt_init) { client.loop(); }     // MQTT LOOP READ
  
  // Led blink status.  ON=Init, Quickblink= WifiOK, MQTT not, One sec blink=All ok

  if (state_Wifi) 
                if (blinkLedCount++ > 1000) {blinkLed(); blinkLedCount=0;}

  
  if (onemin){ // Kör varje ny minut utanför timer rutin då den krashar annars.
    //Serial.print("Time since start: ");Serial.println(tid(0));
    //Serial.print("Minute:"); Serial.println(int(stored_minute_ptr));

    
    rssi_update(); 
    H1_SendCommand("lstat"); 
    mqtt_pub_all(); // refresh

   
    
   uptime++;
   mincount=0;
   onemin=false; 
   }
   
  
   if (every1seconds)  {
        //Serial.print("+");
        every1seconds=0;
        secondsCount=0;

        //statRuns++;write_eeprom();
        
        //ultra_ping(); // Mät klipparastånd
        
        // Connect / Reconnect to MQTT Broker
        if (setting[MQTT_ENAB].toInt())  if (!client.connected()) {reconnect();}    // MQTT Reconnect om avbrott till server
        
        mqtt_pub_all();
  
   }
 
}
  

//---------------------------------------------------------------------------------------

void SerialDebugRX(){   // Data mottages på debugport
  byte i;
  bool hit=0;
  char c;
  static String rxbuff;
 
  if(Serial.available())
    {
      c = (char)Serial.read();
      if (c!=10 && c!=13) {rxbuff += c; return;}
            
                
      if (rxbuff.substring(0,3) == "CFG" || rxbuff.substring(0,3) == "cfg")     {config(rxbuff); hit=true;}
      
      rxbuff.toUpperCase();
    
      if (rxbuff == "STATUS")  {esp_status(0); hit=true;}
      else if (rxbuff == "RESET")   {ESP.reset(); hit=true;}   // Starta om ESP
      else if (rxbuff == "VERBOSE") {Serial.println("Verbose ON"); cfg_VerboseTrace = 1; hit=true; }  
      else if (rxbuff == "STAT0")  {statCharges=0;statRuns=0;write_eeprom(); hit=true; }  
      else if (rxbuff == "M")       {if (setting[MQTT_ENAB].toInt() && state_HPComm)  if (!client.connected()) reconnect(); mqtt_pub_all();}    // MQTT Reconnect 
    
      else if (rxbuff.substring(0,1) == "L")  {H1_SendCommand(rxbuff); hit=true; }  // LYFCO CMD
      
    
      rxbuff="";
    
    
    } 
  
}
