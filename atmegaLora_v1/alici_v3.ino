#include "LoRa_E32.h"
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <DS3231.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>

//** SD Card ****
File myFile;
boolean initSucces = false;
boolean makeDirSucces = false;

char* isim = "LoRaLOG";
char* uzanti = "txt";
String dir = String(isim) + "." + String(uzanti);

unsigned long timer = 0;
byte count = 0;
//*******

//** RTC ****
DS3231  rtc(SDA, SCL);
String Internal_RTC[6];
//*******

//*** Metot Tanımlamaları ***
void proccesBlink(void);
boolean SD_Card_Init(void);
boolean Make_Directory(char* dosyaAdi, char* uzanti);
void File_Write(String dosyaAdi, String data);
void File_Clear_Write(String dosyaAdi, String data);
String File_Read(String dosyaAdi);
String File_ReadLine(String dosyaAdi);
void File_Remove(String dosyaAdi);
void Show_Date_Time(void);
void RTC_Time_Update();
//********

SoftwareSerial mySerial(10, 11);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

 
#define loraAdet 2
 
LoRa_E32 e32ttl(&mySerial);

//alınan veri
typedef struct {
char type[15];
char temp[15];
}Signal;

Signal data;

void setup() {
//    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
//    //Serial.println(F("SSD1306 allocation failed"));
//   
//  }
  Serial.begin(115200);
  e32ttl.begin();
  delay(500);
  
  rtc.begin();
  initSucces = SD_Card_Init();
  if(initSucces == true){
    //makeDirSucces = Make_Directory(isim, uzanti);
    //dir = confDirName(RTC_Get_Date()) + ".txt";
    //dir = "aaaaaaaa.txt";//Max 8 uzunlukta isim olmalı.
    //Serial.println("Dosya Adi : " + RTC_Get_Date());
    //dir = RTC_Get_Date();
    makeDirSucces = Make_Clear_Directory(dir);
    if(makeDirSucces == true)
    timer = millis();
  }
  RTC_Get_Current_Time(); //Internal_RTC değişkeni üzerinde anlık zaman ve tarih bilgisi tutulmakta.
  delay(1000);
//   display.clearDisplay();
//     display.setTextSize(1);
//  display.setTextColor(WHITE);
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
//  Serial.print(i+1);
//  Serial.print(". Loraya Veri Gönderildi - ");
//  Serial.println(rs.getResponseDescription());
// 
  delay(2000);
  kontrol=0;
  while (e32ttl.available()>1){
    ResponseStructContainer rsc = e32ttl.receiveMessage(sizeof(Signal));
  
   data=*(Signal*)rsc.data;
//    Serial.print(i+1);
//     Serial.print(". Loradan Gelen veri : ");
//     Serial.print(data.type);
//     Serial.print(" , ");
//     Serial.println(data.temp);
//     Serial.println("/*/");
     rsc.close();
     kontrol=1;
     
     sendMqtt(data);//data.type & data.temp
     if(makeDirSucces == true){
      String toSD =String(data.type)+","+String(data.temp);
      File_Date_Write(dir, toSD);
      RTC_Get_Current_Time();
      Show_Internal_RTC();
     }
    }
    if(!(kontrol)){
       //   Serial.print(i+1);
       //  Serial.println(". Loradan  veri Gelmedi??????");
       //  Serial.println("/*/");
    }
 }
  
}
void sendMqtt(Signal data){
Serial.print(data.type);
Serial.print(",");
Serial.println(data.temp);
delay(10);
}

void proccesBlink(){
  for(byte i = 0; i < 5; i++){
    delay(500);
    Serial.print(".");
  }
}

char* String_to_CharArray(String data){
  String str = String(data);
  int str_len = str.length(); 
  char charArray [str_len];
  str.toCharArray(charArray, str_len);
  return charArray;
}

boolean SD_Card_Init(){
  Serial.print("\nInitializing SD card");
  proccesBlink(); Serial.println();
  if (!SD.begin(4)) { //SD.begin() metoduna başka bir dijital pin de bağlanabilir. Önceden pinMode(pin, OUTPUT) olarak tanımlanmalıdır.
    Serial.println("Initialization failed!\n");
    //while (1);
    return false;
  }
  Serial.println("Initialization succesful.\n");
  delay(1000);
  return true;
}

String confDirName(String temp){//
  byte len = temp.length();
  String dirName = "";
  for(byte i = 0; i < len; i++){
    char ch = temp[i];
    if(ch != '.')
    dirName += ch;
  }
  return dirName;
}

boolean Make_Directory(char* dosyaAdi, char* uzanti){
  String dirr = String(dosyaAdi) + "." + String(uzanti);
  if(SD.exists(dir)){
    Serial.println("\nBu dosya zaten bulunmakta.");
    return false;
  }
  Serial.print("Dosya Olusturuluyor");
  proccesBlink();Serial.println();
  myFile = SD.open(dirr, FILE_WRITE);
  myFile.close();
  delay(250);
  if(SD.exists(dirr)){
    Serial.println(dir + " Olusturuldu.");
    return true;
  }
  else{
    Serial.println("Dosya Olusturulamadi !");
    return false;
  }
}

