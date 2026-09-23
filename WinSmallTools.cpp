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



//Struct def
struct BackgroungTriplet {
	int id;
	std::wstring wallpaper1;
	std::wstring wallpaper2;
	std::wstring wallpaper3;

};

// Helpers functions
std::wstring toWide(const std::string& str)
{
	int size = MultiByteToWideChar(
		CP_UTF8, 0,
		str.c_str(), -1,
		nullptr, 0
	);

	std::wstring result(size, L'\0');

	MultiByteToWideChar(
		CP_UTF8, 0,
		str.c_str(), -1,
		result.data(), size
	);

	return result;
}
template <typename T>
const T& random_element(const std::vector<T>& v) {
	static std::random_device rd;
	static std::mt19937 gen(rd());

	std::uniform_int_distribution<std::size_t> dist(0, v.size() - 1);
	return v[dist(gen)];
}
static bool isXmlFileValid(const std::string& filePath)
{
	TiXmlDocument xmlDoc;
	if (!xmlDoc.LoadFile(filePath.c_str()))
	{
		std::cout << "Failed to parse XML file: " << filePath << std::endl;
		std::cout << "Error description: " << xmlDoc.ErrorDesc() << std::endl;
		return false;
	}

	return true;
}


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

Config::Config(const std::string& path)
{
	TiXmlDocument xmlDoc(path.c_str());
	if (xmlDoc.LoadFile())
	{
		// XML file loaded successfully
		TiXmlElement* rootElement = xmlDoc.RootElement();
		if (rootElement)
		{
			TiXmlElement* childDefault = rootElement->FirstChildElement("defaultWallpaperPath");
			if (childDefault)
			{
				defaultWallpaperPath = toWide(childDefault->Attribute("path"));
			}
			else
			{
				std::cerr << "Missing Default WallPaper in config." << std::endl;
			}

			TiXmlElement* childMonitor = rootElement->FirstChildElement("monitorDisplay");
			if (childMonitor)
			{
				display1 = std::stoi(childMonitor->Attribute("display1"));
				display2 = std::stoi(childMonitor->Attribute("display2"));
				display3 = std::stoi(childMonitor->Attribute("display3"));

			}
			else
			{
				std::cerr << "Missing display order in config." << std::endl;
			}
			TiXmlElement* Triplets = rootElement->FirstChildElement("backgroungTriplets");
			if (Triplets)
			{
				std::string folderPath = Triplets->Attribute("folderPath");
				TiXmlElement* Triplet = Triplets->FirstChildElement("backgroungTriplet");

				while (Triplet)
				{
					struct BackgroungTriplet tempTriplet {};

					tempTriplet.id = std::stoi(Triplet->Attribute("id"));
					tempTriplet.wallpaper1 = toWide(folderPath + Triplet->Attribute("wallpaper1"));

					tempTriplet.wallpaper2 = toWide(folderPath + Triplet->Attribute("wallpaper2"));

					tempTriplet.wallpaper3 = toWide(folderPath + Triplet->Attribute("wallpaper3"));



					backgrounds.push_back(tempTriplet);
					Triplet = Triplet->NextSiblingElement();


				}
			}
			else
			{
				std::cerr << "Missing triplets in config." << std::endl;
			}

		}
	}
}
int RunWinSmallTools(std::string configFilePath)
{
	Config conf(configFilePath);

	// 1. Initialize the COM library
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (FAILED(hr)) {
		std::cerr << "Failed to initialize COM." << std::endl;
		return 1;
	}

	// 2. Create an instance of the DesktopWallpaper object
	IDesktopWallpaper* pDesktopWallpaper = nullptr;
	hr = CoCreateInstance(CLSID_DesktopWallpaper, NULL, CLSCTX_LOCAL_SERVER,
		IID_PPV_ARGS(&pDesktopWallpaper));

	if (SUCCEEDED(hr)) {
		UINT monitorCount = 0;
		pDesktopWallpaper->GetMonitorDevicePathCount(&monitorCount);
		int monitorCount2 = GetSystemMetrics(SM_CMONITORS);
		if (monitorCount2 == 3) {
			LPWSTR monitor1ID = nullptr;
			LPWSTR monitor2ID = nullptr;
			LPWSTR monitor3ID = nullptr;
			// Get the ID of the first monitor (index 0)
			hr = pDesktopWallpaper->GetMonitorDevicePathAt(conf.display1, &monitor1ID);
			hr = pDesktopWallpaper->GetMonitorDevicePathAt(conf.display2, &monitor2ID);
			hr = pDesktopWallpaper->GetMonitorDevicePathAt(conf.display3, &monitor3ID);

			BackgroungTriplet choosenBackgrounds = random_element(conf.backgrounds);
			if (SUCCEEDED(hr)) {

				// 3. Set the wallpaper for the specific monitor ID
				hr = pDesktopWallpaper->SetWallpaper(monitor1ID, choosenBackgrounds.wallpaper1.c_str());
				hr = pDesktopWallpaper->SetWallpaper(monitor2ID, choosenBackgrounds.wallpaper2.c_str());
				hr = pDesktopWallpaper->SetWallpaper(monitor3ID, choosenBackgrounds.wallpaper3.c_str());

				if (SUCCEEDED(hr)) {
					std::cout << "Successfully changed monitor all three monitors backgroundfrom triplet " << choosenBackgrounds.id << "!" << std::endl;
				}
				else {
					std::cerr << "Failed to set wallpaper. HRESULT: " << hr << std::endl;
				}

				// Free the monitor ID allocated by the system
				CoTaskMemFree(monitor1ID);
				CoTaskMemFree(monitor2ID);
				CoTaskMemFree(monitor3ID);
			}
		}
		else {
			std::cerr << "Different than 3 monitors detected, applying default BG to everything." << std::endl;

			for (std::size_t monitorindex = 0;monitorindex < monitorCount;monitorindex++)
			{
				LPWSTR monitorID = nullptr;
				hr = pDesktopWallpaper->GetMonitorDevicePathAt(monitorindex, &monitorID);
				if (SUCCEEDED(hr)) {
					// Path to your wallpaper image (Must be an absolute path)
					hr = pDesktopWallpaper->SetWallpaper(monitorID, conf.defaultWallpaperPath.c_str());
				}
			}

			// Release the interface pointer
			pDesktopWallpaper->Release();
		}
	}
	else {
		std::cerr << "Failed to create IDesktopWallpaper instance." << std::endl;
	}
	// 4. Uninitialize COM
	CoUninitialize();
	return 0;
}

int main(int argc, char* argv[]) {

	if (argc != 2)
	{
		std::cout << "Missing parameter: <exe> [in]config-file-path" << std::endl;
		return 1;
	}
	std::string configFilePath = argv[1];


	//Wallpaper thread
	using namespace std::chrono_literals;

	std::thread wallpaperThread([&configFilePath]()
		{
			using namespace std::chrono_literals;

			auto nextRun = std::chrono::steady_clock::now();

			while (true)
			{
				RunWinSmallTools(configFilePath);

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


