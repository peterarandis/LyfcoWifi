# Lyfco/Exgain Robotic mower Wifi Module MQTT/restAPI
 
What is this?

A wifi remote control for Lyfco / Exgain robotic mowers models 1800, 1750 and 1600.
Features: Web interface and MQTT client for status readout and remote control.


Materials needed
- ESP8266 development board, like Wemos D1 or similar.
- Thin cables
- Soldering equipment and skills.

Preparing the hardware
- Setup Arduino for your ESP board and compile the code
- Upload the code to the ESP using USB cable
- Run Serial debug window and type CFG and Enter to list current config.
- Change config like Wifi, password, MQTT etc by typing like "CFG wifi_ssid=test"
- As soon it has connected to WiFi you can connect to its IP via web browser for more convenient settings management and monitoring.
- Solder 6 cables as shown in diagram, OBS If not using Wemos, Pins may be different.


Installation
- Turn off the Lyfco completely and open the top cover.
- Locate the built Wifi module (White QR code sticker)
- Connect the 6 cables as shown in diagrams and pictures.
- Mount/place the Wifi module in a good spot.


MQTT Read -----------

- battery        (25.55V)
- battprocent    (66%)
- status         ("00000000000000" Status flags)
- statustxt      ("Lost power on base station")
- chargetime     (140 hours)
- runtime        (110 hours)
- rssi           ("Good -55dBm")
- rssinum        (-55)
- mode			 ("Standby", "Charging" or "Running")
- alarm		     (0 or 1)
  
MQTT Set  (lyfco/set) ------------

- PAYLOAD
- lstop		Stop
- lrun		Run
- lhome		Go home
- lremote	Start remote mode
- lforw		Move forward
- lback		Move backwards
- lleft     Move left
- lright	Move right
- lmow		Start cut motor


