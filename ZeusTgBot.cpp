/*
 *ZeusTgBot.cpp
 *     Author: Mazarei
 *     Edited by: digi boy (sina) 2020 April 26
 *     www.sisoog.com
 */
#include "ZeusTgBot.h"

Zeus_TgBot::Zeus_TgBot(String token, bool IsBale, bool debug)
{
	botkey = "bot" + token;
	UpdateID = 0;
	Func_Message_Event = NULL;
	is_debug = debug;
	IsProxy = false;
	if(IsBale) zHost = BALE_HOST;
	else zHost = TG_HOST;
}

void Zeus_TgBot::SetProxyHost(String pHost)
{
	zHost = pHost;
	IsProxy = true;
}


Zeus_TgBot::~Zeus_TgBot() {}

void Zeus_TgBot::Set_Message_Event(Message_Event Event_Call)
{
	Func_Message_Event = Event_Call;
}

String Zeus_TgBot::SendCommand(String command, JsonObject &payload)
{
	static String lUrl = "";
	String ReqUrl = zHost;


	if (IsProxy == true) /*make Proxy URL*/
	{
		ReqUrl = zHost + "?bot=" + botkey + "&api=" + command;
	}
	else
	{
		ReqUrl = zHost + "/" + botkey + "/" + command;
	}

	std::unique_ptr<BearSSL::WiFiClientSecure > client(new BearSSL::WiFiClientSecure);
	String response = "";

	//client->setFingerprint(fingerprint);
	client->setInsecure();

	HTTPClient https;

	if (is_debug) Serial.print("[HTTPS] begin...\n");
	if (https.begin(*client, ReqUrl))
	{
		// HTTPS

		//POSTING DATA
		String postData;
		payload.printTo(postData);
		https.addHeader("Content-Type", "application/json");	//Specify content-type header

		if (is_debug)
		{
			Serial.print("Send:");
			Serial.println(postData);
		}

		int httpCode = https.POST(postData); /*post data to server*/

		// httpCode will be negative on error
		if (httpCode > 0)
		{
			// HTTP header has been send and Server response header has been handled
			if (is_debug) Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

			// file found at server
			if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
			{
				response = https.getString();
			}
		}
		else
		{
			if (is_debug) Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
		}

		https.end();
	}
	else
	{
		if (is_debug) Serial.printf("[HTTPS] Unable to connect\n");
	}
	return response;
}

bool Zeus_TgBot::GetMe(Zeus_TgBot::User_t *User)
{
	DynamicJsonBuffer jsonBuffer;
	JsonObject &payload = jsonBuffer.createObject();
	String Command = "getme";
	String ServerResult = SendCommand(Command, payload);
	JsonObject &root = jsonBuffer.parseObject(ServerResult);

	if (root.success())
	{
		if (root.containsKey("result"))
		{
			String first_name = root["result"]["first_name"];;
			String last_name = root["result"]["last_name"];
			String username = root["result"]["username"];

			User->first_name = first_name;
			User->last_name = last_name;
			User->username = username;

			return true;
		}
	}
	return false;
}

void Zeus_TgBot::ProssessOneMessage(JsonObject & Message)
{
	String Message_Text = Message["message"]["text"];
	String Message_From = Message["message"]["from"]["id"];
	String Message_ID = Message["message"]["message_id"];

	if (is_debug)
	{
		Serial.println("Receive Message: ");
		Serial.println("Message ID " + Message_ID);
		Serial.println("Message From " + Message_From);
		Serial.println("Message Text " + Message_Text);
	}

	if (Func_Message_Event != NULL)
		Func_Message_Event(Message_ID, Message_From, Message_Text, false, "");
}

void Zeus_TgBot::ProssessOneCallBack(JsonObject & Message)
{
	//Message.printTo(Serial);

	String CallBack_ID = Message["callback_query"]["id"];
	String Message_Text = Message["callback_query"]["data"];
	String Message_From = Message["callback_query"]["from"]["id"];
	String Message_ID = Message["callback_query"]["message"]["message_id"];

	if (is_debug)
	{
		Serial.println("Receive CallBack: ");
		Serial.println("CallBack ID " + CallBack_ID);
		Serial.println("Message ID " + Message_ID);
		Serial.println("Message From " + Message_From);
		Serial.println("CallBack Data " + Message_Text);
	}

	if (Func_Message_Event != NULL)
		Func_Message_Event(Message_ID, Message_From, Message_Text, true, CallBack_ID);

}

