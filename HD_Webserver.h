ESP8266WebServer *server;
void pageLogin();
void handleNotFound();
void Status2Msg();

String buff;
String s="";
String color="#009900";

const String header = "\
     <!DOCTYPE html>\
     <html><head><title>LYFCO</title>\
     <meta name='viewport' content='width=device-width, initial-scale=1.0'></head>\
     <style>\
       table{font-size: 100%; color: black; bgcolor=#009900; border-collapse:separate; border:solid black 1px; border-radius:6px; }\
       body, input{font-size: 110%;}\
       a{font-size: 110%; }\
       textarea{font-size: 110%; }\
       \
     @media only screen and (max-width:500px) {\  
       table{font-size: 100%; color: black; bgcolor=#009900;  border-collapse:separate; border:solid black 1px; border-radius:6px; }\
       body, input{font-size: 80%;}\
       a{font-size: 100%; }\
       textarea{font-size: 100%; }\
       }\
     </style>";
     
const String menu = "\
     <body bgcolor=#eeeeeeee>\
     <table border = 0 cellspacing=4 cellpadding=1 width=100% style='border:solid black 0px'>\
       <tr bgcolor='#909090'>\
       <td align='center'><a href=/ style='text-decoration: none'><font color='white'>&nbsp;Home&nbsp;</a></td>\
       <td align='center'><a href=/config style='text-decoration: none'><font color='white'>&nbsp;Config&nbsp;</a></td>\
       <td align='center'><a href=/log style='text-decoration: none'><font color='white'>&nbsp;Log&nbsp;</a></td>\
       <td align='center'><a href=/support style='text-decoration: none'><font color='white'>&nbsp;Support&nbsp;</a></td>\
       <td align='center'><a href=/logout style='text-decoration: none'><font color='white'>&nbsp;Logout&nbsp;</a></td>\
     </tr>\
     </table>";


const String footer = "<br><br>(C) 2020 Arandis AB \
                </body></html>";
  
//---------------------------------------------------------------------------------------------------------------
byte websession()
{
   
  if (web_session_sec>0){ // Pågående session
      //if (web_session_ip != server->client().remoteIP().toString())  // Om från annan IP
      if (1!=1)
           { server->send(200, "text/html", "Already in use by other client"); return 2; }
            else
           { web_session_sec = 600;return 1;} // Keep alive  // Sessionstid 10 min innan stängs i sekunder
                             
       }
  if (setting[WEB_PASS]=="") {web_session_sec = 600;return 1;} // Blankt lösen
  return 0;// Om ej inloggad
}
//---------------------------------------------------------------------------------------------------------------


void pageSupport() {
    if (!websession()) {pageLogin();return;}
    byte i;
    bool f;

 // --- Check actions   
    if(server->args()>0) 
           {
            if(server->arg("reboot") == "on") {server->send(200, "text/html", "<h2>Rebooting!<br><br><a href='/'>Return</a>");delay(1000);ESP.reset();}
            if(server->arg("stat0") == "on") {statCharges=0;statRuns=0;statRunMinutes=0; write_eeprom(); server->send(200, "text/html", "<h2>Counter reset!<br><br><a href='/'>Return</a>");}
           }  
    

    buff = "\
    <form action='/update' method='post'>\
    <br>\
      <table width=100% bgcolor=white cellspacing=7 cellpadding=3>\
        <tr><td><input type='submit' value='Update firmware'></td></tr>\
      </table></td></tr>\
    </form>\
    <br>\
    <form action='/support' method='post'>\
      <table width=100% bgcolor=white cellspacing=7 cellpadding=3>\
          <input type='hidden' name='reboot' value='on'>\
          <tr><td align=left><input type='submit' value='Reboot'></td></tr>\
      </table>\         
    </form>\    
    <br>\
    <form action='/support' method='post'>\
      <table width=100% bgcolor=white cellspacing=7 cellpadding=3>\
          <input type='hidden' name='stat0' value='on'>\
          <tr><td align=left><input type='submit' value='Reset counters'></td></tr>\
      </table>\         
    </form>\    
    <br><br>\
    <b>Arandis AB</b><br>\
    Norra Pitholmsvagen 14<br>\
    941 46 Pitea<br>\
    <a href='mailto:support@arandis.se'>support@arandis.se</a><br>\
    ";
  
    
   server->send(200, "text/html", header+menu+buff+footer);
}
//---------------------------------------------------------------------------------------------------------------


