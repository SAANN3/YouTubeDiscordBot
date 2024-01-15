#ifndef COMMANDS_LISTENER
#define COMMANDS_LISTENER

#include <dpp/dpp.h>
#include "videoData.h"

class Commands_listener{
    public:
        static void on_commands_create(const dpp::slashcommand_t& event);
        static void on_button_click(const dpp::button_click_t& event);
        static int joinToVc(const dpp::slashcommand_t& event);
        static bool checkIfThreadExists(std::string serverId);
    private:
        

};

#endif


