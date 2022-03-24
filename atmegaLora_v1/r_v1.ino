#include "LoRa_E32.h"
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11);
 
#define loraAdet 2
 
LoRa_E32 e32ttl(&mySerial);

//alınan veri
typedef struct {
char type[15];
char temp[15];
}Signal;

Signal data;

void setup() {
  Serial.begin(9600);
  e32ttl.begin();
  delay(500);
  
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
 
  delay(3000);
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

    }
    if(!(kontrol)){
     Serial.print(i+1);
     Serial.println(". Loradan  veri Gelmedi??????");
      Serial.println("/*******/");
    }
 }
  
}
