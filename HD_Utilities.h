#include <EEPROM.h>
// Time / NTP
#define TZ              0               // (utc+) TZ in hours
#define DST_MN          60              // use 60mn for summer time in some countries
#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)
timeval tv;
time_t now;


// --------------------------- Config -----------------------------
byte cfg_version_major = 0;
byte cfg_version_minor = 0;
byte cfg_API_Write = 0;
byte cfg_VerboseTrace = 0;

#define WIFI_SSID   0 
#define WIFI_PASS   1 
#define DHCP_ENAB   2
#define FIXED_IP    3
#define FIXED_SUB   4
#define FIXED_GW    5
#define FIXED_DNS   6
#define SMS_NR      7 
#define WEB_ENABLED 8 
#define WEB_PASS    9
#define WEB_PORT    10
#define MQTT_ENAB   11
#define MQTT_SRVR   12
#define MQTT_PORT   13 
#define MQTT_USER   14
#define MQTT_PASS   15
#define MQTT_SUBS   16
#define ENGAGED     17


const String setname[] = {"WIFI_SSID","WIFI_PASS","DHCP_ENAB","FIXED_IP","FIXED_SUB","FIXED_GW","FIXED_DNS","SMS_NR","WEB_ENABLED","WEB_PASS","WEB_PORT","MQTT_ENAB","MQTT_SRVR","MQTT_PORT","MQTT_USER","MQTT_PASS","MQTT_SUBS","ENGAGED","eof"};
String setting[] =       {"none",       "none",     "1",   "0.0.0.0","255.255.255.0","0.0.0.0",   "1.1.1.1",   "0",          "1",        "1234",    "80",       "0",      "none",     "1883", "homeassistant", "none",   "0",      "0"};   // Defaults

//-----------------------------------------------------------------------------------------------
void trace (String text, String func) ;

void blinkLed(){
    if (Led) {digitalWrite(LED_BUILTIN, LOW);Led=0;} 
      else
             {digitalWrite(LED_BUILTIN, HIGH);Led=1;}
    }

//-----------------------------------------------------------------------------------------------           
void init_eeprom() {
  EEPROM.begin(512);  // Max 4096
}

void write_eeprom(){

unsigned int a;
    trace("Stored statistics: Charges=" + String(statCharges) + ", Runs=" + String(statRuns) ,"SER LOG LF");

     a=statCharges/256;
     EEPROM.write(0,a);
     EEPROM.write(1,statCharges-(a*256));
     
     a=statRuns/256;
     EEPROM.write(10,a);
     EEPROM.write(11,statRuns-(a*256));

     a=statRunMinutes/256;
     EEPROM.write(20,a);
     EEPROM.write(21,statRunMinutes-(a*256));
    
     EEPROM.commit();


 
}  

void read_eeprom(){

    unsigned int L,M;
    
    statCharges = EEPROM.read(0)*256;
    statCharges = statCharges + EEPROM.read(1);

    statRuns = EEPROM.read(10)*256;
    statRuns = statRuns + EEPROM.read(11);

    statRunMinutes = EEPROM.read(20)*256;
    statRunMinutes = statRunMinutes + EEPROM.read(21);


    //trace("Read statistics: Charges=" + String(statCharges) + ", Runs=" + String(statRuns) ,"SER LOG LF");
     
     //      M=stats_BootCount/256;
     //      L=stats_BootCount - (L*256);
    //       EEPROM.write(1,L); EEPROM.write(2,M); // Skriv boot count + 1
    
}   

//-----------------------------------------------------------------------------------------------

