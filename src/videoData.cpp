#include "videoData.h"
#include <string>
std::string VideoData::stringFromVector(std::vector<VideoData> vector){
	std::string output;
	for(int i = 0;i<vector.size();i++){
		output += std::to_string(i+1) + ")" + vector[i].title + "\n";
	}
	return output;
}
