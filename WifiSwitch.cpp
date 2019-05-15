
/*
  Use ESP 32 to make a switched 5V power source
    Setup local network SSID/Pass in the code

    CONNECTIONS
    - 5V supply connected to both output and ESP 32 5V
    - Using IRLB8721, (1) gate connected to outPin on esp32, (2) drain to the output ground
        and (3) to supply ground with esp32

    STARTUP
    - Blinks 3 times on WiFi connection waits 1s
    - Blinks # of times set to last set of nums in IP

    Usage
    - Go to local ip (e.g 192.168.0.4) in browser
    - Press on/off, Blue LED is on when output is active

*/

// Load Wi-Fi library
#include <WiFi.h>

// Replace with your network credentials
const char* ssid     = "SSID";
const char* password = "PASSWORD";
int blinkCount = 3;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String outState = "off";

// Assign output variables to GPIO pins

const int outPin = 23;
const int ledPin = 2;

void blinkLED(){
digitalWrite(ledPin,HIGH);
delay(500);
digitalWrite(ledPin,LOW);
delay(500);
}

void setup() {
  Serial.begin(9600);
  // Initialize the output variables as outputs
  pinMode(outPin,OUTPUT);
  pinMode(ledPin,OUTPUT);
  // Set outputs to LOW
  digitalWrite(outPin,LOW);
  digitalWrite(ledPin,LOW);

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

  int ipCount = WiFi.localIP()[3];
  server.begin();

  // // Blink LED 3 times on startup
  for (int i = 0; i<blinkCount; i++){
  blinkLED();
  }

  delay(1000);
  for (int i = 0; i<ipCount; i++){
    blinkLED();
  }


}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
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
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /cc/on") >= 0) {
              Serial.println("cc on");
              outState = "off";
              digitalWrite(outPin, HIGH);
              digitalWrite(ledPin, HIGH);
            } 
            
            else if (header.indexOf("GET /cc/off") >= 0) {
              Serial.println("cc off");
              outState = "on";
              digitalWrite(outPin, LOW);
              digitalWrite(ledPin, LOW);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Chromecast</h1>");


            
                
            if (outState=="on") {
              client.println("<p><a href=\"/cc/on\"><button class=\"button button2\">OFF</button></a></p>");
            } 
            else {
              client.println("<p><a href=\"/cc/off\"><button class=\"button\">ON</button></a></p>");
            } 
                    
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
