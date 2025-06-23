#include <WiFi.h>
#include <Stepper.h>

// WiFi credentials
const char* ssid = "StepperMotor123";
const char* password = "abcdefghij";

// Server on port 80
WiFiServer server(80);

// Stepper configuration
const int stepsPerRevolution = 40;
Stepper myStepper1(stepsPerRevolution, 32, 33, 25, 26); // motor 1
Stepper myStepper2(stepsPerRevolution, 19, 18, 22, 23); // motor 2

// Relay pins
const int relay1 = 13; // Grow light
const int relay2 = 12; // Water pump
const int relay3 = 14; // Fan 1
const int relay4 = 27; // Fan 2

// State strings
String stateRelay1 = "off";
String stateRelay2 = "off";
String stateRelay3 = "off";
String stateRelay4 = "off";
String stateStepper = "off";

String header;
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);

  // Set relay pins as outputs
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  // Set default relay states (OFF = HIGH if active LOW relays)
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);

  myStepper1.setSpeed(100);
  myStepper2.setSpeed(100);

  WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";

    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Handle GET requests
            if (header.indexOf("GET /32/on") >= 0) {
              stateStepper = "on";
              myStepper1.step(150);
              myStepper2.step(-150);
            } else if (header.indexOf("GET /32/off") >= 0) {
              stateStepper = "off";
              myStepper1.step(-150);
              myStepper2.step(150);
            }

            if (header.indexOf("GET /relay1/on") >= 0) {
              digitalWrite(relay1, LOW); stateRelay1 = "on";
            } else if (header.indexOf("GET /relay1/off") >= 0) {
              digitalWrite(relay1, HIGH); stateRelay1 = "off";
            }

            if (header.indexOf("GET /relay2/on") >= 0) {
              digitalWrite(relay2, LOW); stateRelay2 = "on";
            } else if (header.indexOf("GET /relay2/off") >= 0) {
              digitalWrite(relay2, HIGH); stateRelay2 = "off";
            }

            if (header.indexOf("GET /relay3/on") >= 0) {
              digitalWrite(relay3, LOW); stateRelay3 = "on";
            } else if (header.indexOf("GET /relay3/off") >= 0) {
              digitalWrite(relay3, HIGH); stateRelay3 = "off";
            }

            if (header.indexOf("GET /relay4/on") >= 0) {
              digitalWrite(relay4, LOW); stateRelay4 = "on";
            } else if (header.indexOf("GET /relay4/off") >= 0) {
              digitalWrite(relay4, HIGH); stateRelay4 = "off";
            }

            // Webpage
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close\n");

            client.println("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<style>html { font-family: monospace; text-align: center;} .button { background-color: yellowgreen; border: none; color: white; padding: 16px 40px; font-size: 20px; cursor: pointer;} .button2 { background-color: gray; }</style></head><body>");

            client.println("<h1>ESP32 Control Panel</h1>");

            // Stepper
            client.println("<h2>Cover</h2>");
            client.println(stateStepper == "off" ? "<p><a href=\"/32/on\"><button class=\"button\">OPEN</button></a></p>" : "<p><a href=\"/32/off\"><button class=\"button button2\">CLOSE</button></a></p>");

            // Relays
            client.println("<h2>Grow Light</h2>");
            client.println(stateRelay1 == "off" ? "<p><a href=\"/relay1/on\"><button class=\"button\">ON</button></a></p>" : "<p><a href=\"/relay1/off\"><button class=\"button button2\">OFF</button></a></p>");

            client.println("<h2>Water Pump</h2>");
            client.println(stateRelay2 == "off" ? "<p><a href=\"/relay2/on\"><button class=\"button\">ON</button></a></p>" : "<p><a href=\"/relay2/off\"><button class=\"button button2\">OFF</button></a></p>");

            client.println("<h2>Fan 1</h2>");
            client.println(stateRelay3 == "off" ? "<p><a href=\"/relay3/on\"><button class=\"button\">ON</button></a></p>" : "<p><a href=\"/relay3/off\"><button class=\"button button2\">OFF</button></a></p>");

            client.println("<h2>Fan 2</h2>");
            client.println(stateRelay4 == "off" ? "<p><a href=\"/relay4/on\"><button class=\"button\">ON</button></a></p>" : "<p><a href=\"/relay4/off\"><button class=\"button button2\">OFF</button></a></p>");

            client.println("</body></html>\n");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
  }
}
