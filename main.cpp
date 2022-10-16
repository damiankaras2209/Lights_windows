#include <cstdio>
#include <csignal>
#include <thread>
#include <cstdlib>
#include <windows.h>
#include <iostream>
#include <sstream>

#include "request.h"

#define IP_ADDRESS "192.168.0.201"

bool headless = true;

HHOOK hKeyboardHook;

__declspec(dllexport) LRESULT CALLBACK KeyboardEvent (int nCode, WPARAM wParam, LPARAM lParam) {

    DWORD CTRL_key;
    DWORD ALT_key;

    if  (nCode == HC_ACTION &&   (wParam == WM_SYSKEYDOWN ||  wParam == WM_KEYDOWN)) {
        KBDLLHOOKSTRUCT hooked_key =    *((KBDLLHOOKSTRUCT*)lParam);

        DWORD key = hooked_key.vkCode;

        CTRL_key = GetAsyncKeyState(VK_CONTROL);
        ALT_key = GetAsyncKeyState(VK_MENU);

            if (CTRL_key !=0 && ALT_key !=0) {

				int index;

				switch (key) {
					case 'i': index = -1; break;
					case '`': index = 0; break;
					case 'a': index = 1; break;
					case 'b': index = 2; break;
					case 'c': index = 3; break;
					case 'd': index = 4; break;
					case 'e': index = 5; break;
					case 'f': index = 6; break;
                    default: index = -2;
				}

				if(index >= 1) {
                    std::stringstream ss;
                    ss << "/switch/bulb_" << index + 1 << "/toggle";
                    HttpsWebRequestPost(IP_ADDRESS, ss.str(), "");
				} else if (index == 0) {
                    HttpsWebRequestPost(IP_ADDRESS, "button/all/press", "");
				} else if (index == -1) {
					headless = abs(headless - 1);
					ShowWindow(GetConsoleWindow(), !headless);
				}
            }
    }
    return CallNextHookEx(hKeyboardHook,    nCode,wParam,lParam);
}

void MessageLoop() {
    MSG message;
    while (GetMessage(&message, nullptr,0,0)) {
        TranslateMessage( &message );
        DispatchMessage( &message );
    }
}

void key_listener(LPVOID lpParm) {
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    if (!hInstance) hInstance = LoadLibrary((LPCSTR)lpParm); 
    hKeyboardHook = SetWindowsHookEx (WH_KEYBOARD_LL, (HOOKPROC)KeyboardEvent, hInstance, 0);
    MessageLoop();
    UnhookWindowsHookEx(hKeyboardHook);
}


int launchOnStartup() {	
    TCHAR szPath[MAX_PATH];
    DWORD pathLen;

    // GetModuleFileName returns the number of characters
    // written to the array.
    pathLen = GetModuleFileName(nullptr, szPath, MAX_PATH);
    if (pathLen == 0) {
		printf(TEXT("Unable to get module file name; last error = %lu\n"), GetLastError());
		return 1;
    }

    HKEY newValue;
	
    if (RegOpenKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &newValue) != ERROR_SUCCESS) {
		printf(TEXT("Unable to open registry key; last error = %lu\n"), GetLastError());
		return 1;
    }
 
    // Need to pass the length of the path string in bytes,
    // which may not equal the number of characters due to
    // character set.
    DWORD pathLenInBytes = pathLen * sizeof(*szPath);
    if (RegSetValueEx(newValue, TEXT("Lights"), 0, REG_SZ, (LPBYTE)szPath, pathLenInBytes) != ERROR_SUCCESS) {
		RegCloseKey(newValue);
		printf(TEXT("Unable to set registry value; last error = %lu\n"), GetLastError());
	    return 1;
    }

    RegCloseKey(newValue);
	
	return 0;
}

int main(int argc, char *argv[])
{
	launchOnStartup();

	std::thread t1(key_listener, (LPVOID) argv[0]);
	ShowWindow(GetConsoleWindow(), !headless);
	t1.join();
	
	return 0;
}