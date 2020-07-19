
const String Host="194.4.4.15";  //Update webserver


void mqtt_callback(char* topic, byte* payload, unsigned int length);
void timerCallback(void *pArg);
void reconnect();   // MQTT
void mqtt_pub_all();
void mqttpub (String topic, String msg);
void SerialDebugRX();   // Tar emot data från inbyggd serieport
void rssi_update();

const char max_stored_minutes = 4;      // Tabort: Antal minuter statdata sparad

// LYFCO
long distance=11; // Standby
long dist_sample[10];
long dist_now=11;
String lyfcoStatus = "Standby";
unsigned long lyfcoRunTime;
unsigned int statCharges=0;
unsigned int statRuns=0;
unsigned int statRunMinutes=0;

bool new_H1_data=false;                   // Flagga ny inkommen data
bool new_H2_data=false;                   // Flagga ny inkommen data
const char rx_buffsizeH1 = 110;           // Serial buffer rcv
const char rx_buffsizeH2 = 40;            // Serial buffer rcv
String receivedCharsH1;                   // Mottaget H1 Serial
String receivedCharsH2;                   // Mottaget H1 Serial
String incomingH1;                        // Mottagsbuffer2 H1 for behandling
String incomingH2;                        // Mottagsbuffer2 H1 for behandling

int PrevVolt;
int l_Volt;
int l_BattProcent;
String l_Status="";
int l_ChargeTime;
int l_RunTime;
int l_Alarm=0;
String l_StatusTxt="";
String l_Mode="Standby";
String Mode[2];



//char receivedChars[rx_buffsize];
String ESP_Update_Result="";
String debug_log;
//byte debug_log_ptr=0;
char Led;
unsigned int uptime=0;                    // Minuter upptid sedan boot
char mincount=0;
byte secondsCount=0;
bool every1seconds=0;
char temp[10];
char stored_minute_ptr = 0; // Index på lagrad minut

String license = "";

char oled_rx[20] = "";
char g_wifi_rssi[20] = "";
char g_wifi_ip[20] = "";
char g_wifi_mac[20] = "";
String g_wifi_rssi_num ="0";


byte   indexConvertPtr = 0;
int    indexNo=0;


bool onemin=0;
byte FailedMQTTConnectCounter = 1;   // Retry MQTT
byte FailCounter = 1;   // Retry 5 times, then reboot ESP
unsigned int blinkLedCount=0;

//--------STATUS--------
byte state_mqtt_init = 0;
bool state_Wifi=0;
bool state_HttpServer=0;
bool state_H1Comm=0;
bool state_HPComm=0;
bool state_XL_ListRead = false;
bool state_OnlineComm=0;
bool state_Indexes_Received=0;
bool state_Startuplog_Sent=0;   // Vid uppstart skickas en log efter 1 min, detta görs bara en gång
bool state_H1_GW = 0;           // H1 OM aktiv, Serieldubg Gatewayar direkt mot H1 interfacet
int    web_session_sec = 0;  // Session, räknar ner...
String web_session_ip = "";   //IP på aktiv session;
byte   web_login_block = 0;
