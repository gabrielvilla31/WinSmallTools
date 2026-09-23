#pragma once
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

class ChromeDeamon
{
public:
	ChromeDeamon();

	void run();
};

