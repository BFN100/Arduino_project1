
// Load Wi-Fi library
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MyWifi.h"

//OLED screen
#define SEALEVELPRESSURE_HPA (1013.25)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// BME Temperature
Adafruit_BME280 bme; // I2C

// Replace with your network credentials
const char* ssid = STASSID;
const char* password = STAPSK;

// For the buzzer
const int buzzer = 16;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String led1State = "off";
String led2State = "off";
String led3State = "off";
String led4State = "off";

// Button variables
#define bouton 13
bool lectBouton;
bool boutonEtat = LOW;
bool boutonPrec = LOW;
bool count = true;

// Loop helpers
int i = 0;
int j = 0;

// Assign output variables to GPIO pins
const int led1 = 12;
const int led2 = 14;
const int led3 = 0;
const int led4 = 2;
int led[4] = {12, 14, 0, 2};

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  pinMode(OLED_RESET, OUTPUT); 
  Serial.begin(74880);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  Serial.println(F("BME280 test"));

  bool status;

  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not detect a BME280 sensor, Fix wiring Connections!");
    while (1);
  }

  Serial.println("-- Print BME280 readings--");
  Serial.println();
  
  // Initialize the output variables as outputs
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);

  //Bouton 
  pinMode(bouton, INPUT_PULLUP);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  // How the button works
  boutonPrec = boutonEtat;
  boutonEtat = !digitalRead(bouton);

  // Loop calculation variables
  unsigned long currentMillis = millis();
  unsigned long previousMillis = 0;
  const long interval = 600;

  if(boutonEtat == HIGH && boutonPrec == LOW)
  {
    count = !count;
  }

  if(count == true)
  {
    if (currentMillis - previousMillis >= interval) 
      {
       previousMillis = currentMillis;
       
       if(i > 15)
       {
        i=0;
       }
     
      for(int j=0; j<=3; j++)
       {
          if(((i>>j)&1)==1) 
          {
            digitalWrite(led[j], HIGH);
          } 
          else 
          {
            digitalWrite(led[j], LOW);
          }
          
        } 
     
        i++;
        Serial.print(i);
        Serial.print(" ");
        lectBouton = !digitalRead(bouton);
     }
   }else{

      //A little melody
      tone(buzzer,261);
      delay(200);
      noTone(buzzer); 
        
      tone(buzzer,293);             
      delay(200);    
      noTone(buzzer); 
      
      tone(buzzer,329);      
      delay(200);
      noTone(buzzer);   
        
      tone(buzzer,349);    
      delay(200);    
      noTone(buzzer); 
      
      tone(buzzer,392);            
      delay(200);
      noTone(buzzer); 
      
      count = true; //returns the operation of lights and temperature
   }
   
  printTemperature();

  //CONEXION
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            //HOW THE PAGE WEB WORKS
            if (header.indexOf("GET /Climat/on") >= 0) {
              
              //Here I show the weather information in the browser
              client.print("<p class=\"temp\">Temperature = ");
              client.print(bme.readTemperature());
              client.print(" *C </p>");
              client.print(" ");
            
              client.print("<p class=\"temp\"> Pressure = ");
              client.print(bme.readPressure() / 100.0F);
              client.print(" hPa </p>");
              client.print(" ");
              
              client.print("<p class=\"temp\"> Approx. Altitude = ");
              client.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
              client.print(" m </p>");
              client.print(" ");
              
              client.print("<p class=\"temp\"> Humidity = ");
              client.print(bme.readHumidity());
              client.print(" % </p>");
              client.print(" ");
              
            } else if (header.indexOf("GET /Buzzer/on") >= 0) {
              
              //A little melody2
              tone(buzzer,500);
              delay(200);
              noTone(buzzer); 
                
              tone(buzzer,300);             
              delay(200);    
              noTone(buzzer); 
              
              tone(buzzer,250);      
              delay(200);
              noTone(buzzer);   
                
              tone(buzzer,250);    
              delay(200);    
              noTone(buzzer); 
              
              tone(buzzer,250);            
              delay(200);
              noTone(buzzer); 
              
              tone(buzzer,300);            
              delay(200);
              noTone(buzzer);
              
              tone(buzzer,300);            
              delay(200);
              noTone(buzzer);  
            } 
            
            // Pour afficher le HTML
            client.println("<!DOCTYPE html><html>"); //pour montrer que le code que j'envoie est HTML
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"); //pour la réactivité
            client.println("<link rel=\"icon\" href=\"data:,\">"); 
            
            // Voici le style CSS de votre page
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.print(".button {background-color: white; 1px solid; color: black; padding: 15px 20px; margin-top: 15px; border-radius: 6px; border-color: #D3D3D3; font-size: 20px; font-weight: bold;}");
            client.print(".button:hover {color: #FF1493;}");
            client.print(".button:focus {background-color: #FFB6C1; color: black;}");
            client.println("h1 {color: white;}");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.print(".button2 {background-color: white; 1px solid; color: black; padding: 15px 20px; margin-top: 15px; border-radius: 6px; border-color: #D3D3D3; font-size: 20px; font-weight: bold; }");
            client.print(".button2:hover {color: #FF1493;}");
            client.print(".button2:focus {background-color: #FFB6C1; color: black;}");
            client.print(".temp {color: white; font-size: 20px; font-weight: bold; margin-top: 20px; margin-bottom: 20px;}");
            client.println("body {background-image: linear-gradient( to right, #FF69B4, #DA70D6, #7B68EE, #7B68EE);}</style></head>");
            
            // titre
            client.println("<body><h1>Selectionnez ce que vous voulez voir: </h1>");
            
            // Premier bouton qui affichera la météo       
            client.println("<p><a href=\"/Climat/on\"><button class=\"button\">Climat</button></a></p>");

            // Le bouton deux déclenchera une mélodie sur le buzzer        
            client.println("<p><a href=\"/Buzzer/on\"><button class=\"button\">Buzzer</button></a></p>");
          
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    
    // Clear the header variable
    header = "";
    
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");  
  }
}

void printTemperature(void) {
  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  
  display.print("Temperature = ");
  display.print(bme.readTemperature());
  display.println(" *C");

  display.print("Pressure = ");
  display.print(bme.readPressure() / 100.0F);
  display.println(" hPa");
  
  display.print("Approx. Altitude = ");
  display.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  display.println(" m");
  
  display.print("Humidity = ");
  display.print(bme.readHumidity());
  display.println(" %");
  display.display(); //TOUJOUR METTRE ÇA, PARCE QUE C'EST ÇA QU'AFFICHE TOUS LES CHOSES!
  delay(1000);
}
