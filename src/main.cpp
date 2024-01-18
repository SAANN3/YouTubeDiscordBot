#include <dpp/dpp.h>
#include <getopt.h>
#include "commands_listener.h"
#include "audioThread.h"
#include <fstream>
#include <vector>

std::string getToken(){
	std::fstream file("BOT_TOKEN");
	std::string line;
	std::getline(file,line);
	return line;
}

const std::string BOT_TOKEN = getToken();
extern dpp::cluster bot;
extern const std::string path;
//variable with stores audioThreads for every server
extern std::map<std::string,AudioThread> audioPerServer;
dpp::cluster bot(BOT_TOKEN, dpp::i_default_intents | dpp::i_message_content);
std::map<std::string,AudioThread> audioPerServer;
#ifdef _WIN32
    const std::string path = "tmp";
#elif __linux__
    const std::string path = "/tmp/discordBot";
#endif


int main(int argc, char *argv[]) {
	
	int opt;
	int setup_commands = 0;
	//registering commands only if we recive -s argument on start
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
	
	//creating a tmp folder if it doesn't exists
	if(!std::filesystem::exists(path)){
		std::filesystem::create_directory(path);
	}
	if(!std::filesystem::exists("BOT_TOKEN")){
		std::cout << "file BOT_TOKEN was not found" << std::endl;
		return -1;
	}
	//event loop thats check if some Audiothread leaved and no longer needed
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
	bot.start(dpp::st_wait);
}