boolean Make_Clear_Directory(String dosyaAdi){
  //String dirr = String(dosyaAdi) + "." + String(uzanti);
  if(SD.exists(dosyaAdi)){
    Serial.println("\nBu dosya zaten bulunmakta.");
    delay(500);
    Serial.println("Mevcut dosya siliniyor.");
    SD.remove(dosyaAdi);
  }
  Serial.print("Dosya Olusturuluyor");
  proccesBlink();Serial.println();
  myFile = SD.open(dosyaAdi, FILE_WRITE);
  myFile.println("########## " + RTC_Get_Date() + " ##########\n");
  myFile.close();
  delay(250);
  if(SD.exists(dosyaAdi)){
    Serial.println(dir + " Olusturuldu.");
    return true;
  }
  else{
    Serial.println("Dosya Olusturulamadi !");
    return false;
  }
}

void File_Write(String dosyaAdi, String data){
  myFile = SD.open(dosyaAdi, FILE_WRITE);
  data = String(RTC_Get_Time()) + " LoRa Gateway ---> " + String(data) + "\n";
  if(myFile){
    Serial.print("\nWriting to " + dosyaAdi); //proccesBlink();
    myFile.print(data);
    Serial.println("\nWriting Succesfuly.");
  }
  else
  Serial.println("Writing Error!");
  myFile.close();
}

void File_Clear_Write(String dosyaAdi, String data){
  if(SD.exists(dosyaAdi))
  SD.remove(dosyaAdi);  //Dosya varsa sil.
  delay(200);
  myFile = SD.open(dosyaAdi, FILE_WRITE); //Dosyayı oluştur.
  data = String(RTC_Get_Time()) + " LoRa Gateway ---> " + String(data) + "\n";
  if(myFile){
    Serial.print("\nWriting to " + dosyaAdi); proccesBlink();
    myFile.print(data);
    Serial.println("\nWriting Succesfuly.");
  }
  else
  Serial.println("Writing Error!");
  myFile.close();
}

void File_Date_Write(String dosyaAdi, String data){
  myFile = SD.open(dosyaAdi, FILE_WRITE);
  //myFile.println("########## " + RTC_Get_Date() + " ##########\n");
  data = String(RTC_Get_Time()) + " ---> LoRa Gateway : " + String(data) + "\n";
  if(myFile){
    Serial.print("\nWriting to " + dosyaAdi); //proccesBlink();
    myFile.print(data);
    Serial.println("\nWriting Succesfuly.");
  }
  else
  Serial.println("Writing Error!");
  myFile.close();
}

String File_Read(String dosyaAdi){
  String rxBuffer = "";
  myFile = SD.open(dosyaAdi);
  if(myFile){
    Serial.print("\nReading from " + dosyaAdi); //proccesBlink(); Serial.println();
    while(myFile.available()){
      char inChar = myFile.read();
      rxBuffer += inChar;
      //rxBuffer = myFile.readStringUntil('\r');
    }
    myFile.close();
    Serial.println("Reading Succesfuly.");
    return rxBuffer;
  }
  else
  Serial.println("Reading Error!");
  return "\0";
}

String File_ReadLine(String dosyaAdi){
  String rxBuffer = "";
  myFile = SD.open(dosyaAdi);
  if(myFile){
    Serial.print("\nSatir Okuma Islemi Basladi"); proccesBlink(); Serial.println();
    byte line = 1;
    while(myFile.available()){
      rxBuffer = myFile.readStringUntil('\n');
      Serial.println(String(line) + ". Satir : " + rxBuffer);
      line++;
    }
    myFile.close();
    Serial.println("Okuma Islemi Bitti.");
    return rxBuffer;
  }
  else
  Serial.println("Okuma Islemi Baslatilamadi !");
  return "\0";
}

void File_Remove(String dosyaAdi){
  if(!SD.exists(dosyaAdi))
  SD.remove(dosyaAdi);
}

void RTC_Get_Current_Time(){
  String zaman = RTC_Get_Time();
  String tarih = RTC_Get_Date();

  Internal_RTC[0] = String(zaman[0]) + String(zaman[1]);  //Saat
  Internal_RTC[1] = String(zaman[3]) + String(zaman[4]);  //Dakika
  Internal_RTC[2] = String(zaman[6]) + String(zaman[7]);  //Saniye
  
  Internal_RTC[3] = String(tarih[0]) + String(tarih[1]);  //Gun
  Internal_RTC[4] = String(tarih[3]) + String(tarih[4]);  //Ay
  Internal_RTC[5] = String(tarih[6]) + String(tarih[7]) + String(tarih[8]) + String(tarih[9]);  //Yıl
}

void Show_Internal_RTC(){
  Serial.println("Tarih -> " + Internal_RTC[3] + "." + Internal_RTC[4] + "." + Internal_RTC[5]);
  Serial.println("Zaman -> " + Internal_RTC[0] + ":" + Internal_RTC[1] + ":" + Internal_RTC[2]);
}

void RTC_Time_Update(){
  rtc.setDOW(WEDNESDAY);
  rtc.setTime(9, 10, 50);
  rtc.setDate(19, 4, 2022);
  //Internal_RTC
}

String RTC_Get_Date(){
  return rtc.getDateStr();
}

String RTC_Get_Time(){
  return rtc.getTimeStr();
}

void Show_Date_Time(){
  // Send Day-of-Week
  Serial.print(rtc.getDOWStr());
  Serial.print(" ");
  
  // Send date
  Serial.print(rtc.getDateStr());
  Serial.print(" -- ");

  // Send time
  Serial.println(rtc.getTimeStr());
}
