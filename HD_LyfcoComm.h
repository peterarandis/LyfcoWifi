void Status2Msg();



void H2_Recv() {     // Data mottages from Lyfco APP

  
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;
    
  while (H2.available() > 0 && new_H2_data == false) {
    rc = H2.read();
    if (rc != endMarker)  {
      receivedCharsH2 += rc;
      ndx++;
      if (ndx >= rx_buffsizeH2) {  receivedCharsH2="";ndx=0;  }
      if ((rc == '\n' || rc == '\r') && ndx < 2) {ndx=0;receivedCharsH2="";}   
    }
    else {
      receivedCharsH2 += '\0';
      new_H2_data = true;
      incomingH2=receivedCharsH2;
      //Serial.print ("H2> " + incomingH2);
      ndx = 0;
      receivedCharsH2="";
    }
    
  yield();
  }
 
}


//========================================================================================================

void H1_Recv() {     // Data mottages from H1 Interface

  
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;
    
  while (H1.available() > 0 && new_H1_data == false) {
    rc = H1.read();
    if (rc != endMarker)  {
      receivedCharsH1 += rc;
      ndx++;
      if (ndx >= rx_buffsizeH1) {  receivedCharsH1="";ndx=0;  }
      if ((rc == '\n' || rc == '\r') && ndx < 2) {ndx=0;receivedCharsH1="";}   
    }
    else {
      receivedCharsH1 += '\0';
      new_H1_data = true;
      incomingH1=receivedCharsH1;
      //Serial.print (incomingH1);
      ndx = 0;
      receivedCharsH1="";
    }
    
  yield();
  }
 
}


//========================================================================================================

void H2_HandleData() {   // Hantera inkommet H2 data från Lyfco app
  
  if (!new_H2_data) return;
   // Serial.print ("incoming ");  
    //cmd = incoming.substring(0,2);
    //Serial.print (sprintf(".%s.",row); Serial.println (cmd);

   
   //trace("RX " + incomingH1,"SER LOG LF");
   
   trace("App> " + incomingH2.substring(0,incomingH2.length()-2),"SER LOG LF");       
   
   //H1.println (incomingH2);
   H1.println (incomingH2.substring(0,incomingH2.length()-2));
   new_H2_data = false;
}

//========================================================================================================

void H1_HandleData() {   // Hantera inkommet H1 data från MQTT/Web
  
  if (!new_H1_data) return;
   // Serial.print ("incoming ");  
    //cmd = incoming.substring(0,2);
    //Serial.print (sprintf(".%s.",row); Serial.println (cmd);

   
   //trace("RX " + incomingH1,"SER LOG LF");
   
   trace("Response> " + incomingH1.substring(0,incomingH1.length()-2),"SER LOG LF");         
   
   
  byte i;
  int len;

   
    // STATUS
    if (incomingH1.substring(0,5)=="##35W"){
        
        //##35W0001030145259600000000000000F997
        //##35W00 0103 0145 2596 00000000000000 F997
        // cmd    run  chg  volt flags
 
        l_RunTime    = incomingH1.substring(7,11).toInt(); 
        l_ChargeTime = incomingH1.substring(11,15).toInt();
        l_Volt       = incomingH1.substring(15,19).toInt();
        l_Status     = incomingH1.substring(19,33);
        
        Serial.println ("Status: chgtime=" + String(l_ChargeTime) + "h, runtime=" + String(l_RunTime) + "h, Batt=" + String(float(l_Volt)/100) + "V, Status=" + l_Status);
        Status2Msg();

        if (PrevVolt==0) PrevVolt=l_Volt;   // Startup
        else if (l_Volt+5 < PrevVolt)  l_Mode = "Running";  // Om voltage drop > 0,05v under en min = klipper
        else if (l_Volt > PrevVolt )     l_Mode = "Charging";  // Om volt ökar
        else l_Mode = "Standby";  // Voltage drop 0.01v / min
        
        PrevVolt=l_Volt;
        
        
        l_BattProcent = l_Volt - 2400;  // 24.0 är lika med 0%
        
        l_BattProcent = (float(l_BattProcent)/510) * 100; // 24.0v + 5.1V är max volt
        
        //if (l_BattProcent) < 0 then l_BattProcent=0;
        //if (l_BattProcent) > 100 then l_BattProcent=100;
        
        
    }

  new_H1_data = false;

}

//=============================================================

void H1_SendCommand(String cmd) {  
  
  String TX = "UNKNOWN";

  cmd.toLowerCase();
  
  if (cmd == "lstat")       TX="##07WFEFC";   //##35W0001030145259600000000000000F997

  if (cmd == "lsettings")   TX="##07FFF0D";  // Read settings
    /*
    ##18F 40004111110FCEE
    40  ??
    00  Språk 0=Eng, 2=Svenska
    4   Räckvidd (1-4)
    1   ??
    1   Regnsensor 1,0
    1   Vidrörings sensor 1,0
    1   Trycksensor 1,0
    1   Kompass 1,0
    0   Slå larm 1,0
    FCEE   Checksumma
    */


  //if (cmd == "lsettime")  {TX="##22T...."; // EJ KLAR
  //22T 20200713 11 1857 18 FC04
  //    datum   dag klock ?? chk
  
  // Körläge
  if (cmd == "lstop")  TX="##08Y0FEC9";  // Stoppa
  if (cmd == "lrun")   TX="##08Y5FEC4";  // Kör klippning auto
  if (cmd == "lhome")  TX="##08Y7FEC2";  // Hemgång


  // Fjärrkontroll
  if (cmd == "lforw")  TX="##08Y1FEC8";
  if (cmd == "lback")  TX="##08Y2FEC7";
  if (cmd == "lleft")  TX="##08Y3FEC9";
  if (cmd == "lright") TX="##08Y4FEC5";
  if (cmd == "lmow")   TX="##08Y8FEC1"; // Starta klippmotor
  if (cmd == "lremote")TX="##08Y6FEC3"; // Starta fjärrkontroll läge
  
  trace("Cmd> " + cmd + ",    TX=" + TX,"SER LOG LF");
  if (TX!="UNKNOWN") H1.println (TX);
  

  
}


//=============================================================


void Status2Msg()
{

 String s="";

  //##35W00 0103 0145 2596 00000000000000 F997
  //                         Status flags
  if (l_Status.substring(0,1) == "1")   s=s + "E1" + ",";
  if (l_Status.substring(1,2) == "1")   s=s + "E2 No power on charge station" + ",";
  if (l_Status.substring(2,3) == "1")   s=s + "E3" + ",";
  if (l_Status.substring(3,4) == "1")   s=s + "E4" + ",";
  if (l_Status.substring(4,5) == "1")   s=s + "E5" + ",";
  if (l_Status.substring(5,6) == "1")   s=s + "E6" + ",";
  if (l_Status.substring(6,7) == "1")   s=s + "E7 Machine lifed" + ",";
  if (l_Status.substring(7,8) == "1")   s=s + "E8 Machine pressed" + ",";
  if (l_Status.substring(8,9) == "1")   s=s + "E9 Collision sensor problem" + ",";
  if (l_Status.substring(9,10) == "1")  s=s + "E10" + ",";
  if (l_Status.substring(10,11) == "1") s=s + "E11" + ",";
  if (l_Status.substring(11,12) == "1") s=s + "E12 Outside guide wire" + ",";
  if (l_Status.substring(12,13) == "1") s=s + "E13" + ",";
  if (l_Status.substring(13,14) == "1") s=s + "E14" ;

  trace(s,"SER LOG LF");    
  l_StatusTxt = s;
  

}
