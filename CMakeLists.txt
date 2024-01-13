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
     )      
     # Find our pre-installed DPP package (using FindDPP.cmake).
     find_package(DPP REQUIRED)
     find_package(CURL REQUIRED)
      
     # Link the pre-installed DPP package.
     target_link_libraries(${PROJECT_NAME} 
         ${DPP_LIBRARIES}
	 curl
	 mpg123
     )
      
     # Include the DPP directories.
     target_include_directories(${PROJECT_NAME} PRIVATE
         ${DPP_INCLUDE_DIR}
     )
      
     # Set C++ version
     set_target_properties(${PROJECT_NAME} PROPERTIES
         CXX_STANDARD 17
         CXX_STANDARD_REQUIRED ON
     )
