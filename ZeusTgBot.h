/*
 * ZeusTgBot.h
 *     Author: Mazarei
 *     Edited by: digi boy (sina) 2020 April 26
 *     www.sisoog.com
 */

#ifndef ZEUSTGBOT_H_
#define ZEUSTGBOT_H_
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include "ArduinoJson.h"
#include <WiFiClientSecureBearSSL.h>
// Fingerprint for demo URL, expires on June 2, 2019, needs to be updated well before this date
const uint8_t fingerprint[20] = {0x5A, 0xCF, 0xFE, 0xF0, 0xF1, 0xA6, 0xF4, 0x5F, 0xD2, 0x11, 0x11, 0xC6, 0x1D, 0x2F, 0x0E, 0xBC, 0x39, 0x8D, 0x50, 0xE0};


#define TG_HOST 		"https://api.telegram.org"
#define BALE_HOST 		"https://tapi.bale.ai"
#define LIMIT_Message	1

typedef void (*Message_Event) (String Message_id,String Message_From,String Message_Text,bool is_callback,String Call_ID);

class Zeus_TgBot {
public:

	typedef struct
	{
		uint32_t	id;
		String		first_name;
		String		last_name;
		String		username;
	}User_t;

	Zeus_TgBot(String token, bool IsBale=false, bool debug=false);
	virtual ~Zeus_TgBot();

	void Set_Message_Event(Message_Event Event_Call);
	bool GetMe(User_t *User);
	bool GetUpdates();
	bool sendMessage(String chat_id, String text);
	bool sendMessage(String chat_id, String text, String reply_markup);
	bool answerCallbackQuery(String callback_query_id,String Text);
	bool EditMessage(String msg_id,String chat_id, String text, String reply_markup);

	void SetProxyHost(String pHost);
	
private:
	String botkey;

	volatile uint32_t	UpdateID;

	String SendCommand(String command, JsonObject& payload);
	void ProssessOneMessage(JsonObject& Message);
	void ProssessOneCallBack(JsonObject& Message);

	Message_Event Func_Message_Event;
	bool is_debug = false;
	String   zHost;
	bool     IsProxy;
};

#endif /* ZEUSTGBOT_H_ */
