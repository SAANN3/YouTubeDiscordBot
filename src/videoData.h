#ifndef VIDEODATA
#define VIDEODATA
#include <iostream>
#include <vector>
class VideoData{
    public:
        std::string id;
        std::string title;
        static std::string stringFromVector(std::vector<VideoData> vector);

};
#endif