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



#pragma comment(lib, "Wtsapi32.lib")

bool isChromeRunning()
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (Process32First(snapshot, &entry))
	{
		do
		{
			if (_wcsicmp(entry.szExeFile, L"chrome.exe") == 0)
			{
				CloseHandle(snapshot);
				return true;
			}
		} while (Process32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);
	return false;
}

void openChromeTabs()
{
	ShellExecuteA(
		NULL,
		"open",
		"chrome.exe",
		"https://outlook.cloud.microsoft/mail/ https://outlook.live.com/mail/0/ https://music.youtube.com/",
		NULL,
		SW_SHOWNORMAL
	);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	if (msg == WM_WTSSESSION_CHANGE)
	{
		if (wParam == WTS_SESSION_LOGON || wParam == WTS_SESSION_UNLOCK)
		{
			std::cout << "Messsage : " << wParam << std::endl;
			std::cout << "Utilisateur connecte\n";

			if (!isChromeRunning())
			{
				std::cout << "Chrome non ouvert -> ouverture\n";
				openChromeTabs();
			}
			else
			{
				std::cout << "Chrome deja ouvert\n";
			}
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}





int main(int argc, char* argv[]) {

	if (argc != 2)
	{
		std::cout << "Missing parameter: <exe> [in]config-file-path" << std::endl;
		return 1;
	}
	std::string configFilePath = argv[1];
	WallPaperSetter myWallPaperSetter(configFilePath);

	//Wallpaper thread
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


	//ChromeDeamon
	//init
	if (!isChromeRunning())
	{
		std::cout << "Chrome non ouvert -> ouverture\n";
		openChromeTabs();
	}

	//loop setup
	HINSTANCE hInstance = GetModuleHandle(NULL);

	const wchar_t CLASS_NAME[] = L"SessionListener";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		0,
		CLASS_NAME,
		L"SessionListener",
		0,
		0, 0, 0, 0,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	WTSRegisterSessionNotification(hwnd, NOTIFY_FOR_THIS_SESSION);

	MSG msg;

	//loop
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
}


