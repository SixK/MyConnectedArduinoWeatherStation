# MyConnectedArduinoWeatherStation

A Connected Weather Station based on Arduino, Raspberry Pi and LAMP Web Server.

Arduino Features :
- Read Temperature, Humidity and Pressure
- TFT Screen to read Time, Temperature, Humidity and Pressure Values
- TFT Graphical view of Temperature, Humidity and Pressure for last hours
- Clock synchronisation
- Data saved on SD card

Raspberry Features :
- Collect data from Arduino and send them to internet
- Synchronize Arduino Clock on start

Web Site Features:
- Save data in database
- Visualize data curves

Hardware :
- Arduino Mega 2560 (Seeduino Mega) - Needed to handle SD Card and TFT (memory size limitations)
- ADA358 1"8 color TFT with SD Card support 
- DHT22 Temperature / Humidity sensor
- SEN12671P - Real Time Clock (RTC)
- BMP180 - ADA1603 - Pressure / Temperature sensor
- RF433Mhz kit (to comunicate with external commercial sensors)