void settings_rw(bool read_settings){ //Läs och skriv inställningar till SPIFFS fil
  byte i;
  String s, key, val;  
  byte mid, end; 
  bool Hit=0;  
    
  cfg_version_major = 0;
  cfg_version_minor = 5;
  if (!read_settings)
    {
    // Ladda config från SPIFFS fil.
    // Öppna fil
    File f = SPIFFS.open("/config2.txt", "r");
    if (!f) {trace("Config file failed!","SER LF LOG"); 
             return;
            }
      
    // Parsa json string till variabler
    while (f.available()){   // Så länge ej EOF och H1 svarar med ACK
            s = f.readStringUntil(','); // Läs en rad
            s.trim();
            //Serial.println(s);
            mid = s.indexOf(':');
            end = s.length();
            key = s.substring(1,mid-1);
            val = s.substring(mid+2,end-1);
           // Serial.println (key + " : " + val);
            
            i=0;Hit=0;
            while (setname[i] != "eof" && !Hit)
               {
                if (key == setname[i]) 
                     {setting[i] = val; 
                      //Serial.println("set " + String(i));
                      Hit=1;
                      
                     }
                i++;     
               }

          }   
      }
   else
      {
     // STORE SETTINGS
      File f = SPIFFS.open("/config2.txt", "w");
      if (!f) { trace("Config file write failed!","SER LF LOG"); 
               return;
              }
      i=0;
      while (setname[i] != "eof")  // Skriv till json format
              {f.println("'" + setname[i] + "':'"  + setting[i] + "',");
               i++;
              }
      f.close();
    
      }
}

//-----------------------------------------------------------------------------------------------


void rssi_update()
{
   int i = WiFi.RSSI();
   char s[7] = "Strong";
   if (i< -60) sprintf(s,"Good");
   if (i< -70) sprintf(s,"Okay");
   if (i< -80) sprintf(s,"Weak");
   if (i< -90) sprintf(s,"Bad");
   sprintf(g_wifi_rssi,"%s %ddBm", s, i); // Uppdatera signalstyrka
   g_wifi_rssi_num = String(i);
}

//-----------------------------------------------------------------------------------------------


String tid(char val){

  String t;
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  
   if (val==0 || val==1 || val==3) // {return String(hr) + ":" + String(min % 60) + ":" + String(sec % 60);} // "10:00:00"
   //else if (val==1) // "2018-12-01 10:00:00"
   {
     now = time(nullptr);
     char buffer[80];
     if (val==1) 
         strftime (buffer,80,"%Y-%m-%d %H:%M:%S",localtime(&now));
     else if (val==3) 
         strftime (buffer,80,"%H%M%S",localtime(&now));
     else    
         strftime (buffer,80,"%H:%M:%S",localtime(&now));
         
         
     t = buffer;
     return t;
     //Serial.println (buffer);
   }
   else if (val==2)   // Connect to NTP and report
   { 
    char buffer[10];
    buffer[0]=1; // 1970
    char i=0;
    while (buffer[0] != '2' && i++<5)
       {
        now = time(nullptr);
        delay(100);
        strftime (buffer,10,"%Y",localtime(&now));
       }
     if (buffer[0] != '2') trace("Cannot get NTP updates!","SER LOG LF"); else trace("NTP server connected successfully!","SER LOG LF");
     
   }  

  /*
  %a Abbreviated weekday name 
  %A Full weekday name 
  %b Abbreviated month name 
  %B Full month name 
  %c Date and time representation for your locale 
  %d Day of month as a decimal number (01-31) 
  %H Hour in 24-hour format (00-23) 
  %I Hour in 12-hour format (01-12) 
  %j Day of year as decimal number (001-366) 
  %m Month as decimal number (01-12) 
  %M Minute as decimal number (00-59) 
  %p Current locale's A.M./P.M. indicator for 12-hour clock 
  %S Second as decimal number (00-59) 
  %U Week of year as decimal number,  Sunday as first day of week (00-51) 
  %w Weekday as decimal number (0-6; Sunday is 0) 
  %W Week of year as decimal number, Monday as first day of week (00-51) 
  %x Date representation for current locale 
  %X Time representation for current locale 
  %y Year without century, as decimal number (00-99) 
  %Y Year with century, as decimal number 
  %z %Z Time-zone name or abbreviation, (no characters if time zone is unknown) 
  %% Percent sign 
  You can include text literals (such as spaces and colons) to make a neater display or for padding between adjoining columns. 
  You can suppress the display of leading zeroes  by using the "#" character  (%#d, %#H, %#I, %#j, %#m, %#M, %#S, %#U, %#w, %#W, %#y, %#Y) 
*/

}