void pageUpdate() {
    if (!websession()) {pageLogin();return;}
    byte i;
    bool f;
    //<meta http-equiv='refresh' content='10' ></head>
    buff = "\
    <html><head><title>Updating</title></head>\
    <h3>Updating firmware!</h3>\
    <h1>Do NOT interfere</h1>\
    <table width=100% border=0 cellacing=0 cellpadding=0 style='border:solid black 0px'><tr><td>\
    Firmware update has started. <b>Do not turn off or reset the device.</b><br>\
    Depending on the internet download speed it will take between 30secs and several minutes.<br><br>\
    The device will reboot after update is finished.<br><br>";
    buff += ESP_Update_Result;
    
    buff+="</td></tr></table><br><br><br>";
    server->send(200, "text/html", buff+footer);
    
    if (ESP_Update_Result.length()<2) update_esp_firmware();
   
   
   
}


//---------------------------------------------------------------------------------------------------------------

void pageLog() { // Visa loggen
   
   if (!websession()) {pageLogin();return;}
   int i=0, x;
   buff = "<h3>System log</h3>";

   x=debug_log.indexOf("\n",i);
   while (x>0)
       {
        buff += "&nbsp;" + debug_log.substring(i,x) + "<br>";
        i=x+1;
        x=debug_log.indexOf("\n",i);
       }
    
   server->send(200, "text/html", header+menu+buff+footer);
   
}

//---------------------------------------------------------------------------------------------------------------

void pageConfig() {  // Inställningar ESP
    if (!websession()) {pageLogin();return;}
    byte i;
    String SaveResult;
    String inputType;
            
    buff = "\
    <h3>System configuration</h3>\
    <form action='/config' method='post' autocomplete='off'>\
    <table width=100% border=0 cellspacing=0 cellpadding=2><tr bgcolor=#133670><td>\
    <table border = 0 width=100% bgcolor=white cellpadding=1>\
         <tr><td width=60%><b>&nbsp;Settings</td><td><b>Value</b></td></tr>";
        i=0;
        while (setname[i] != "eof"){
          //  buff+= "<tr><td>&nbsp;" + setname[i] + "</td><td>" + setting[i] + "</td></tr>";
            if(server->args()>0) 
                {if (server->arg(setname[i]) != setting[i]) 
                      if (config("CFG " + setname[i] + "=" + server->arg(setname[i])))  SaveResult+="Setting saved: " + setname[i] + "<br>";    else    SaveResult+="Error saving setting: " + setname[i] + "<br>";
                }      

            if (setname[i]=="WEB_PASS") inputType = "password"; else inputType = "text";
            buff += "<tr><td>&nbsp;" + setname[i] + "</td><td><input type='" + inputType + "' autocomplete='new-password' size='13' maxlength='25' name='" + setname[i] + "' value='" + setting[i] + "'></td></tr>";
            //<input name="password" id="password" type="password" autocomplete="false" readonly onfocus="this.removeAttribute('readonly');" />
            i++;
        }  
        
    buff+="</table></td><tr></table>\
    <br><br>&nbsp;<input type='submit' value='Save changes'>\
    </form>";
    buff += "<br>&nbsp;" + SaveResult;

       
    server->send(200, "text/html", header+menu+buff+footer);
}   



