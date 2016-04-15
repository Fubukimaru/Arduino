/*
   Web client sketch for IDE v1.0.1 and w5100/w5200
   Uses POST method.
   Posted November 2012 by SurferTim
*/

#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

byte mac[] = {  
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//Change to your server domain
char serverName[] = "www.ambmobilitat.cat";

// change to your server's port
int serverPort = 80;

// change to the page on that server
char pageName[] = "/ambtempsbus";

//Exp
char expresion[] = "60 - ";
//char expresion[] = "32 - ";
int esize = 5;
char tiempo[] = "99 min.<";

bool first = true;

EthernetClient client;
int totalCount = 0;
// insure params is big enough to hold your variables
char params[32];

// set this to the number of milliseconds delay
// this is 30 seconds
#define delayMillis 10000UL

unsigned long thisMillis = 0;
unsigned long lastMillis = 0;

void setup() {
  lcd.begin(16, 2); 
  lcd.clear();
  lcd.println("START");
  Serial.begin(9600);

  // disable SD SPI
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);

  Serial.print(F("Starting ethernet..."));
  if(!Ethernet.begin(mac)) Serial.println(F("failed"));
  else Serial.println(Ethernet.localIP());

  lcd.clear();
  lcd.println("ETH ok. Wait.");
  delay(2000);
  Serial.println(F("Ready"));
  lcd.clear();
  lcd.println("READY");;
}

void loop()
{
  // If using a static IP, comment out the next line
  Ethernet.maintain();

  thisMillis = millis();

  if(thisMillis - lastMillis > delayMillis)
  {
    lastMillis = thisMillis;

    // params must be url encoded.
    sprintf(params,"codi=817");//%i",totalCount);    
    if(!postPage(serverName,serverPort,pageName,params)) Serial.print(F("Fail "));
    else Serial.print(F("Pass "));
    totalCount++;
    Serial.println(totalCount,DEC);
  }    
}


byte postPage(char* domainBuffer,int thisPort,char* page,char* thisData)
{
  int inChar;
  char outBuf[64];

  Serial.print(F("connecting..."));

  if(client.connect(domainBuffer,thisPort) == 1)
  {
    Serial.println(F("connected"));

    // send the header
    sprintf(outBuf,"POST %s HTTP/1.1",page);
    client.println(outBuf);
    sprintf(outBuf,"Host: %s",domainBuffer);
    client.println(outBuf);
    client.println(F("Connection: close\r\nContent-Type: application/x-www-form-urlencoded"));
    sprintf(outBuf,"Content-Length: %u\r\n",strlen(thisData));
    client.println(outBuf);

    // send the body (variables)
    client.print(thisData);
  }
  else
  {
    Serial.println(F("failed"));
    return 0;
  }

  int connectLoop = 0;
  
  int i = 0;
  bool markerOK = false;  
  int t = 0;

  Serial.println(tiempo);

  first = true;
  while(client.connected())
  {
    while(client.available())
    {
      inChar = client.read();
      //Serial.write(inChar);
      connectLoop = 0;
      if (!markerOK) {
        if (inChar == expresion[i]) {
          ++i;        
        } else {
          i = 0;
        }
  
        //IF it matches with expression
        if (i == esize) {
          Serial.println("MATCH!");
          i = 0;
          markerOK = true;
        }
      } else {
        if (inChar == '<') {
          Serial.println("CODO OPEN");
          while(inChar != '>') { //Discard decorations
             inChar = client.read(); 
          }
          t = 0;
          while(inChar != '<') {
            inChar = client.read(); 
            Serial.print(inChar);
            tiempo[t++] = inChar; 
          }          
          Serial.println("TIEMPO");
          Serial.println(tiempo);          
          if (first) {
            lcd.clear();
            first= false;
          }
          else {
            lcd.setCursor(0, 1); 
          }
          lcd.println(tiempo);
          //sendToScreen(tiempo);
          markerOK = false;
        }
      }
      
    }

    delay(1);
    connectLoop++;
    if(connectLoop > 10000)
    {
      Serial.println();
      Serial.println(F("Timeout"));
      client.stop();
    }
  }

  Serial.println();
  Serial.println(F("disconnecting."));
  client.stop();
  return 1;
}
