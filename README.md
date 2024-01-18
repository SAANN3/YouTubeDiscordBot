
# Youtube discord bot
a simple bot that can play music from youtube



## prerequisite for linux
As dependency you need :```DPP curl fmt ffmpeg mpg123```

Also you need to put somewhere in PATH yt-dlp, for example


```
sudo curl -L https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp -o /usr/local/bin/yt-dlp
sudo chmod a+rx /usr/local/bin/yt-dlp
```
## Installation
[Download](https://github.com/SAANN3/YouTubeDiscordBot/releases) a linux binary or windows zip archive 
## Usage
Create a file near app named BOT_TOKEN and put your bot token retrieved from [here](https://discord.com/developers/applications/)

For first time only after placing token you should launch program with ``` -s ``` argument to register bot commands

Then you can invite bot and start playing music with /play 
### Or if you prefer to compile from source 
#### for linux

```bash
  git clone https://github.com/SAANN3/YouTubeDiscordBot
  cd YouTubeDiscordBot
  mkdir build
  cmake --build build/ -j4
  
```

#### for windows
I used a vcpkg for windows build, if you don't have it you should install it first.
Then using vcpkg install theese packages

``` dpp curl fmt getopt getopt-win32 libsodium mpg123 nlohmann-json openssl ffmpeg```
```
   git clone https://github.com/SAANN3/YouTubeDiscordBot
   cd YouTubeDiscordBot
   mkdir build
   cd build
   cmake -DCMAKE_TOOLCHAIN_FILE="PATH TO VCPKG.cmake" ..
   cmake   --build . --config=release
```
navigate to ./build/Release

now download a [yt-dlp](https://github.com/yt-dlp/yt-dlp#installation) , and put it near our .exe file.
Also we need [ffmpeg](https://www.gyan.dev/ffmpeg/builds/) as dependency for yt-dlp. 
