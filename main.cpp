#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <curl/curl.h>

#include <tgbot/tgbot.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s)
{
    size_t newLength = size*nmemb;
    size_t oldLength = s->size();
    try
    {
        s->resize(oldLength + newLength);
    }
    catch(std::bad_alloc &e)
    {
        //handle memory problem
        return 0;
    }

    std::copy((char*)contents,(char*)contents+newLength,s->begin()+oldLength);
    return size*nmemb;
}

int main() {
    std::ifstream botTokenFile("botTokenFile");
    std::string botToken;
    getline(botTokenFile, botToken);

    std::ifstream weatherTokenFile("weatherTokenFile");
    std::string weatherToken;
    getline(weatherTokenFile, weatherToken);


//    std::ifstream cityListFile("city.list.json");
//    json cityListJSON;
//    cityListFile >> cityListJSON;



    printf("Token: %s\n", botToken.c_str());

    TgBot::Bot bot(botToken);
    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi!");
    });
    bot.getEvents().onAnyMessage([&bot, &weatherToken](TgBot::Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }
        CURL *curl;
        curl = curl_easy_init();
        std::string weatherResponceStringBuffer;
        std::string cityFromMessage = "Murom";
        std::string weatherUrl = "https://api.openweathermap.org/data/2.5/find?q=" + cityFromMessage + ",ru&units=metric&lang=ru&appid="+weatherToken;
        CURLcode res;
        if(curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL, weatherUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &weatherResponceStringBuffer);

            res = curl_easy_perform(curl);
                    /* Check for errors */
                    if(res != CURLE_OK)
                    {
                        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                                curl_easy_strerror(res));
                    }

                    /* always cleanup */
            curl_easy_cleanup(curl);

        }

        json weatherResponceJson = nlohmann::json::parse(weatherResponceStringBuffer);

        float ftemp = weatherResponceJson["list"][0]["main"]["temp"];
        std::string strtemp = std::to_string(ftemp);
        std::string description = weatherResponceJson["list"][0]["weather"][0]["description"];

        if ((message->text == "Murom")||message->text == "Moscow") cityFromMessage = message->text;
        else cityFromMessage = "Murom";
        if (message->from->username == "egorpugin"){
            bot.getApi().sendMessage(message->chat->id, "В МУРМАНСКЕ " + description + ", НО НЕ ЗАБЫВАЙ НАДЕВАТЬ ВАЛЕНКИ И УШАНКУ");
            bot.getApi().sendMessage(message->chat->id, strtemp);

        } else {
            bot.getApi().sendMessage(message->chat->id, "Погода в " + cityFromMessage);
            bot.getApi().sendMessage(message->chat->id, "Температура " +strtemp);
            bot.getApi().sendMessage(message->chat->id, description);


        }
    });

    signal(SIGINT, [](int s) {
        printf("SIGINT got\n");
        exit(0);
    });

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook();

        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (std::exception& e) {
        printf("error: %s\n", e.what());
    }

    return 0;
}
