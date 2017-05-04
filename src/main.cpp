#include <ESP8266WiFi.h>                        // Follow instructions @ https://www.wemos.cc/tutorial/get-started-arduino.html
#include <Arduino.h>
#include <Hash.h>
#include <Servo.h>
#include <WebSocketsServer.h>           // From library : WebSockets
#include <ArduinoJson.h>                        // From library : ArduinoJson

#define PIN_SERVO                       14
#define PIN_SERVO_OUT           0

WebSocketsServer webSocket = WebSocketsServer(80);
Servo servo;

void webSocket_OnEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  Serial.printf("WS Evt %d\n", type);
  if(type == WStype_TEXT)
  {
    Serial.println((char *)payload);
    StaticJsonBuffer<1024> jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(payload);
    if(!json.success()) {
      Serial.println("lol");
      return;
    }

    if(json.containsKey("action") && json["action"] == "servo_set_pwm" && json.containsKey("pwm"))
    {
      // {"action":"servo_set_pwm", "pwm":500}
      int pwmValue = json["pwm"];
      servo.attach(PIN_SERVO, 0, 2500);
      servo.writeMicroseconds(pwmValue);
      Serial.printf("RX servo_set_pwm | PWM %d\n", pwmValue);
    }
    else if(json.containsKey("action") && json["action"] == "servo_detach")
    {
      // {"action":"servo_detach"}
      servo.detach();
      Serial.printf("RX servo_detach\n");
    }
    else if(json.containsKey("action") && json["action"] == "servo_get_position")
    {
      // {"action":"servo_get_position"} --> reply {"position":559}
      int ADCvalue = analogRead(PIN_SERVO_OUT);
      webSocket.sendTXT(num, "{\"position\":" + String(ADCvalue) + "}");
      Serial.printf("RX servo_get_position | %d\n", ADCvalue);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  Serial.print("\n\nConnecting to Wifi...\n");
  WiFi.begin("FastAndSerious", "42FastAndSerious42", 4);

  // Wait until we are connected, then print the IP
  while(WiFi.waitForConnectResult() != WL_CONNECTED);
  Serial.print("Connected | IP " + WiFi.localIP().toString() + "\n");

  // Start WS
  webSocket.begin();
  webSocket.onEvent(webSocket_OnEvent);
}

void loop()
{
  webSocket.loop();
}
