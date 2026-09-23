#include "ChromeDeamon.h"

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



ChromeDeamon::ChromeDeamon()
{
}

void ChromeDeamon::run()
{
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
