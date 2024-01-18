 	# Minimum CMake version required, we'll just use the latest version.
     cmake_minimum_required(VERSION 3.22)
     # Project name, version and description
     project(discord-bot VERSION 1.0 DESCRIPTION "A discord bot")
     list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
      
     # Create an executable
     add_executable(${PROJECT_NAME}
         src/main.cpp
	 src/commands_listener.h
	 src/commands_listener.cpp
	 src/videoData.h
	 src/videoData.cpp
	 src/audioThread.h
	 src/audioThread.cpp	 
     )      
     # Find our pre-installed DPP package (using FindDPP.cmake).
     find_package(DPP REQUIRED)
     find_package(CURL REQUIRED)
     if(WIN32)
     find_package(fmt REQUIRED)
     find_package(mpg123 REQUIRED)
     find_package(nlohmann_json REQUIRED)
     find_package(unofficial-getopt-win32 REQUIRED)
     

     # Link the pre-installed DPP package.
     target_link_libraries(${PROJECT_NAME} 
         ${DPP_LIBRARIES}
	 dpp::dpp
	 fmt::fmt
	 nlohmann_json::nlohmann_json
	 CURL::libcurl
	 MPG123::libmpg123 
         MPG123::libout123 
         MPG123::libsyn123
         unofficial::getopt-win32::getopt
     )
     endif()
     if(UNIX)
     target_link_libraries(${PROJECT_NAME} 
         ${DPP_LIBRARIES}
	 curl
	 mpg123
     )
     endif()
     # Include the DPP directories.
     target_include_directories(${PROJECT_NAME} PRIVATE
         ${DPP_INCLUDE_DIR}
	 
     )
      
     # Set C++ version
     set_target_properties(${PROJECT_NAME} PROPERTIES
         CXX_STANDARD 17
         CXX_STANDARD_REQUIRED ON
     )
