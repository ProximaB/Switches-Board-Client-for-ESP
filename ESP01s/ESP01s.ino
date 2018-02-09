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

const char* SwitchId = "1";


const int led = 2;

void SwitchActions(bool state)
{
	digitalWrite(led, !state);
}

void setupPins()
{
	pinMode(led, OUTPUT);

	digitalWrite(led, 1);
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
	webSocket.setAuthorization("Admin", "Admin");

	Serial.print("Zakonczono!!\n");
	webSocket.setReconnectInterval(5000);
}

void loop() {
	webSocket.loop();
}
