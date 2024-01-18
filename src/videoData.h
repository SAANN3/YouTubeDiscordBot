#ifndef VIDEODATA
#define VIDEODATA
#include <iostream>
#include <vector>
//class which represents video and holding some data
class VideoData{
    public:
        std::string id;
        std::string title;
        static std::string stringFromVector(std::vector<VideoData> vector);

};
#endif