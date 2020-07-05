#include <DYIRDaikinSend.h>
#include <DYIRDaikin.h>
#include <DYIRDaikinBRC.h>
#include <DYIRDaikinPWM.h>
#include <boarddefs.h>
#include <DYIRDaikinDef.h>
#include <DYIRDaikinRecv.h>

#include "EspMQTTClient.h"

#include "private_info.h"

const int ledPin =  LED_BUILTIN;// the number of the LED pin
const int buttonPin = 13; //D7
const int irPin = 5; //D1

DYIRDaikin irdaikin;
int isTrigger = 0;

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
  irdaikin.on();
  irdaikin.setSwing_off();
  irdaikin.setMode(1);
  irdaikin.setFan(4);//FAN speed to MAX
  irdaikin.setTemp(25);
  //----everything is ok and to execute send command-----
  irdaikin.sendCommand();
  client.publish("IoT/DaikinIR_MSG", "Daikin AC ON");
  isTrigger = 1;
}

void ACOff()
{
  irdaikin.off();
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
  pinMode(irPin, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonIRQ, FALLING);

  digitalWrite(ledPin, LOW);
  irdaikin.begin(irPin);

  // Optionnal functionnalities of EspMQTTClient : 
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
  
  Serial.println("Begin End.");
}

void loop() {
  client.loop();
}