//===============================================================================

void trace (String text, String func)  // SER LOG LF
{
  byte i; 
  if (func.indexOf("VB")!=-1) if (!cfg_VerboseTrace) return;
  
  if (debug_log.length() > 2000) {debug_log="";}
 
  if (func.indexOf("LF")!=-1) text += "\n";  
  //if (func.indexOf("SER")!=-1) Serial.print(debug_log.length() + " " + text);
  if (func.indexOf("SER")!=-1) Serial.print(text);
  if (func.indexOf("LOG")!=-1)  {debug_log += tid(0) + " " + text;  }
  if (func.indexOf("LIST")!=-1) {
       Serial.print("TRACE LOG");Serial.println("  ------------");
       Serial.print(debug_log);

  }
 
}

//===============================================================================
// behöver inte returnera nått, modifierar originalvariabeln
char *tolow (char *s)
{
   char *p;
   for (p = s; *p != '\0'; ++p)  { *p = tolower(*p);  }
   return s;  
}

//===============================================================================

bool config (String cmd)
{
  String subcmd = "";
  String val = "";
  String s;
  byte i, idx;
  byte Hit=0;

  if (cmd.length() > 35) return 0;  // too long
  
      //Serial.println (cmd);
      idx = cmd.indexOf('=');
      subcmd = cmd.substring(4,idx);  // Läs ut kommando
      subcmd.toUpperCase();
      val =  cmd.substring(idx+1,cmd.indexOf('\n'));  // Läs ut värde att sätta
      if (val.length() > 25) return 0;  // too long value
      
      i=0;Hit=0;
         while (setname[i] != "eof" && !Hit)
               {
                if (subcmd == setname[i]) 
                     {setting[i] = val; 
                      //Serial.println("set " + String(i));
                      Hit=1;
                     }
                i++;     
               }

           
            if (idx==-1 || Hit==0) { // Endast CMD utan värde = lista config
                   
                   String output = "SER LF";
                   if (subcmd == "statuslog") output = "SER LOG LF";
                   i=0;
                   while (setname[i] != "eof")
                        {
                          trace(setname[i] + "=" + setting[i], output);
                          i++;
                        }  
            } 
            if(Hit) 
                 {trace(subcmd + " is now set to " + val, "SER LF");
                 // Serial.print(subcmd);Serial.print(" is now set to ");Serial.println(val);
                  settings_rw(true); //Skriv ner settings
                  return 1;
                  }
                  else
                  trace("Invalid setting format: " + cmd, "SER LF LOG");
                  return 0; // Settting not found
}

//------------------------------------------------------------------------------------------------------------------
void esp_status(byte val)
{
 char x[20];
 String output = "SER LF";
        
        if (val == 0 || val == 3)
         {
            if (val==3) output = "SER LF LOG";
            trace("-- Status --",output);
            trace("Time NTP: " + tid(1),output);                                      
            trace("Uptime: " + tid(0),output);                                      
            trace("WIFI MAC address: " + String(g_wifi_mac),output);                  
            trace("WIFI IP address: " + String(g_wifi_ip),output);                    
            trace("WIFI Signal: " + String(g_wifi_rssi),output);                      
            trace("WIFI connection: " + String(state_Wifi),output);                   
            trace("Internal Webserver status: " + String(state_HttpServer),output);   
         }
         else if (val == 2)
         {
            tid(1).toCharArray(x,20);  mqttpub ("STATUS/TIME", x);
            tid(0).toCharArray(x,20);  mqttpub ("STATUS/UPTIME", x);
            mqttpub ("STATUS/MAC", g_wifi_mac);
            mqttpub ("STATUS/IP", g_wifi_ip);
            mqttpub ("STATUS/RSSI", g_wifi_rssi);       
     
         }



}

 
