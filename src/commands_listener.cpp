#include "commands_listener.h"
#include <iostream>

#include "audioThread.h"


extern std::map<std::string,AudioThread> audioPerServer;


void Commands_listener::on_commands_create(const dpp::slashcommand_t& event) {
	dpp::user user = event.command.get_issuing_user();
	dpp::guild server = event.command.get_guild();
	std::string serverId = server.id.str();
	auto user_vc = server.voice_members.find(user.id);
	auto bot_vc = event.from->get_voice(server.id);
	if (event.command.get_command_name() == "ping"){
		event.reply("Pong!");
	}
	else if(event.command.get_command_name() == "join"){
		//check if user in voice channel
		
		int result = joinToVc(event);
		switch(result){
		case 0:
			audioPerServer.insert(std::pair(serverId,AudioThread()));
			audioPerServer[serverId].start(event);
			event.reply(user.global_name + " asked to join into channel" );
			break;
		case 1: event.reply("i'm already in this voice channel");break;
		case 2: event.reply(user.global_name + ", you're not in a voice channel" );break;
		case 3: event.reply("i'm already on another channel,leave first");break;
		}
		
		
    }
	
	else if(event.command.get_command_name() == "leave"){
		if(bot_vc){
			audioPerServer[serverId].setLeave(1);
			event.reply(user.global_name + " asked to leave " );
			}
		else{
			event.reply("i'm not connected to any voice channel");
		}
	}
	else if(event.command.get_command_name() == "skip"){
		if(checkIfThreadExists(serverId)){
			audioPerServer[serverId].setSkip(1);
		}
		event.reply("skipped");
	}
	else if(event.command.get_command_name() ==  "clear"){
		if(checkIfThreadExists(serverId)){
			audioPerServer[serverId].clearQueue();
		}
		event.reply("cleared");
	}
	else if(event.command.get_command_name() == "list"){
		std::string output;
		if(checkIfThreadExists(serverId)){
			output = audioPerServer[serverId].getListQueue();
		}
		if(output.empty()){
			event.reply("list is empty");
		}
		else{
			event.reply(output);
		}
	}	
	else if(event.command.get_command_name() == "play"){
		int res = joinToVc(event);
		if(res == 2){
			event.reply(user.global_name + ", you're not in a voice channel" );
		}
		if(res != 0 && res !=1){
			return;
		}
		if(res == 0){
			audioPerServer.insert(std::pair(serverId,AudioThread()));
			audioPerServer[serverId].start(event);
		}
		std::string audioName = std::get<std::string>(event.get_parameter("audio"));
		audioPerServer[serverId].findVideo(audioName);
		std::vector<VideoData> results = audioPerServer[serverId].getSearchResults();
		std::string msgOuptut = VideoData::stringFromVector(results);
		dpp::message msg(event.command.channel_id, msgOuptut);
		if(results.size()>0){
			dpp::component row;
			for(int i=0;i<results.size();i++){
				row.add_component(
	  	                  dpp::component()
		                        .set_label(std::to_string(i+1))
		                        .set_type(dpp::cot_button)
		                        .set_style(dpp::cos_primary)
		                        .set_id(std::to_string(i))
		        );
			}
			msg.add_component(row);
		}
		else{
			msg = dpp::message(event.command.channel_id,"Nothing was found");
		}
		
		event.reply(msg);
	}
		
}



void Commands_listener::on_button_click(const dpp::button_click_t& event){
	std::string serverId = event.command.get_guild().id.str();
	int buttonId = std::stoi(event.custom_id);
	if(!checkIfThreadExists(serverId)){
		event.reply("Server currently not accounted");
		return ;
	}
	if(buttonId > audioPerServer[serverId].getSearchResults().size()){
		event.reply("Value outdated");
		return;
	}
	VideoData selectedVideo = audioPerServer[serverId].getSearchResults()[buttonId];
	event.reply("You choosed: " + selectedVideo.title);
	audioPerServer[serverId].addSong(selectedVideo);

}
int Commands_listener::joinToVc(const dpp::slashcommand_t& event)
{
	dpp::user user = event.command.get_issuing_user();
	dpp::guild server = event.command.get_guild();
	auto user_vc = server.voice_members.find(user.id);
	auto bot_vc = event.from->get_voice(server.id);
	if( user_vc == server.voice_members.end()){
			return 2;
		}
		//check if bot already in this vc
		if(bot_vc){
			if(bot_vc->channel_id == user_vc->second.channel_id){
				return 1;
			}
			if(bot_vc->channel_id != user_vc->second.channel_id){
				return 3;
				//event.from->disconnect_voice(server.id);
			}
		}
		server.connect_member_voice(user.id,0,1);
		return 0;
	return -1;
}
bool Commands_listener::checkIfThreadExists(std::string serverId)
{
	if(audioPerServer.find(serverId)!=audioPerServer.end()){
		return 1;
	}
	else{
		return 0;
	}
}