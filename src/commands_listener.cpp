#include "commands_listener.h"
#include <iostream>
#include <curl/curl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <thread>
#include <fmt/format.h>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <mpg123.h>
#include <out123.h>
extern std::map<std::string,std::vector<VideoData>> vect;


void Commands_listener::on_commands_create(const dpp::slashcommand_t& event) {
	dpp::user user = event.command.get_issuing_user();
	dpp::guild server = event.command.get_guild();
	auto user_vc = server.voice_members.find(user.id);
	auto bot_vc = event.from->get_voice(server.id);
	if (event.command.get_command_name() == "ping"){
		event.reply("Pong!");
	}
	else if(event.command.get_command_name() == "join"){
		//check if user in voice channel
		if( user_vc == server.voice_members.end()){
			event.reply(user.global_name + ", you're not in a voice channel" );
			return;
		}
		//check if bot already in this vc
		if(bot_vc){
			if(bot_vc->channel_id == user_vc->second.channel_id){
			event.reply("i'm already in this voice channel");
			return;
			}
			if(bot_vc->channel_id != user_vc->second.channel_id){
			event.from->disconnect_voice(server.id);
			}
		}
		server.connect_member_voice(user.id,0,1);
		event.reply(user.global_name + " asked to join into channel" );
    }
	else if(event.command.get_command_name() == "leave"){
		if(bot_vc){
			event.from->disconnect_voice(server.id);
			event.reply(user.global_name + " asked to leave " );
			}
		else{
			event.reply("i'm not connected to any voice channel");
		}
	}	
	else if(event.command.get_command_name() == "play"){
		std::string audioName = std::get<std::string>(event.get_parameter("audio"));
		std::vector<VideoData> result = findVideo(audioName);
		vect.erase(server.id.str());
		vect.insert(std::pair(server.id.str(),result));
		std::string msgOuptut = VideoData::stringFromVector(result);
		dpp::message msg(event.command.channel_id, msgOuptut);
		if(vect[server.id.str()].size()>0){
			dpp::component row;
			for(int i=0;i<vect[server.id.str()].size();i++){
				row.add_component(
	  	                  dpp::component()
		                        .set_label(std::to_string(i))
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


size_t Commands_listener::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::vector<VideoData> Commands_listener::findVideo(std::string NAMEVIDEO){
  std::string output = "output for " + NAMEVIDEO + "\n" ;
  std::string URL = "https://www.youtube.com/results?search_query=" + NAMEVIDEO;
  std::cout << URL << std::endl;
  struct curl_slist* headers = NULL;  
  CURL *curl;
  CURLcode res;
  std::string readBuffer;
  curl = curl_easy_init();
  curl = curl_easy_init();
  std::vector<VideoData> pushVector;
  if(curl) {
	curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
	std::string toFind = "ytInitialData = ";
	size_t pos = readBuffer.find(toFind);
	readBuffer.erase(0,pos + toFind.size());
	toFind = ";</script>";
	pos = readBuffer.find(toFind);
	readBuffer.erase(pos);
	json j = json::parse(readBuffer);
	for(int i = 0;i<5;i++){
	    VideoData video;
		video.id = std::string(j["contents"]["twoColumnSearchResultsRenderer"]["primaryContents"]["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]["contents"][i]["videoRenderer"]["videoId"]);
		video.title = std::string(j["contents"]["twoColumnSearchResultsRenderer"]["primaryContents"]["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]["contents"][i]["videoRenderer"]["title"]["runs"][0]["text"]);
		pushVector.push_back(video);
	}
  }
  return pushVector;
}
void Commands_listener::on_button_click(const dpp::button_click_t& event){
	std::string serverId = event.command.get_guild().id.str();
	int buttonId = std::stoi(event.custom_id);
	if(vect.find(serverId) == vect.end()){
		event.reply("Server currently not accounted");
		return ;
	}
	if(buttonId > vect[serverId].size()){
		event.reply("Value outdated");
		return;
	}
	event.reply("You choosed: " + vect[serverId][buttonId].title);
	//LINUX
	std::string path = "/tmp/discordBot";
	if(!std::filesystem::exists(path)){
		std::filesystem::create_directory(path);
	}
	std::string command = std::string("youtube-dl --extract-audio --audio-format mp3 ") + " -o '" + path + "/" + vect[serverId][buttonId].id + ".mp3'" + " 'https://www.youtube.com/watch?v=" + vect[serverId][buttonId].id+ "' &";
	system(command.c_str());
	std::thread thread(&Commands_listener::playAudio,event, path+"/" + vect[serverId][buttonId].id + ".mp3");
	thread.join();
}
void Commands_listener::playAudio(const dpp::button_click_t& event,std::string name)
{
	std::this_thread::sleep_for(std::chrono::seconds(12));
	std::vector<uint8_t> pcmdata;
	mpg123_init();
	int err;
	unsigned char* buffer;
	size_t buffer_size, done;
	int channels, encoding;
	long rate;
	mpg123_handle *mh = mpg123_new(NULL, &err);
	mpg123_param(mh, MPG123_FORCE_RATE, 48000, 48000.0);
	buffer_size = mpg123_outblock(mh);
	buffer = new unsigned char[buffer_size];
	buffer_size = mpg123_outblock(mh);
	buffer = new unsigned char[buffer_size];
	mpg123_open(mh, name.c_str());
	mpg123_getformat(mh, &rate, &channels, &encoding);
	unsigned int counter = 0;
	for (int totalBytes = 0; mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK; ) {
	    for (size_t i = 0; i < buffer_size; i++) {
	         pcmdata.push_back(buffer[i]);
	      }
	      counter += buffer_size;
	    totalBytes += done;
    }
    delete[] buffer;
    mpg123_close(mh);
    mpg123_delete(mh);
	event.from->get_voice(event.command.guild_id)->voiceclient->send_audio_raw((uint16_t*)pcmdata.data(), pcmdata.size());
}
