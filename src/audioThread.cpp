#include "audioThread.h"
#include <fmt/format.h>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <mpg123.h>
#include <out123.h>
#include <curl/curl.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef _WIN32
#include <io.h>
#include <nlohmann/json.hpp>
#elif __linux__
#include <unistd.h>
#endif

extern const std::string path;

extern std::map<std::string,AudioThread> audioPerServer;

void AudioThread::start(const dpp::slashcommand_t& event)
{	
    
    thread = std::thread(&AudioThread::run,this,event);
}

AudioThread::AudioThread()
{

}
void AudioThread::join()
{
    thread.join();
}
std::string AudioThread::getListQueue()
{
	return VideoData::stringFromVector(queue);
}
std::vector<VideoData> AudioThread::getSearchResults()
{
	return searchResults;
}
std::vector<uint8_t> AudioThread::prepareAudio(std::string name)
{
	//convert mp3 file to opus data and send it to discord
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
		return pcmdata;
}
void AudioThread::joinThread()
{
	thread.join();
}


void AudioThread::run(const dpp::slashcommand_t& event)
{	
	std::string serverId = event.command.get_guild().id.str();
	
	
	while(true){
		
		auto bot_vc = event.from->get_voice(serverId);
		while(queue.size()==0 || !bot_vc){
			auto bot_vc = event.from->get_voice(serverId);
			//bot lives until it gets disconnected so if queue is empty we wait for it to be filled
			if(leave || !bot_vc){
				leave = 0;
				killNeeded = 1;
				return;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
		dpp::discord_voice_client *vc = event.from->get_voice(event.command.guild_id)->voiceclient;
		std::string name = path +"/" + queue[0].id + ".mp3";
		downloadSong(queue[0]);
		extern dpp::cluster bot;
		bot.message_create(dpp::message(event.command.get_channel().id,"Now playing : " + queue[0].title));
		std::vector<uint8_t> pcmdata = prepareAudio(name);
		int pos = 0;
		int chunkLength = 3686400;
		
		//send silence to enter loop
		vc->send_silence(60);
		while(vc->is_playing()){
			auto bot_vc = event.from->get_voice(serverId);
			if(!bot_vc){
			//if bot gets kicked by someone we instantly stop this thread
				killNeeded = 1;
				return;
			}
			//if we send all at once the bot will stop responding on commands for a  very long time(depends on size of a file)
			//so we sending it by a chunks. Value of a chunkLength was getted by  11520*320
			// 11520 from a send_audio_raw description and 320 because lower values cause audio to stutter at some points
			if(pos+chunkLength <pcmdata.size()){
				std::vector<uint8_t> pcdata(pcmdata.begin()+ pos ,pcmdata.begin()+pos+chunkLength);
				vc->send_audio_raw((uint16_t*)pcdata.data(), pcdata.size());
				pos+=chunkLength ;
			}
			else{
				std::vector<uint8_t> pcdata(pcmdata.begin()+ pos ,pcmdata.end());
				vc->send_audio_raw((uint16_t*)pcdata.data(), pcdata.size());
			}
			if(skip){
				vc->stop_audio();
				skip = 0;
			}
			if(leave){
				event.from->disconnect_voice(serverId);
				leave = 0;
				killNeeded = 1;
				return;
			}
			//sleep for a small amount so we dont burn our dearest cpu
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			
		}
		if(queue.size()>0){
			queue.erase(queue.begin());
		}
		
	}
}
void AudioThread::setSkip(bool value)
{
    skip = value;
}
void AudioThread::setLeave(bool value)
{
	skip = value;
    leave = value;
	clearQueue();
}
void AudioThread::downloadSong(VideoData video)
{
	
	std::string command;
	#ifdef _WIN32
    	command = std::string("youtube-dl.exe --extract-audio --audio-format mp3 ") + " -o " + path + "/" + video.id + ".mp3" + " https://www.youtube.com/watch?v=" + video.id;
    #elif __linux__
    	command = std::string("youtube-dl --extract-audio --audio-format mp3 ") + " -o '" + path + "/" + video.id + ".mp3'" + " 'https://www.youtube.com/watch?v=" + video.id+ "' ";
    #endif
	system(command.c_str());
	
}
void AudioThread::addSong(VideoData video)
{
	queue.push_back(video);
}
void AudioThread::clearQueue()
{
	queue.clear();
	skip = true;
}
void AudioThread::findVideo(std::string NAMEVIDEO){
  searchResults.clear();
  CURL *curl;
  int type = 0;
  std::string output = "output for " + NAMEVIDEO + "\n" ;
  while(NAMEVIDEO.find(" ")!=-1){
	NAMEVIDEO.replace(NAMEVIDEO.find(" "),1,"+");
  }
  //if we get as input a url, we try to get an ID value 
  if(NAMEVIDEO.find("https://")!=-1){
	type=1;
	size_t pos = NAMEVIDEO.find("&");
	if(pos!=-1){
	NAMEVIDEO.erase(pos+1);
	}
	pos = NAMEVIDEO.find("?si");
	if(pos!=-1){
	NAMEVIDEO.erase(pos+1);
	}
	pos = NAMEVIDEO.find("?v=");
	if(pos!=-1){
		NAMEVIDEO = NAMEVIDEO.substr(pos+3);
	}
  }
  
  std::string URL;
  //if passed a url then we use retrieved id value
  //else we first convert a name because non english letters  not working
  //then we change every %2B value(which supposed to be a single space)  to "+" which youtube recognizes as spaces
  if(type){
	URL =  "https://www.youtube.com/results?search_query=" + NAMEVIDEO;
  }else{
  	char* NAMEVIDEOCHAR = curl_easy_escape(curl, NAMEVIDEO.c_str(), NAMEVIDEO.length());
  	URL = "https://www.youtube.com/results?search_query=" + std::string(NAMEVIDEOCHAR);
 	 while(URL.find("%2B")!=-1){
		URL.replace(URL.find("%2B"),3,"+");
	   }
  }
  //We use curl to download a html file with our search
  //Then we remove everything except json value in variable ytInitialData which represents all retrieved videos
  //Also using ignoreList because there is no need in channels or playlists
  CURLcode res;
  std::string readBuffer;
  curl = curl_easy_init();
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
	std::string ignoreList[7] = {"radioRenderer","playlistRenderer","channelRenderer","shelfRenderer","reelShelfRenderer","showingResultsForRenderer","didYouMeanRenderer"};
	int k = j["contents"]["twoColumnSearchResultsRenderer"]["primaryContents"]["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]["contents"].size()-1;
	if(k > 5){k = 5;}
	for(int i = 0;i<k;i++){
	    VideoData video;
		int toBreak = 0;
		for(int i1 = 0;i1<7;i1++){
			if(j["contents"]["twoColumnSearchResultsRenderer"]["primaryContents"]["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]["contents"][i].find(ignoreList[i1]) != j["contents"]["twoColumnSearchResultsRenderer"]["primaryContents"]["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]["contents"][i].end()){
				toBreak=1;
				break;
			}
		}
		if(toBreak){
			if(k>i+1){
				k+=1;
			}
			continue;
		}
		//Getting a video id and  a title from json
		video.id = std::string(j["contents"]["twoColumnSearchResultsRenderer"]["primaryContents"]["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]["contents"][i]["videoRenderer"]["videoId"]);
		video.title = std::string(j["contents"]["twoColumnSearchResultsRenderer"]["primaryContents"]["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]["contents"][i]["videoRenderer"]["title"]["runs"][0]["text"]);
		searchResults.push_back(video);
	}
  }
}
//function for a curl that retrieves content
size_t AudioThread::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
