#include <dpp/dpp.h>
#include <getopt.h>
#include "commands_listener.h"
#include "audioThread.h"
#include <fstream>
#include <vector>
extern const std::string path;
#ifdef _WIN32
    const std::string path = "tmp";
	const std::string tokenPath = "";
#elif __linux__
	#include<stdlib.h>
    const std::string path = "/tmp/discordBot";
	const std::string tokenPath = std::string(getenv("HOME")) + "/.config/ytdsbot/";
	
#endif

std::string getToken(){
	if(!std::filesystem::exists(tokenPath + "BOT_TOKEN")){
		if(!std::filesystem::exists(tokenPath)){
			std::filesystem::create_directory(tokenPath);
		}
		std::cout << "file BOT_TOKEN was not found , please place one inside " + tokenPath << std::endl;
		std::cout << "Press enter to exit, or enter bot key here and continue" << std::endl;
		std::string key;
		std::getline(std::cin,key);
		if(key.empty()){
			exit(0);
		}
		else{
			std::fstream file(tokenPath+"BOT_TOKEN",std::fstream::trunc | std::fstream::in | std::fstream::out);
			file << key;
			file.close();

		}
		
	}
	std::fstream file(tokenPath + "BOT_TOKEN");
	std::string line;
	std::getline(file,line);
	return line;
}
const std::string BOT_TOKEN = getToken();

//Global declaration of a bot vairable
extern dpp::cluster bot;
dpp::cluster bot(BOT_TOKEN, dpp::i_default_intents | dpp::i_message_content);

//variable with stores audioThreads for every server
extern std::map<std::string,AudioThread> audioPerServer;
std::map<std::string,AudioThread> audioPerServer;


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
	
	//event loop thats check if some Audiothread leaved voice channel and no longer needed
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
		//if -s variable passed we send discord all commands declaration
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

