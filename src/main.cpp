#include <dpp/dpp.h>
#include <getopt.h>
#include "commands_listener.h"
#include "audioThread.h"
#include <fstream>
#include <vector>


const std::string BOT_TOKEN = "MTE4OTMxMzc0MTQ4NjAzNTAyNA.GZDYvb.fXLYovf7yql78Omsm7xdFB9xYeYS5AcuT644Ug";
extern dpp::cluster bot;
extern std::map<std::string,AudioThread> audioPerServer;
dpp::cluster bot(BOT_TOKEN, dpp::i_default_intents | dpp::i_message_content);
std::map<std::string,AudioThread> audioPerServer;


int main(int argc, char *argv[]) {
	std::thread eventLoop([](){
		while(1){
			std::vector<std::string> it;
			for(auto i = audioPerServer.begin();i != audioPerServer.end();i++){
				
				if(i->second.killNeeded){
					it.push_back(i->first);
				}
			}
			if(it.size()>0){
				for(int i = 0;i<it.size();i++){
					audioPerServer[it[i]].joinThread();
					audioPerServer.erase(it[i]);
				}
				it.clear();
			}
 			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	});
	int opt;
	int setup_commands = 0;
	while ((opt = getopt(argc, argv, ":s")) !=-1){
		switch (opt){
			case 's': 
				setup_commands = 1; 
				std::cout << "commands registered" << std::endl;
				break;
			case '?':
      			printf("Missing arg for %c\n", optopt);
      			break;
		}
	}
	
	bot.on_log(dpp::utility::cout_logger());
	if(setup_commands){
		bot.on_ready([](const dpp::ready_t& event) {
		if (dpp::run_once<struct register_bot_commands>()) {
			dpp::slashcommand playCommand("play","Plays a music",bot.me.id);
			playCommand.add_option(dpp::command_option(dpp::co_string,"audio","name of song",true));
			bot.global_command_create(playCommand);
			bot.global_command_create(dpp::slashcommand("ping", "Ping pong!", bot.me.id));
			
			bot.global_command_create(dpp::slashcommand("join", "join!", bot.me.id));
			bot.global_command_create(dpp::slashcommand("leave", "bye!", bot.me.id));
			bot.global_command_create(dpp::slashcommand("skip","skip current song",bot.me.id));
			bot.global_command_create(dpp::slashcommand("clear","clear queue",bot.me.id));
			bot.global_command_create(dpp::slashcommand("list","prints queue",bot.me.id));
			
		}
		});
	}
	bot.on_slashcommand(&Commands_listener::on_commands_create);
	bot.on_button_click(&Commands_listener::on_button_click);
	
	bot.on_message_create([](const dpp::message_create_t& event) {
		
		if (event.msg.content.find("dd") != std::string::npos) {
			event.reply("kek", true);
		}
		});
	bot.start(dpp::st_wait);
}

