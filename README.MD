### Arduino ESP32 OLED LoRa Example

Sample code to send and receive LoRaWAN messages.

## Usage

When the device turns on it perfroms an OTAA join. 
Press the PRG button to send a msg. It sends 1, 2 , 3, etc..
Long press the PRG button to toggle between unconfiremd and confirmed messages.

In case of an ACK or a dwonstream message the value will be displayed on the OLED with the RSSI and SNR values.

## Notes

Remember to change the AppEUI, AppKey and DevEUI. Also change the pin mapping (lmic_pinmap) for your ESP32 LoRa device if not an Heltec. 

It is tested using the Azure IoT Edge LoRaWAN module: https://github.com/Azure/iotedge-lorawan-starterkit and an Heltec ESP32 OLED Lora V2 using the  lmic library "heltec version" : https://github.com/HelTecAutomation/heltec_lmic 