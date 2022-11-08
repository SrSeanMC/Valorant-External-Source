#include "obsidium64.h"

#include "globals.hpp"
#include "cheat.hpp"
#include "print.hpp"
#include "auth.hpp"

#include <TlHelp32.h>
#include <filesystem>
#include <fstream>

#include <iostream> 
#include <Windows.h> // OS function
#include <Psapi.h> 


void SetConsoleSize()
{
	HANDLE hOut;
	SMALL_RECT DisplayArea = { 0, 0, 0, 0 };
	int x = 84;
	int y = 26;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DisplayArea.Right = x;
	DisplayArea.Bottom = y;

	SetConsoleWindowInfo(hOut, TRUE, &DisplayArea);
}

void rndmTitle()
{

	constexpr int length = 15;
	const auto characters = TEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	TCHAR title[length + 1]{};

	for (int j = 0; j != length; j++)
	{
		title[j] += characters[rand() % 55 + 1];
	}

	SetConsoleTitle(title);

}

#include "Mapper/Kdmapper/kdmapper.hpp"

HANDLE iqvw64e_device_handle;

LONG WINAPI SimplestCrashHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
	if (ExceptionInfo && ExceptionInfo->ExceptionRecord)
		std::cout << skCrypt("Driver Error at 0x") << ExceptionInfo->ExceptionRecord->ExceptionAddress << skCrypt(" by 0x") << std::hex << ExceptionInfo->ExceptionRecord->ExceptionCode << std::endl;
	else
		std::cout << skCrypt("Driver Crash") << std::endl;

	if (iqvw64e_device_handle)
		intel_driver::Unload(iqvw64e_device_handle);

	return EXCEPTION_EXECUTE_HANDLER;
}

int load_driver()
{
	SetUnhandledExceptionFilter(SimplestCrashHandler);
	if (intel_driver::IsRunning())
	{
		std::cout << skCrypt("Restart PC!") << std::endl;
		Sleep(500);
	}
	iqvw64e_device_handle = intel_driver::Load();

	NTSTATUS exitCode = 0;
	if (!kdmapper::MapDriver(iqvw64e_device_handle, 0, 0, false, true, 0, 0, 0, &exitCode)) {
		{
			std::cout << skCrypt("Failed to map the driver!") << std::endl;
			intel_driver::Unload(iqvw64e_device_handle);
			Sleep(500);
			exit(0);
		}
	}
	intel_driver::Unload(iqvw64e_device_handle);

	Sleep(500);
}


HWND Entryhwnd;

uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += offsets[i];
	}
	return addr;
}

int main()
{
	SetConsoleTitleA("Your shitty p2c name");

	CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hConsole, &screenBufferInfo);
	COORD new_screen_buffer_size;
	new_screen_buffer_size.X = screenBufferInfo.srWindow.Right -
		screenBufferInfo.srWindow.Left + 1;
	new_screen_buffer_size.Y = screenBufferInfo.srWindow.Bottom -
		screenBufferInfo.srWindow.Top + 1;
	SetConsoleScreenBufferSize(hConsole, new_screen_buffer_size);


	if (GlobalFindAtomA(skCrypt("DriverAlreadyLoadedx")) == 0)
	{
		load_driver();
		GlobalAddAtomA(skCrypt("DriverAlreadyLoadedx"));
	}

	Sleep(500);

	while (Entryhwnd == NULL)
	{
		std::cout << "[>] Waiting for valorant" << std::endl;;
		Entryhwnd = FindWindowA(0, skCrypt("Valorant  "));
		Sleep(1);
	}

	Beep(300, 300);

	if (mem::find_driver())
	{
		std::cout << ("[>] Driver Loaded\n");
	}
	else Log4(skCrypt("\nDriver Failed... \n"));

	mem::find_process(L"VALORANT-Win64-Shipping.exe");
	virtualaddy = mem::get_guarded_region();

	uintptr_t world = read<uintptr_t>(virtualaddy + 0x60);
	world = check::validate_pointer(world);
	system(skCrypt("cls"));
	std::cout << "[>] Uworld: 0x" << std::hex << world << std::endl;
	std::cout << "[>] Base Address: 0x" << std::hex << mem::base_address() << std::endl;

	start_cheat();
}
