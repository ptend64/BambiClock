/*******************************************************************
    A telegram bot for your ESP8266 that controls the 
    onboard LED. The LED in this example is active low.

    Parts:
    D1 Mini ESP8266 * - http://s.click.aliexpress.com/e/uzFUnIe
    (or any ESP8266 board)

      = Affilate

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/


    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <TimeLib.h>

// Wifi network station credentials
#define WIFI_SSID "FASTWEB-TENDOTT"
#define WIFI_PASSWORD "YOUR_PASSWORD"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "5930957527:AAEXaKBhSy4vD--fSEbiFvA3XC5PeKMnreY"
#define RELE 0

#define BENVENUTO "/welcome"
#define CARICA    "/carica"
#define FERMA     "/stop"
#define STATO     "/stato"

#define LAMPADA "\xF0\x9F\x92\xA1"
#define TORCIA "\xF0\x9F\x94\xA6"
#define PENDOLA "\xF0\x9F\x95\xB0"
#define CIAO "\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBC"
#define BOH "\xE2\x9D\x93"
#define MERDA "\xF0\x9F\x92\xA9"
#define DIVIETO "\xE2\x9B\x94\xEF\xB8\x8F"

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done
time_t nowt;

const int ledPin = LED_BUILTIN;
int ledStatus = LOW;

void handleNewMessages(int numNewMessages)
{
  //Serial.print("handleNewMessages ");
  //Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "Pupi64")
    {
      if (text == CARICA)
      {
        if(ledStatus == LOW)
          bot.sendMessage(chat_id, "La carica è già attiva dalle " + String(hour(nowt)) + ":" + String(minute(nowt)), "");
        else
        {
          digitalWrite(ledPin, LOW); // turn the LED on (HIGH is the voltage level)
          digitalWrite(RELE, HIGH);         // Turn the RELE off
          ledStatus = LOW;
          bot.sendMessage(chat_id, "La carica è partita alle " + String(hour(nowt)) + ":" + String(minute(nowt)), "");
        }
      } else if (text == FERMA)
      {
        if(ledStatus == HIGH)
          bot.sendMessage(chat_id, "La carica è già ferma");
        else
        {
          ledStatus = HIGH;
          digitalWrite(ledPin, HIGH); // turn the LED off (LOW is the voltage level)
          digitalWrite(RELE, LOW);         // Turn the RELE off
          bot.sendMessage(chat_id, "Ho fermato la carica alle " + String(hour(nowt)) + ":" + String(minute(nowt)), "");
        }
      } else if (text == STATO)
      {
        if (ledStatus == LOW)
        {
          bot.sendMessage(chat_id, LAMPADA, "");
          bot.sendMessage(chat_id, "L'orologio si sta caricando da " , "");
        }
        else
        {
          bot.sendMessage(chat_id, TORCIA, "");
          bot.sendMessage(chat_id, "L'orologio non è in carica attualmente.", "");
        }
      } else if (text == BENVENUTO)
      {
        String welcome = "Ciao, " + from_name + " sei connesso al servizio BambiClock.\n";
        welcome += "Questi sono i comandi che puoi inviare:\n\n";
        welcome += "/carica : Per iniziare la carica della molla.\n";
        welcome += "/stop : per interrompere la carica.\n";
        welcome += "/stato : Restituisce lo stato del dispositivo.\n";
        welcome += "\xF0\x9F\x98\x80";
        bot.sendMessage(chat_id, CIAO, "Markdown");
        bot.sendMessage(chat_id, welcome, "Markdown");
      } else
      {
        bot.sendMessage(chat_id, String(MERDA) + " comando non riconosciuto\n\"" + text + "\"", "");
        char buf[20];
        text.toCharArray(buf, 20);
        for(i = 0; i < text.length(); i++)
          Serial.print(buf[i], HEX);
      }
    }
    else
    {
      bot.sendMessage(chat_id, DIVIETO, "");
      bot.sendMessage(chat_id, "Spiacente " + from_name + " non sei autorizzato.", "");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output
  pinMode(RELE, OUTPUT);        // Initialize the RELE pin as an output
  digitalWrite(LED_BUILTIN, ledStatus); // Turn the LED on (Note that LOW is the voltage level
  digitalWrite(RELE, LOW);      // Turn the RELE off

  delay(10);

  // attempt to connect to Wifi network:
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, "Gabibbo1964");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  ledStatus = HIGH;
  digitalWrite(LED_BUILTIN, ledStatus); // Turn the LED off (Note that LOW is the voltage level

  // Check NTP/Time, usually it is instantaneous and you can delete the code below.
  Serial.print("Retrieving time: ");
  nowt = time(nullptr);
  while (nowt < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    nowt = time(nullptr);
  }

  int h = hour(nowt);
  Serial.print(h);
  Serial.print(minute());
  bot_setup();
}

void bot_setup()
{
  const String commands = F("["
                            "{\"command\":\"welcome\", \"description\":\"Mostra i comandi disponibili.\"},"
                            "{\"command\":\"carica\",  \"description\":\"Fa partire la carica dell'orologio.\"},"
                            "{\"command\":\"stop\",  \"description\":\"Interrompe la carica dell'orologio.\"},"
                            "{\"command\":\"stato\",\"description\":\"Restituisce lo stato del sistema.\"},"
                            "{\"command\":\"jj \xE2\x9D\x93 \",\"description\":\"boh.\"}"// no comma on last command
                            "]");
  bot.setMyCommands(commands);
  bot.sendMessage("88885550", "BambiClock attivo", "Markdown");
}
void loop()
{
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      //Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
    nowt = time(nullptr);
  }
}
