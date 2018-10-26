### Arduino ESP32 OLED LoRa Example

Sample code to send and receive LoRaWAN messages.

## Usage

When the device turns on in does a OTAA join. 
If you press the PRG button it sends a msg.
If you long press the PRG button it switches between unconfiremd and confirmed msgs.

In case of an ACK or a dwonstream message the value will be displayed on the OLED with the RSSI and SNR values.

## Notes

Remember to change the pin mapping (lmic_pinmap) for your ESP32 LoRa device if not an Heltec. 

It is tested using an Heltec ESP32 OLED Lora V2 using the  lmic library "heltec version" : https://github.com/HelTecAutomation/heltec_lmic 