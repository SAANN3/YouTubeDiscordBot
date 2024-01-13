#ifndef COMMANDS_LISTENER
#define COMMANDS_LISTENER

#include <dpp/dpp.h>
#include "videoData.h"

class Commands_listener{
    public:
        static void on_commands_create(const dpp::slashcommand_t& event);
        static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
        static std::vector<VideoData> findVideo(std::string);
        static void on_button_click(const dpp::button_click_t& event);
        static void playAudio(const dpp::button_click_t& event,std::string name);
    private:
        

};

#endif


