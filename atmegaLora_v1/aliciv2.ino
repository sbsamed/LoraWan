#include "LoRa_E32.h"
#include <SoftwareSerial.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

SoftwareSerial mySerial(10, 11);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

 
#define loraAdet 2
 
LoRa_E32 e32ttl(&mySerial);

//alınan veri
typedef struct {
char type[15];
char temp[15];
}Signal;

Signal data;

void setup() {
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
   
  }
  Serial.begin(9600);
  e32ttl.begin();
  delay(500);
   display.clearDisplay();
     display.setTextSize(1);
  display.setTextColor(WHITE);
  }


void loop() {
   
  //gönderilen veri
  struct Signal2 {
    char type[8]="gateway";
    char temp[8]="ready";
} data2;


int loraAdresler[loraAdet]={45,46};

byte kontrol=0;
 for(byte i=0;i<loraAdet;i++){
  ResponseStatus rs = e32ttl.sendFixedMessage(0, loraAdresler[i], 7, &data2, sizeof(Signal2));
  Serial.print(i+1);
  Serial.print(". Loraya Veri Gönderildi - ");
  Serial.println(rs.getResponseDescription());
 
  delay(2000);
  kontrol=0;
  while (e32ttl.available()>1){
    ResponseStructContainer rsc = e32ttl.receiveMessage(sizeof(Signal));
    data=*(Signal*)rsc.data;
    Serial.print(i+1);
     Serial.print(". Loradan Gelen veri : ");
     Serial.print(data.type);
     Serial.print(" , ");
     Serial.println(data.temp);
     Serial.println("/*******/");
     rsc.close();
     kontrol=1;
  
  ////display
    display.clearDisplay();
     display.setCursor(0,0);
    display.println("GateWay");
    display.display();
    
    
    display.setCursor(0,20);
    display.println(data.type);
    display.display();

    ///
    display.setCursor(0,40);
    display.println(data.temp);
    display.display();

      
  
    }
    if(!(kontrol)){
     Serial.print(i+1);
     Serial.println(". Loradan  veri Gelmedi??????");
      Serial.println("/*******/");
    }
 }
  
}
