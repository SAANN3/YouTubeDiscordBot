#ifndef AUDIOTHREAD
#define AUDIOTHREAD

#include <thread>
#include <dpp/dpp.h>
#include "videoData.h"
class AudioThread 
{
    public:
        AudioThread();
       
        void join();
        void start(const dpp::slashcommand_t& event);
        void setSkip(bool value);
        void setLeave(bool value);
        void addSong(VideoData);
        void clearQueue();
        void joinThread();
        std::string getListQueue();
        std::vector <VideoData> getSearchResults();
        void findVideo(std::string NAMEVIDEO);
        int killNeeded = 0;
       
       
    private:
        void downloadSong(VideoData video);
        
        std::vector<VideoData> searchResults;
        std::string path = "/tmp/discordBot";
        
        bool skip = 0;
        bool leave = 0;
        std::vector<VideoData> queue;
        void run(const dpp::slashcommand_t& event);
        std::thread thread;
        static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
        std::vector<uint8_t> prepareAudio(std::string name);

    protected:
        
            


};

#endif