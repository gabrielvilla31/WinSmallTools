#include "WallPaperSetter.h"




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

WallPaperSetter::WallPaperSetter(std::string configFilePath)
{
	myConf= std::make_unique<Config>(configFilePath);
}
int WallPaperSetter::RunWallPaperSetter()
{
	

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
			hr = pDesktopWallpaper->GetMonitorDevicePathAt(myConf->display1, &monitor1ID);
			hr = pDesktopWallpaper->GetMonitorDevicePathAt(myConf->display2, &monitor2ID);
			hr = pDesktopWallpaper->GetMonitorDevicePathAt(myConf->display3, &monitor3ID);

			BackgroungTriplet choosenBackgrounds = random_element(myConf->backgrounds);
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
					hr = pDesktopWallpaper->SetWallpaper(monitorID, myConf->defaultWallpaperPath.c_str());
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