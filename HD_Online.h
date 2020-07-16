String urlencode(String str);
void handle_response (String payload) ;

void esp_status(byte val);

//====================================================================================================

String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}

//====================================================================================================
String update_esp_firmware()
{
  trace("Updating firmware, dont turn off!  ...","SER LF LOG");

                                
   HTTPUpdateResult ret = ESPhttpUpdate.update("194.4.4.15", 80, "/LyfcoWiFi.ino.d1_mini.bin");
        
        switch(ret) {
            case HTTP_UPDATE_FAILED:
                Serial.println("[update] Update failed." + ESPhttpUpdate.getLastErrorString());
                ESP_Update_Result="[update] Update failed." + ESPhttpUpdate.getLastErrorString();
                break;
            case HTTP_UPDATE_NO_UPDATES:
                Serial.println("[update] Update no Update.");
                break;
            case HTTP_UPDATE_OK:
                Serial.println("[update] Update ok."); // may not called we reboot the ESP
                break;
                  }
                    
}                                  