//---------------------------------------------------------------------------------------------------------------
void pageRoot() {
  
  if (!websession()) {pageLogin();return;}
  //rssi_update(); 
  byte i;
  
    buff = "<br>\
     <table width=100% border=0 cellspacing=0 cellpadding=2>\
       <tr><td>\
         <table  width=100% border=0 cellspacing=0 cellpadding=1>\
           <tr>\
             <td width=40><font color='white'><b>&nbsp;Status</td>\
             <td width=40><font color='white'></td></tr>\
             <tr bgcolor=#FFFFFF><td>&nbsp;Lyfco Version</td><td>" + String(cfg_version_major) + "." + String(cfg_version_minor) + "</td></tr>\
             <tr bgcolor=#FFFFFF><td>&nbsp;IP Address</td><td>" + String(g_wifi_ip) + "</td></tr>\
             <tr bgcolor=#FFFFFF><td>&nbsp;Mac Address</td><td>" +  String(g_wifi_mac) + "</td></tr>\
             <tr bgcolor=#FFFFFF><td>&nbsp;Signal strength</td><td>" + String(g_wifi_rssi) + "</td></tr>\
             <tr bgcolor=#FFFFFF><td>&nbsp;System time</td><td>" + tid(0) + "</td></tr>\
             <tr bgcolor=#FFFFFF><td>&nbsp;Uptime min</td> <td>" + uptime + "</td></tr>\
        </table>\
        </td></tr>\
     </table>\
     \
     <h3>Lyfco WiFi modul</h3>\
     <table width=100% cellspacing=0 cellpadding=2 border = 0>\
        <tr bgcolor=#133670><td>\
          <table width=100% cellspacing=0 cellpadding=1 border=0>\
             <tr bgcolor=#133670>\
                <td width=25%><font color='white'><b>&nbsp;</td>\
                <td width=25%><font color='white'><b>&nbsp;</td>\
                <td widtd=25%><font color='white'><b>&nbsp;</td>\
            </tr>";

  byte x;
  
   //buff += "<tr bgcolor=#888888><td>" + Device[i].Name + " (DeviceID:" + String(i) + ", MQTT-Ident:" +  Device[i].MQTT_Ident + ")</td><td>&nbsp</td><td>&nbsp</td></tr>";

    buff += "<tr bgcolor=#FFFFFF><td>Battery volt: " + String(float(l_Volt)/100)  + "</td><td>Status flags: "+l_Status+"</td><td> Batt% " + String(l_BattProcent) +  "</td></tr>";
    buff += "<tr bgcolor=#FFFFFF><td>Chargetime h: " + String(l_ChargeTime)  + "</td><td>Runtime h: "+String(l_RunTime)+"</td><td>Status: " + l_StatusTxt + "</td></tr>";
    
    buff += "<tr bgcolor=#FFFFFF><td>&nbsp;</td><td>&nbsp</td><td>&nbsp</td></tr>";  // Mellanrum
       
   

   buff += "</table>\
          </td></tr>\
      </table>";
  
  server->send(200, "text/html", header+menu+buff+footer);
 rssi_update(); 
}

//---------------------------------------------------------------------------------------------------------------
void pageLogin() {
    byte i;
    static byte login_attempts;

    if (websession()) return;  // Om aktiv inloggad session, avbryt
    if (web_login_block>0) {server->send(200, "text/html", "Blocked one min");login_attempts = 0;return;}  // Avbryt om felaktiga loginförsök
    
    if (server->hasArg("pass")){ 
       if (server->arg("pass") == setting[WEB_PASS])  // Rätt Pass
           { web_session_ip = server->client().remoteIP().toString();
             web_session_sec = 120;
             login_attempts = 0;
             web_login_block = 0;
             pageRoot();
             return;
           } 
           else
           { // Fel Pass
            if (login_attempts++>=3)  {web_login_block = 60;}
           }
      }

    if (web_login_block>0) {server->send(200, "text/html", "Blocked one min");return;}  
        
    if (!web_session_sec)
      {
        buff = "\
        <body bgcolor=#eeeeeeee>\
        <form action='/login' method='post'>\
        <table width=300px align='center' border=3 cellspacing=0 cellpadding=2><tr bgcolor=#FF0000><td>\
          <table border = 0 cellspacing=10 align='center' cellpadding=1 width=100% bgcolor=white>\
             <tr><td align='center'><h3>Please Login</h3><input type='password' style='font-size: 14pt' autofocus='autofocus' size='12' maxlength='15' name='pass' value=''></td></tr>";
            if (login_attempts>0) buff += "<tr><td align='center'>Login attempt " + String(login_attempts) + " of 3 </td></tr>";
            buff+="<tr><td align='center'><input type='submit' value='    Login    '>\
            <tr><td></td></tr>\
            </td></tr></table></td></tr></table>\
        </form>";
     
       server->send(200, "text/html", header+buff+"</body></html>");
       }
       else
          server->send(200, "text/html", "non");
}

void pageLogout() {
    
   if (websession()==2 || setting[WEB_PASS]=="") return; // Om anrop från annan klient
   web_session_sec = 0;
   web_login_block = 0;
   pageLogin();
  
}
//----------------------------------------------------------------------------------------------
void web_session_loop()  // Anropas varje sek från Timer main
{
  if (web_session_sec>0) web_session_sec--;
  if (web_login_block>0) web_login_block--;

}

