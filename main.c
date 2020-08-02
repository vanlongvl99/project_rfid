
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h> //library responsible for communicating with the module RFID-RC522
#include <SPI.h> //library responsible for communicating of SPI bus
#define SS_PIN    16
#define RST_PIN   17
#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16
#define greenPin     12
#define redPin       32
#define MAX 15

//List of members
char *Name[MAX]={"Bao Nguyen","Loc dam","Cong Hoang"," Trung Hieu","Xuan Tien","Tien Thang","Anh hung"","Trong nghia","Van Long"};
//List of UIDs
int UID[MAX]={1977547202,1185140471,2521087479,3035878275,388956443,640750325,3863374837,2799210996,3063437045};

//used in authentication
MFRC522::MIFARE_Key key;
//authentication return status code
MFRC522::StatusCode status;
// Defined pins to module RC522
MFRC522 mfrc522(SS_PIN, RST_PIN); 

// Replace with your network credentials
const char* ssid     = "Van long";
const char* password = "vanlong99";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;


//bien cho RFID
unsigned long uidDec, uidDecTemp; // hien thi so UID dang thap phan
byte bCounter, readBit;
unsigned long ticketNumber;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x3F, lcdColumns, lcdRows);  

String messageStatic = "Static message";
String messageToScroll = "This is a scrolling message with more than 16 characters";

// Replace with your unique IFTTT URL resource
const char* resource = "/trigger/attendance/with/key/eoIM0A9Uwelrl5GP3MgVk6xDHyYEtLUIZ80Q6ac0dgX";

// Maker Webhooks IFTTT
const char* server = "maker.ifttt.com";


void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  SPI.begin();            
  mfrc522.PCD_Init();     
  Serial.println("Prilozhite kartu / Waiting for card...");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT 
  timeClient.setTimeOffset(25200);

  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight(); 
  
}
void loop() {
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.print("DATE: ");
  Serial.println(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);
  // set cursor to first column, first row
  lcd.setCursor(0, 0);
  // print static message
  lcd.print(timeStamp);
  // print scrolling message
  delay(1000);

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Doc the
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }

  uidDec = 0;
  Serial.println("Pyroject attendance");
  Serial.println("Please insert your card");

  // Display UID on terminal
  Serial.println("Card UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
    uidDecTemp = mfrc522.uid.uidByte[i];
    uidDec = uidDec*256+uidDecTemp;
  } 
  Serial.print("            [");
  Serial.print(uidDec);
  //lcd.print(uidDec);

  lcd.setCursor(0,1);
  //Tra cuu the
  for(int i=0;i<MAX;i++)
  {
  if(uidDec == UID[i])
  {
   lcd.print("Chao"+ String(Name[i]));
    delay(3000);
    Serial.print("Connecting to "); 
  Serial.print(server);
  
  WiFiClient client;
  int retries = 5;
  while(!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if(!!!client.connected()) {
    Serial.println("Failed to connect...");
  }
  
  Serial.print("Request resource: "); 
  Serial.println(resource);

  // Temperature in Celsius
  String jsonObject = String("{\"value1\":\"") + Name[i] + "\"\"value2\":\"" + dayStamp  + "\"\"value3\":\"" + timeStamp + "\"}";
                                      
  client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server); 
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length());
  client.println();
  client.println(jsonObject);
        
  int timeout = 5 * 10; // 5 seconds             
  while(!!!client.available() && (timeout-- > 0)){
    delay(100);
  }
  if(!!!client.available()) {
    Serial.println("No response...");
  }
  while(client.available()){
    Serial.write(client.read());
  }
  
  Serial.println("\nclosing connection");
  client.stop(); 
  }
  else{
  }
  }
  //--------------------------------
  
  Serial.println("]");
  Serial.println("================================================");
 
//instructs the PICC when in the ACTIVE state to go to a "STOP" state
  mfrc522.PICC_HaltA(); 
  // "stop" the encryption of the PCD, it must be called after communication with authentication, otherwise new communications can not be initiated
  mfrc522.PCD_StopCrypto1();
  lcd.setCursor(0,1);
    lcd.print("Pyroject      ");
}