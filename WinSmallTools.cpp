#include <windows.h>
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
#include "WallPaperSetter.h"
#include "ChromeDeamon.h"



#pragma comment(lib, "Wtsapi32.lib")





int main(int argc, char* argv[]) {

	if (argc != 2)
	{
		std::cout << "Missing parameter: <exe> [in]config-file-path" << std::endl;
		return 1;
	}
	std::string configFilePath = argv[1];

	//Wallpaper thread
	WallPaperSetter myWallPaperSetter(configFilePath);
	using namespace std::chrono_literals;
	std::thread wallpaperThread([&myWallPaperSetter]()
		{
			using namespace std::chrono_literals;

			auto nextRun = std::chrono::steady_clock::now();

			while (true)
			{
				myWallPaperSetter.RunWallPaperSetter();

				nextRun += 1h;
				std::this_thread::sleep_until(nextRun);
			}
		});


	//ChromeDeamon thread
	ChromeDeamon myChromeDeamon{};
	std::thread ChromeDeamonThread([&myChromeDeamon]()
		{
			myChromeDeamon.run();

		});

	
	ChromeDeamonThread.join();

}

