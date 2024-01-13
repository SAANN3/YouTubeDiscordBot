#include "videoData.h"

std::string VideoData::stringFromVector(std::vector<VideoData> vector){
	std::string output;
	for(int i = 0;i<vector.size();i++){
		output += std::to_string(i) + ")" + vector[i].title + "\n";
	}
	return output;
}