//----------------------------------------------------------------------------------------------

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server->uri();
  message += "\nMethod: ";
  message += (server->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server->args();
  message += "\n";
  for (uint8_t i = 0; i < server->args(); i++) {
    message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
  }
  server->send(404, "text/plain", message);
}
//---------------------------------------------------------------------------------
void AllData() { // Read all registers in JSON format
  String buff;
  byte i;
  //{"0001":238,"0002":459,"0003":167,"0004":426,"0005":26,"0006":43,"0007":8,"0008":133,"0009":356,"000A":388,"000B":310,"3104":472,"0107":364,"0111":278,"0203":204,"2204":36,"2205":230,"0207":441,"0208":229,"7209":214,"1A01":1,"1A02":0,"1A03":0,"1A04":1,"1A05":0,"1A06":1,"1A07":1,"1A20":0}
   // API Setting has to be 1 or larger

    buff = "{";
    i=0;
    /*
    while (Device[i].Name != "eof" && i<13)
        {    
          if (i!=0) buff += ",";
          buff += String(char(34)); 
          buff += String(Device[i].MQTT_Ident) + String(char(34));
          buff += ":"; buff += String(char(34));
          buff += String(Device[i].Last_Payload);
          buff += String(char(34));
          i++;  
        }
     buff += "}";

     */

     buff += String(char(34)); 
     buff += "distance" + String(char(34));
     buff += ":"; buff += String(char(34));
     buff += String(distance);
     buff += String(char(34));
     
     buff += ",";

     buff += String(char(34)); 
     buff += "status" + String(char(34));
     buff += ":"; buff += String(char(34));
     buff += String(lyfcoStatus);
     buff += String(char(34));

     buff += ",";

     buff += String(char(34)); 
     buff += "charges" + String(char(34));
     buff += ":"; buff += String(char(34));
     buff += String(statCharges);
     buff += String(char(34));

     buff += ",";

     buff += String(char(34)); 
     buff += "runs" + String(char(34));
     buff += ":"; buff += String(char(34));
     buff += String(statRuns);
     buff += String(char(34));

     buff += ",";

     buff += String(char(34)); 
     buff += "runtime" + String(char(34));
     buff += ":"; buff += String(char(34));
     buff += String(statRunMinutes);
     buff += String(char(34));
     
     buff += "}";
     
   server->send(200, "text/plain", buff);
   /*
   buff = "{";
   for (i=0;i<indexNo;i++)
   {
        if (i!=0) buff += ",";
        buff += String(char(34));  
        if (homey) buff += "X";
        buff += String(H1_ID[i]) + String(char(34));
        buff += ":";
        buff += String(H1_ValueNow[i]);
   }
   buff += "}";
   //Serial.print ("A");
   server->send(200, "text/plain", buff);
   */
}

void ApiStatus() { // Read status and cfg in JSON format
  String buff;
  
  String snutt = String(char(34));
  
  buff = "{";
  buff += snutt + "status" + snutt + ":{";
  buff += snutt + "mac" + snutt + ":" + snutt + String(g_wifi_mac) + snutt + ",";
  buff += snutt + "rssi" + snutt + ":" + snutt + String(g_wifi_rssi) + snutt ;
                 
  buff += "}";
             
     
  buff += "}";
  trace("API Status call" , "SQ SER LF VB");
  server->send(200, "text/plain", buff);
 
}

//---------------------------------------------------------------------------------------

void webserver_loop(void) {
  server->handleClient();
}

void setup_webserver(void) {
  /*

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
*/

  int p = 80;
  p = setting[WEB_PORT].toInt();
  server = new ESP8266WebServer(p);

  server->on("/", pageRoot);
  server->on("/login", pageLogin);
  server->on("/logout", pageLogout);
  server->on("/config", pageConfig);
  server->on("/log", pageLog);
  server->on("/support", pageSupport);
  server->on("/update", pageUpdate);
  server->on("/api/alldata", AllData);
  server->on("/status", ApiStatus); 
  server->onNotFound(handleNotFound);
  server->begin();
  trace("HTTP server started on port " + setting[WEB_PORT], "SER LOG LF");
  state_HttpServer = 1;

}
