#include <SPI.h>
#include <Ethernet.h>
#define trigPin 6 
#define echoPin 7 
#define MAX_DISTANCE 200 
#define led 3

byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0xFE, 0x40 };
byte ip[] = { 192,168,0,28 };    

float timeOut = MAX_DISTANCE * 60;
int soundVelocity = 340; 
String query;
String ledState;
boolean currentLineIsBlank = true;
float changedDistance = 0;
float distance = 0;
boolean isTrue;


EthernetServer server = EthernetServer(3013); 
EthernetClient client_supersonic;


void setup() {
  Serial.begin(9600);
  
  Ethernet.begin(mac, ip); 
  server.begin(); 
  Serial.println("server is at ");
  Serial.println(Ethernet.localIP());

  pinMode(trigPin,OUTPUT);
  pinMode(echoPin,INPUT);
  pinMode(led,OUTPUT);
  
  Serial.println("starting simple arduino client test");
  Serial.println();
}

void loop() {
  String ulaz = "";
  EthernetClient client = server.available(); 

  if (client) {
    while(client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (c == '\n' && currentLineIsBlank) {
          writeResponse(ledState, &client);
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if ( c!= '\r') { // po standardu
          currentLineIsBlank = false;
        }
        if (ulaz.length() < 100) {
          ulaz += c;
        }
        if (c == '\n') {
          if (ulaz.indexOf("?upali") > 0) {
            digitalWrite(led, HIGH);
            ledState = "HIGH";
            isTrue = true;
          } else if(ulaz.indexOf("?ugasi") > 0) {
            digitalWrite(led, LOW);
            ledState = "LOW";
            isTrue = false;
          }
        }
      }
    }
    client.stop();
    Serial.println("client disconnected");
  }
  //-------------------------------------------------------
  distance = getSonar();
  if (client_supersonic.connect("192.168.0.19",8010) && isTrue) {
    query = "POST /measured-distance?distance=";
    distance = getSonar();
    changedDistance = distance; //radi testiranja
    query = query + String(distance) + " HTTP/1.0";
    Serial.println(query);

    client_supersonic.println(query);
    client_supersonic.println("Host: 192.168.0.19:8010"); 
    client_supersonic.println("Content-Type: application/x-www-form-urlencoded"); // DATA TYPE
    client_supersonic.println();
  } else {
    client_supersonic.println("Failed sending data");
  }
  if (client_supersonic.connected()) { 
    client_supersonic.stop();  // DISCONNECT FROM THE SERVER
  }
  delay(1000);
  isTrue = false;
}
//-------------------------------------------------------
void writeResponse(String ledState, EthernetClient *client) {
  client -> println("HTTP/1.1 200 OK");
  client -> println(" application/json");
  client -> println("Connection: close");  
  client -> println("Access-Control-Allow-Origin: *");
  client -> println();
  client -> print(ledState);
}

float getSonar() {
 unsigned long pingTime;
 float distance;
 
 digitalWrite(trigPin, HIGH); 
 delayMicroseconds(10);
 digitalWrite(trigPin, LOW);
 
 pingTime = pulseIn(echoPin, HIGH, timeOut); 
 distance = (float)pingTime * soundVelocity / 2 / 10000; 

 return distance; 
}
