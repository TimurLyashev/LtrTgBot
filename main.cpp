#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <vector>

#include <tgbot/tgbot.h>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

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

struct coordType{
    double lon;
    double lat;
};

struct cityType{
    int id;
    std::string name;
    std::string country;
    coordType coord;
};

struct userType{
    int32_t id;
    std::string city;
    std::string country;
    std::string lang;
    userType() : id(0), city("placeholder"), country("RU"), lang("ru") {}
};

auto findCity(std::vector<cityType> &cityVector, std::string cityNameFromMessage, std::string cityCountryFromMessage) {
  return std::find_if(cityVector.begin(), cityVector.end(), [&](const auto &gr) {
      return ((gr.name == cityNameFromMessage)&&(gr.country == cityCountryFromMessage));});
}

auto findUser(std::vector<userType> &userDB, int32_t userID) {
  return std::find_if(userDB.begin(), userDB.end(), [&](const auto &gr) {
      return gr.id == userID;});
}

int main() {
    std::ifstream botTokenFile("botTokenFile");
    std::string botToken;
    getline(botTokenFile, botToken);

    std::vector<userType> userDB;

    std::ifstream weatherTokenFile("weatherTokenFile");
    std::string weatherToken;
    getline(weatherTokenFile, weatherToken);


    std::ifstream cityListFile("city.list.json");
    json cityListJSON;
    cityListFile >> cityListJSON;
    std::vector<cityType> cityVector;
    for (auto cityFromJSON : cityListJSON){
        cityType cityVectorElem;
        cityVectorElem.id = cityFromJSON["id"];
        cityVectorElem.name = cityFromJSON["name"];
        cityVectorElem.country = cityFromJSON["country"];
        cityVectorElem.coord.lon = cityFromJSON["coord"]["lon"];
        cityVectorElem.coord.lat = cityFromJSON["coord"]["lat"];
        cityVector.push_back(cityVectorElem);
    }

    printf("Token: %s\n", botToken.c_str());

    TgBot::Bot bot(botToken);

    userType newUser;

    bot.getEvents().onCommand("start", [&bot, &userDB](TgBot::Message::Ptr message) {
        if (findUser(userDB,message->from->id)==userDB.end()) {
           bot.getApi().sendMessage(message->chat->id, "Привет!");
           bot.getApi().sendMessage(message->chat->id, "Введи имя города и страну. Пример: /setcity Moscow");
           bot.getApi().sendMessage(message->chat->id, "/setcountry RU");
        }
        else {
           auto user = findUser(userDB,message->from->id);
           if ((user->city == "placeholder")&&(user->country == "placeholder")){
               bot.getApi().sendMessage(message->chat->id, "Привет!");
               bot.getApi().sendMessage(message->chat->id, "Введи имя города и страну. Пример: /setcity Moscow");
               bot.getApi().sendMessage(message->chat->id, "/setcountry RU");
           } else if ((user->city == "placeholder")&&(user->country != "placeholder")) {
               bot.getApi().sendMessage(message->chat->id, "Привет!");
               bot.getApi().sendMessage(message->chat->id, "Введи имя города. Пример: /setcity Moscow");
               bot.getApi().sendMessage(message->chat->id, "Выбранная страна: " + user->country);
           } else if ((user->city != "placeholder")&&(user->country == "placeholder")){
               bot.getApi().sendMessage(message->chat->id, "Привет!");
               bot.getApi().sendMessage(message->chat->id, "Введи код страны. Пример: /setcountry RU");
               bot.getApi().sendMessage(message->chat->id, "Выбранный город: " + user->city);
           } else {
               bot.getApi().sendMessage(message->chat->id, "Привет!");
               bot.getApi().sendMessage(message->chat->id, "Выбранный город: " + user->city);
               bot.getApi().sendMessage(message->chat->id, "Выбранная страна: " + user->country);
           }
        }

    });

    bot.getEvents().onCommand("setcity", [&bot, &userDB, &newUser](TgBot::Message::Ptr message) {
        std::string setcity = message->text;
        if (setcity.size() <9) {
            bot.getApi().sendMessage(message->chat->id, "Пример: /setcity Moscow");
        } else {
            setcity = setcity.substr(9);

        auto user = findUser(userDB,message->from->id);

        if (findUser(userDB,message->from->id)==userDB.end()) {
                newUser.id = message->from->id;
                newUser.city = setcity;
                userDB.push_back(newUser);
                bot.getApi().sendMessage(message->chat->id, "Выбран город: " + setcity);
        } else {
            user->city = setcity;
            bot.getApi().sendMessage(message->chat->id, "Выбран город: " + setcity);
        }
        }
    });

    bot.getEvents().onCommand("setcountry", [&bot, &userDB, &newUser](TgBot::Message::Ptr message) {
        std::string setcountry = message->text;
        if (setcountry.size() < 12) {
            bot.getApi().sendMessage(message->chat->id, "Пример: /setcountry RU");
        } else {
            setcountry = setcountry.substr(12);
        auto user = findUser(userDB,message->from->id);

        if (findUser(userDB,message->from->id)==userDB.end()) {
                newUser.id = message->from->id;
                newUser.country = setcountry;
                userDB.push_back(newUser);
                bot.getApi().sendMessage(message->chat->id, "Выбрана страна: " + setcountry);
        } else {
            user->country = setcountry;
            bot.getApi().sendMessage(message->chat->id, "Выбрана страна: " + setcountry);
        }
        }
    });



    bot.getEvents().onNonCommandMessage([&bot, &weatherToken, &cityVector, &userDB](TgBot::Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }

        auto user = findUser(userDB,message->from->id);
        if (findUser(userDB,message->from->id)==userDB.end()) {
           bot.getApi().sendMessage(message->chat->id, "Введи имя города и страну. Пример: /setcity Moscow");
           bot.getApi().sendMessage(message->chat->id, "/setcountry RU");
        } else if (findCity(cityVector, user->city, user->country)==cityVector.end()){
            bot.getApi().sendMessage(message->chat->id, "Сочетание города " + user->city + " и страны " + user->country + " не найдено.");
            bot.getApi().sendMessage(message->chat->id, "Введи новое имя города и страну. Пример: /setcity Moscow");
            bot.getApi().sendMessage(message->chat->id, "/setcountry RU");
        } else {
        CURL *curl;
        curl = curl_easy_init();
        std::string weatherResponceStringBuffer;

        std::string weatherUrl = "https://api.openweathermap.org/data/2.5/find?q=" + user->city + "," + user->country + "&units=metric&lang=ru&appid="+weatherToken;
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

            if (message->from->username == "egorpugin"){
                bot.getApi().sendMessage(message->chat->id, "В МУРМАНСКЕ (" + user->city + ") " + description + ", НО НЕ ЗАБЫВАЙ НАДЕВАТЬ ВАЛЕНКИ И УШАНКУ");
                bot.getApi().sendMessage(message->chat->id, strtemp);

            } else {
                bot.getApi().sendMessage(message->chat->id, "Погода в " + user->city + " " + user->country);
                bot.getApi().sendMessage(message->chat->id, "Температура " +strtemp);
                bot.getApi().sendMessage(message->chat->id, description);


            }
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
