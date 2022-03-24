#include "LoRa_E32.h"
#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // Arduino RX <-- e32 TX, Arduino TX --> e32 RX
LoRa_E32 e32ttl(&mySerial);
 
//alınan veri
struct Signal {
  char type[8];
  char temp[8];
} data;
 
void setup() {
  Serial.begin(9600);
  e32ttl.begin();
  delay(500);
}
 
void loop() {
  while (e32ttl.available()  > 1) {
 
    // Gelen mesaj okunuyor
    ResponseStructContainer rsc = e32ttl.receiveMessage(sizeof(Signal));
    struct Signal data = *(Signal*) rsc.data;
    Serial.print("Gelen Mesaj: ");
    Serial.print(data.type);
     Serial.print(",");
     Serial.println(data.temp);
      rsc.close();
      
      if(strcmp(data.temp,"ready")==0){
  
    //Gönderilecek paket veri hazırlanıyor
     struct Signal2 {
    char type2[15]="Lora_1";
    char temp2[15]="AET";
      } data2;
    ResponseStatus rs = e32ttl.sendFixedMessage(0, 44, 7, &data2, sizeof(Signal2));
   Serial.print("Veri Gönderildi-");
  Serial.println(rs.getResponseDescription());
  Serial.println("/***********************/");
 
        
    }
  }
}
