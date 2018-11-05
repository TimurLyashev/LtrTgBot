#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>

#include <tgbot/tgbot.h>

using namespace std;
using namespace TgBot;

int main() {
    ifstream tokenfile("tokenfile");
    string token;
    getline(tokenfile, token);

    printf("Token: %s\n", token.c_str());

    Bot bot(token);
    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi!");
    });
    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }

        bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
        bot.getApi().sendMessage(message->chat->id, "Your first name is: " + message->from->firstName);
        bot.getApi().sendMessage(message->chat->id, "Your last name is: " + message->from->lastName);
        bot.getApi().sendMessage(message->chat->id, "Your user name is: " + message->from->username);
        bot.getApi().sendMessage(message->chat->id, "Your id is: " + to_string(message->from->id));
    });

    signal(SIGINT, [](int s) {
        printf("SIGINT got\n");
        exit(0);
    });

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook();

        TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (exception& e) {
        printf("error: %s\n", e.what());
    }

    return 0;
}
