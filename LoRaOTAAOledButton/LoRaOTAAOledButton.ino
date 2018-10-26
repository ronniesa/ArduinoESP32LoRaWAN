#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <U8x8lib.h>
#include <AceButton.h>

using namespace ace_button;

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

const int BUTTON_A = 0;
uint8_t mydata[20];

bool confirmed = false;

unsigned long elapsedTime;
int joiningLoop=0;
bool joining = false;
int i=0;

AceButton button(BUTTON_A);

void handleEvent(AceButton*, uint8_t, uint8_t);

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
//https://www.cnblogs.com/wuyuegb2312/archive/2013/06/08/3126510.html
static const u1_t PROGMEM APPEUI[8]= { 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8]= { 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
// The key shown here is the semtech default key.
static const u1_t PROGMEM APPKEY[16] = { 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}


static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 20;

// Pin mapping
#if defined(V1)
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,
    .dio = {26, 33, 32},
};
#elif defined(V2)
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,
    .dio = {26, 35, 34},
};
#endif




void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            u8x8.clear();
            u8x8.drawString(0,0,"Joining....");            
            joining=true;
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            u8x8.clear();   
            u8x8.drawString(0,0,"Join succeeded");
            u8x8.drawString(0,2,"Press PRG to");
            u8x8.drawString(0,3,"send a msg");
            u8x8.drawString(0,5,"Long press");
            u8x8.drawString(0,6,"PRG to change");
            u8x8.drawString(0,7,"conf/unconf");
            joining =false;
            // Disable link check validation
            LMIC_setLinkCheckMode(0);
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            u8x8.clear();   
            u8x8.drawString(0,0,"Join failed");
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            u8x8.clear();   
            u8x8.drawString(0,0,"Rejoin failed");
            break;
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));           
            u8x8.drawString(0,1,"Send completed");
            if (LMIC.txrxFlags & TXRX_ACK)
            {
              Serial.println(F("Received ack"));
              u8x8.drawString(0,2,"Received ack");
              if (!LMIC.dataLen)
              {
                u8x8.setCursor(0, 7);
                u8x8.printf("RSSI %d SNR %.1d", LMIC.rssi, LMIC.snr);             
              }
            }
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));

                u8x8.drawString(0,4, "Data recieved:");
                char str[LMIC.dataLen+1];
                str[LMIC.dataLen]='\0'; // Add terminator
                memcpy(&str,&(LMIC.frame+LMIC.dataBeg)[0],LMIC.dataLen);  
                Serial.println(str);
                u8x8.drawString(0,5, str);
                u8x8.setCursor(0, 7);
                u8x8.printf("RSSI %d SNR %.1d", LMIC.rssi, LMIC.snr); 
            }

           
            
            // Schedule next transmission
            //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void do_send(osjob_t* j){
	Serial.print("Send, txCnhl: ");
	Serial.println(LMIC.txChnl);
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        sprintf((char *)mydata,"%d",i);
        u8x8.clear();

        String str = "Sending: " + String(i);
        char charStr[20];

        str.toCharArray(charStr, 20);        
        
        u8x8.drawString(0,0, charStr);

        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata,  strlen((char *)mydata), confirmed );
        Serial.println(F("Packet queued"));
        i++;

        
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("Starting"));

    pinMode(BUTTON_A, INPUT_PULLUP);

    button.setEventHandler(handleEvent);
    
    ButtonConfig* buttonConfig = button.getButtonConfig();

    buttonConfig->setEventHandler(handleEvent);

    button.getButtonConfig()->setFeature(ButtonConfig::kFeatureLongPress);
    
    buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
   
    buttonConfig->setLongPressDelay(1000);

    

    u8x8.begin();
    u8x8.setPowerSave(0);
 
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.drawString(0,0,"Starting");

    

    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Start job (sending automatically starts OTAA too)
    //do_send(&sendjob);
    LMIC_startJoining();
}

void handleEvent(AceButton* /* button */, uint8_t eventType,
    uint8_t /* buttonState */) {
  switch (eventType) {
    case AceButton::kEventReleased:
      do_send(&sendjob);
      break;
     case AceButton::kEventLongPressed:
      confirmed = !confirmed;
      u8x8.clear();
      if(confirmed)
      {  
        u8x8.drawString(0,0,"Using");
        u8x8.drawString(0,1,"confirmed");
      }
      else
      {  u8x8.drawString(0,0,"Using");
         u8x8.drawString(0,1,"unconfirmed");
      }
      break;
  }
}

void loop() {
    os_runloop_once();

    button.check();

//    if (!digitalRead(BUTTON_A))
//    {
//      delay(500);
//       do_send(&sendjob);
//    }

    if(joining &&  millis() > elapsedTime + 500)
    {
      if(joiningLoop ==0)
        u8x8.drawString(0,0,"Joining     "); 
      if(joiningLoop ==1)
        u8x8.drawString(0,0,"Joining .   ");  
      if(joiningLoop ==2)
        u8x8.drawString(0,0,"Joining ..  "); 
      if(joiningLoop ==3)
      {
        u8x8.drawString(0,0,"Joining ... "); 
        joiningLoop=0;
      }
      else
      {
        joiningLoop++;
      }
      elapsedTime = millis();
    }
}
