/*
* WebSocketClient.ino
*
*  Created on: 24.05.2015
*
*/

#include <Arduino.h>
#include <cstdlib>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <stdlib.h>
#include <WebSocketsClient.h>

#include <Hash.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
String receive_id, state;
String payloadStr;

/*  Configuration  */

const char* ssid = "TP-LINK_A1A5A5";
const char* password = "28387756";

const char* host = "192.168.137.1";
const uint  port = 80;

const char* SwitchId = "2";


const int led = 2;
const int _switch = 13;
const int button = 14;
const int greenLed = 4;

void SwitchActions(bool state)
{
	digitalWrite(led, !state);
	digitalWrite(_switch, state);
	greenLedBlink();
}

void setupPins()
{
	pinMode(led, OUTPUT);
	pinMode(_switch, OUTPUT);
	pinMode(button, INPUT);
	pinMode(greenLed, OUTPUT);

	digitalWrite(_switch, 1);
	digitalWrite(led, 1);
	analogWrite(greenLed, 120);
}

void greenLedBlink(void)
{
	digitalWrite(greenLed, 0);

	for (int i = 1; i <= 3; i++)
	{
		analogWrite(greenLed, (i % 2) * 120);
		delay(30);
	}
}
/* ----------------- */

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length)
{

	switch (type)
	{
	case WStype_DISCONNECTED:
		Serial.print("[WSc] Disconnected!\n");
		break;
	case WStype_CONNECTED:
		Serial.print("[WSc] Connected to url : ");
		Serial.print((char*)payload);
		webSocket.sendTXT("Connected");
		break;
	case WStype_TEXT:
		Serial.print("[WSc] get text: \n");
		Serial.print((char*)payload);
		Serial.print("\n");
		payloadStr = (char*)payload;

		for (int i = 0; i < payloadStr.length(); i++)
		{
			if (payloadStr[i] == ':')
			{
				receive_id = payloadStr.substring(0, i);
				state = payloadStr.substring(i + 1, i + 3);
				break;
			}
		}
		Serial.print("[Wsc] payloadStr: " + payloadStr + "\n");

		Serial.print("[Wsc] Parsed state: " + state + "\n");
		Serial.print("[Wsc] Parsed SwitchId " + receive_id + "\n");


		if (receive_id == SwitchId)
		{
			if (state[0] == 'O' && state[1] == 'N')
			{
				Serial.print("[Wsc] payloadState == ON");
				SwitchActions(1);

			}
			else
			{
				Serial.print("[Wsc] payloadState == OFF");
				SwitchActions(0);
			}
			// send message to server
			//webSocket.sendTXT("message here");
		}

		break;

	case WStype_BIN:
		//USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
		Serial.print("[WSc] get binary length: ");
		Serial.print(length);
		Serial.print("\n");

		hexdump(payload, length);

		// send data to server
		webSocket.sendBIN(payload, length);
		break;
	}
}

void setup() {

	Serial.begin(115200);

	setupPins();

	for (uint8_t t = 4; t > 0; t--) {
		Serial.print("[SETUP] BOOT WAIT");
		Serial.print(t);
		Serial.print("\n");
		delay(1000);
	}

	WiFiMulti.addAP(ssid, password);

	while (WiFiMulti.run() != WL_CONNECTED) {
		for (int i = 1; i <= 7; i++)
		{
			digitalWrite(led, i % 2); //light when 0
			delay(80);
		}
		delay(300);
	}
	Serial.print("Trying connect to server.\n");

	webSocket.begin(host, port, "/notifications");

	// event handler
	webSocket.onEvent(webSocketEvent);

	// HTTP Basic Authorization
	//webSocket.setAuthorization("Admin", "Admin");

	Serial.print("Zakonczono!!\n");
	webSocket.setReconnectInterval(5000);
}

void putStatus(String switchId, String state)
{
	WiFiClient client;

	if (client.connect(host, port))
	{
		Serial.print("postStatus: wifi connected! \n");

		WiFiClient client;

		Serial.printf("\n[Connecting to %s ... ", host);
		if (client.connect(host, port))
		{
			Serial.println("connected]");

			Serial.println("[Sending a request]");

			client.println(String("PUT /api/Switches/") + String(switchId) + String("/") + String(state) + String(" HTTP/1.1"));
				client.println(String("Host: ") + String(host) + String(":") + String(port));
				client.println("Connection: close");
				client.println("Content-Type: application/json");
				client.println("Content-Length: 0");
				client.println();


			Serial.println("[Response:]");
			while (client.connected())
			{
				if (client.available())
				{
					String line = client.readStringUntil('\n');
					Serial.println(line);
				}
			}
			client.stop();
			Serial.println("\n[Disconnected]");
		}
		else
		{
			Serial.println("connection failed!]");
			client.stop();
		}
	}
}

int _button;
int toggle = 1;

void loop() {
	webSocket.loop();

	_button = digitalRead(button);
	delay(50);

	if (digitalRead(button) == 1 && _button == 0) {

		switch (toggle)
		{
			case 1:
			SwitchActions(1);
			Serial.print("Butto, ON");
			putStatus(SwitchId, "ON");
			toggle = 2;
			break;

			case 2:
			SwitchActions(0);
			Serial.print("Button2 OFF");
			putStatus(SwitchId, "OFF");
			toggle = 1;
			break;
		}
	}
}
