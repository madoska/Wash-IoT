// ***** WIFI CONFIGURATION *****
#include <ESP8266WiFi.h>
const char* ssid = "telenet-4DDBB";
const char* password = "USKUKFQ9TZCf";
const char* host = "maker.ifttt.com";

// ***** IR RECEIVER LIBRARY by crankyoldgit***
//(https://github.com/crankyoldgit/IRremoteESP8266)
// necessary to decode infrared signals
#include <IRremoteESP8266.h>
#include <IRrecv.h>

const uint16_t kRecvPin = 14; // IR receiver in GPIO 14 (INPUT)
IRrecv irrecv(kRecvPin);
decode_results results;

// ***** VARIABLES *****
#define start_btn  16738455   // remotes' code for button 0
#define stop_btn  16732845    // remotes' code for button 9
#define blue 13               // LED BLUE in 13 (OUTPUT)
#define red 16                // LED RED in 16 (OUTPUT)
#define green 15              // LED GREEN in 15 (OUTPUT)

const int interval = 4000;   // 20 seconds of handwashing
unsigned long start;          // start, stop & elapsed variables for stopwatch
unsigned long stop;           // use longs because millis() can return large values over time
unsigned long elapsed;

void setup() {
  Serial.begin(9600);       // rate at which information is transferred
  Serial.println();         // ↑ must match baud rate in serial monitor
    
  // ***** WIFI CONNECTION *****
  Serial.print("Connecting to ");
  Serial.print(ssid);
  
  // initialize WiFi network settings
  WiFi.begin(ssid, password);             
  while(WiFi.status() != WL_CONNECTED){   // as long as status = not connected
    delay(500);                              // print a dot every 0.5 seconds
    Serial.print(".");
  }
  Serial.println();

  // once connected print the IP address assigned to ESP8266
  Serial.println("WiFi successfully connected!");
  Serial.print("Your IP address: ");
  Serial.println(WiFi.localIP());
  
  Serial.println("Starting WashIoT");
  
  // ***** IR RECEIVER by crankyoldgit*****
  irrecv.enableIRIn();      // start the receiver
  while (!Serial){          // wait for the serial connection to be establised
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("WashIoT is running! Please wash your hands well~ :)");

  pinMode(blue, OUTPUT);    // set the pins for the LEDs to OUTPUT
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
}

void loop() {
  if (irrecv.decode(&results)) {           // IR RECEIVER by crankyoldgit
    unsigned int result = results.value;  // values of decoded infrared signals
    digitalWrite(blue, LOW);              // start with all LEDs turned off
    digitalWrite(red, LOW);
    digitalWrite(green, LOW);
    

    // ***** IF START BUTTON IS PRESSED *****
    if(result == start_btn){        // millis() starts counting as soon as program starts running
        start = millis();           // save start time in "start"
        Serial.println(start);

        digitalWrite(blue, HIGH);   // blue LED to indicate start button has been pressed
     }

    // ***** IF STOP BUTTON IS PRESSED *****
     if(result == stop_btn){
        stop = millis();            // save stop time in "stop"
        elapsed = (stop-start);     // calculate time elapsed between start & stop
        if(elapsed <= interval){      // if elapsed time is shorter than required time
          digitalWrite(red, HIGH);    // turn on red LED
          
          Serial.print("FAILED - Elapsed time: ");
          Serial.print(elapsed/1000); // calculate seconds
          Serial.println(" seconds. Sending tweet now.");
          Serial.print("Connecting to ");
          Serial.println(host);

          WiFiClient client;
          const int httpPort = 80;
          // connect to specified IP address and port; connect() supports DNS lookup
           if (!client.connect(host, httpPort)) {
              Serial.println("Connection failed");
              return;
           }

          // trigger handwashing event in IFTTT
          String url = "/trigger/hands_notwashed/with/key/ePNnjI3d96299R1yzjm6oEecaTdTku0IxfvpObpWXNm";
          Serial.print("Triggering hands_notwashed event at: ");
          Serial.println(url);
  
          client.print(String("GET ") + url + " HTTP/1.1\r\n" +
             "Host: " + host + "\r\n" + 
             "Connection: close\r\n\r\n");
          delay(500);
  
          while(client.available()){
          String line = client.readStringUntil('\r');
          Serial.print(line);
          }

          Serial.println();
          Serial.println("Tweet successfully posted. Closing connection");
          
        } else {      
          digitalWrite(green, HIGH);                
          Serial.println(elapsed);    // else don't send tweet
        }
     }
    
    irrecv.resume();  // receive the next value
  }
  delay(100);
} 
