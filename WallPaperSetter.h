#pragma once

#include <shobjidl.h>
#include <iostream>
#include <vector>
#include "tinyxml.h"
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <wtsapi32.h>
#include <tlhelp32.h>


//Struct def
struct BackgroungTriplet {
	int id;
	std::wstring wallpaper1;
	std::wstring wallpaper2;
	std::wstring wallpaper3;

};

//Config class def
class Config {
public:
	std::vector<BackgroungTriplet> backgrounds;
	std::wstring defaultWallpaperPath;
	int display1 = 0;
	int display2 = 0;
	int display3 = 0;
public:
	Config(const std::string& path);

};

class WallPaperSetter
{
public:
	WallPaperSetter(std::string configFilePath);

	int RunWallPaperSetter();

	std::unique_ptr<Config> myConf;
};

