#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Daikin.h>

#include "EspMQTTClient.h"

#include "private_info.h"

int isTrigger = 0;
const int ledPin =  LED_BUILTIN;// the number of the LED pin
const int buttonPin = 13; //D7
const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).

IRDaikinESP ac(kIrLed);  // Set the GPIO to be used to sending the message

// PI params should put in pirvate_info.h
EspMQTTClient client(
  PI_ssid,
  PI_pin,
  PI_serverIp,  // MQTT Broker server ip
  PI_usr,   // Can be omitted if not needed
  PI_pwd,   // Can be omitted if not needed
  PI_ID      // Client name that uniquely identify your device
);

void Blink()
{
  unsigned long previousMillis = millis();
  unsigned long currentMillis = previousMillis;

  digitalWrite(ledPin, LOW);
  while (currentMillis - previousMillis < 50)
  {
    currentMillis = millis();
  }
  digitalWrite(ledPin, HIGH);
}

void onConnectionEstablished()
{
  // Subscribe to "mytopic/test" and display received message to Serial
  client.subscribe("IoT/DaikinIR_CMD", [](const String & payload) {
    Serial.println(payload);
    Blink();

    if (payload == "AC_ON")
    {
      ACOn();
    }
    else if (payload == "AC_OFF")
    {
      ACOff();
    }
    else
    {
      client.publish("IoT/DaikinIR_MSG", "error:-1"); 
    }
  });

  digitalWrite(ledPin, HIGH); 
}

void ACOn()
{
  ac.on();
  ac.setFan(1);
  ac.setMode(kDaikinCool);
  ac.setTemp(25);
  ac.setSwingVertical(false);
  ac.setSwingHorizontal(false);

  // Set the current time to 1:33PM (13:33)
  // Time works in minutes past midnight
  //ac.setCurrentTime(13 * 60 + 33);
  // Turn off about 1 hour later at 2:30PM (14:30)
  //ac.enableOffTimer(14 * 60 + 30);

  ac.send();
  client.publish("IoT/DaikinIR_MSG", "Daikin AC ON");
  isTrigger = 1;
}

void ACOff()
{
  ac.off();
  client.publish("IoT/DaikinIR_MSG", "Daikin AC OFF");
  isTrigger = 0;

}

ICACHE_RAM_ATTR void buttonIRQ()
{
  Blink();

  if (isTrigger == 1)
  {
    ACOff();
  }
  else
  {
    ACOn();
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonIRQ, FALLING);

  digitalWrite(ledPin, LOW);
  ac.begin();


  // Optionnal functionnalities of EspMQTTClient : 
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
  
  Serial.println("Begin End.");
}

void loop() {
  client.loop();
}