bool Zeus_TgBot::GetUpdates()
{
	DynamicJsonBuffer jsonBuffer;
	JsonObject &payload = jsonBuffer.createObject();

	String Command = "getupdates";

	payload["offset"] = String(UpdateID);
	payload["limit"] = String(LIMIT_Message);

	String ServerResult = SendCommand(Command, payload);

	if (ServerResult != "")
	{
		if (is_debug)
		{
			Serial.print("incoming message length ");
			Serial.println(ServerResult.length());
		}

		// Parse response into Json object
		DynamicJsonBuffer jsonBuffer;
		JsonObject &root = jsonBuffer.parseObject(ServerResult);

		if (root.success())
		{
			if (is_debug) Serial.println();
			if (root.containsKey("result"))
			{
				int Message_Number = root["result"].size();
				if (Message_Number > 0)
				{
					for (int MeaageID = 0; MeaageID < Message_Number; MeaageID++)
					{
						int update_id = root["result"][MeaageID]["update_id"];
						JsonObject &Message = root["result"][MeaageID];
						UpdateID = update_id + 1;

						if (Message.containsKey("message"))
						{
							ProssessOneMessage(Message);
						}

						if (Message.containsKey("callback_query"))
						{
							ProssessOneCallBack(Message);
						}
					}
				}
				else
				{
				 		//if (is_debug) Serial.println("no new messages");
				}
				//if (is_debug) Serial.println();
				return true;
			}
			else
			{
				if (is_debug) Serial.println("Response contained no 'result'");
			}
		}
	}
	//if (is_debug) Serial.println();
	return false;
}

bool Zeus_TgBot::answerCallbackQuery(String callback_query_id, String Text)
{
	String command = "answerCallbackQuery";
	DynamicJsonBuffer jsonBuffer;
	JsonObject &payload = jsonBuffer.createObject();

	payload["callback_query_id"] = callback_query_id;
	payload["text"] = Text;

	String Resp = SendCommand(command, payload);
	if (is_debug) Serial.println(Resp);
	return true;
}

bool Zeus_TgBot::sendMessage(String chat_id, String text)
{
	String command = "sendMessage";

	DynamicJsonBuffer jsonBuffer;
	JsonObject &payload = jsonBuffer.createObject();

	payload["chat_id"] = chat_id;
	payload["text"] = text;

	String Resp = SendCommand(command, payload);
	if (is_debug) Serial.println(Resp);

	return true;
}

bool Zeus_TgBot::sendMessage(String chat_id, String text, String reply_markup)
{
	String command = "sendMessage";
	DynamicJsonBuffer jsonBuffer;
	JsonObject &payload = jsonBuffer.createObject();

	payload["chat_id"] = chat_id;
	payload["text"] = text;
	JsonObject &replyMarkup = payload.createNestedObject("reply_markup");

	DynamicJsonBuffer keyboardBuffer;
	replyMarkup["inline_keyboard"] = keyboardBuffer.parseArray(reply_markup);

	String response = SendCommand(command, payload);
	JsonObject &root = jsonBuffer.parseObject(response);
	if (root.success())
	{
		return true;
	}
	return false;
}

bool Zeus_TgBot::EditMessage(String msg_id, String chat_id, String text, String reply_markup)
{
	String command = "editMessageText";
	DynamicJsonBuffer jsonBuffer;
	JsonObject &payload = jsonBuffer.createObject();

	payload["chat_id"] = chat_id;
	payload["message_id"] = msg_id;
	payload["text"] = text;

	JsonObject &replyMarkup = payload.createNestedObject("reply_markup");

	DynamicJsonBuffer keyboardBuffer;
	replyMarkup["inline_keyboard"] = keyboardBuffer.parseArray(reply_markup);

	String response = SendCommand(command, payload);

	JsonObject &root = jsonBuffer.parseObject(response);
	if (root.success())
	{
		return true;
	}
	return false;
}