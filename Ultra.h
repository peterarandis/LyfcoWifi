
// utlrasonic pinout
#define ULTRASONIC_TRIG_PIN     5   // pin TRIG to D1
#define ULTRASONIC_ECHO_PIN     4   // pin ECHO to D2

const unsigned int writeInterval = 1500; // write interval (in ms)


void ultrasetup()
{
  // ultraonic setup 
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
} 



void ultra_ping() {


        long duration;
        digitalWrite(ULTRASONIC_TRIG_PIN, LOW);  
        delayMicroseconds(2); 
        
        digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
        delayMicroseconds(10); 
        
        digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
        duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
        dist_now = (duration/2) / 29.1;
        //Serial.print("Ultrasonic Distance: " + String(dist_now) + "cm");
        
        if (dist_now>45) dist_now = 45; // Kan variera massor när klipparen är ute

        // Förskjut listan, Måste vara samma värde i fyra avläsningsar efter varandra (ca 7 sekunder) innan det sätts som "distance".
        dist_sample[3] = dist_sample[2];
        dist_sample[2] = dist_sample[1];
        dist_sample[1] = dist_sample[0];
        dist_sample[0] = dist_now; // Spara senaste
        
        // Om 4 senaste avläsningarna är lika, spara
        if ((dist_now == dist_sample[1]) &&  (dist_now == dist_sample[2]) && (dist_now == dist_sample[3])) 
             {
             distance = dist_now;
             //Serial.print("Confirmed Ultrasonic Distance: " + String(dist_now) + "cm");
             }
        if (dist_now!=dist_sample[9])
                   {dist_sample[9]=dist_now;
                    trace("DistNow=" + String(dist_now) + " Dist=" + String(distance) +" "+ lyfcoStatus,"LOG SER LF");
                   }
        
       //-------
       if ((distance <8) && (lyfcoStatus != "Charging"))                          {lyfcoStatus = "Charging";statCharges++;write_eeprom();}  // 6-8 cm normalt
       if (distance >7 && distance <22)                                            lyfcoStatus = "Standby";                                 // 11-13 cm normalt
       if (distance >=22 && lyfcoStatus != "Running" && lyfcoStatus != "Stuck" )  {lyfcoStatus = "Running"; 
                                                                                   statRuns++; write_eeprom();lyfcoRunTime = millis();}     // 25+ cm, sätt starttid
       if (lyfcoStatus == "Running" && (millis() > (lyfcoRunTime+10800000)))       lyfcoStatus = "Stuck";                                   // Borta längre än 3 timmar

   
       
       
 

}
