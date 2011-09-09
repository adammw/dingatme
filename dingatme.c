/*
 * DingAtMe v0.2
 * To compile: gcc -Wl,-subsystem,windows -lwinmm -o dingatme dingatme.c
 * To exit: Press Enter 5 times
 */

#define _WIN32_WINNT 0x0501
#define DEBUG 0
#if defined(DEBUG) && DEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <windows.h>

LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
WPARAM MsgLoop();
void RegisterKeyboardForRawInput(HWND);
void HandleInput(WPARAM, LPARAM);

LRESULT CALLBACK MainWndProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam)    // second message parameter
{
	switch (uMsg)
	{
		case WM_INPUT:
			HandleInput(wParam, lParam);
			return 0;

		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

int APIENTRY WinMain(
    HINSTANCE hinstance,  // handle to current instance
    HINSTANCE hinstPrev,  // handle to previous instance
    LPSTR lpCmdLine,      // address of command-line string
    int nCmdShow)         // show-window type
{
	WNDCLASS wc;
	HWND hwnd;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC) MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance;
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "MainWindowClass";

	if (!RegisterClass(&wc))
	   return EXIT_FAILURE;

	hwnd = CreateWindowEx(0, "MainWindowClass", NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hinstance, NULL);
    if(hwnd == NULL)
    	return EXIT_FAILURE;

    RegisterKeyboardForRawInput(hwnd);

	return MsgLoop();
}

WPARAM MsgLoop()
{
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

void RegisterKeyboardForRawInput(HWND hwndTarget)
{
	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01;
	rid.usUsage = 0x06;
	rid.dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;
	rid.hwndTarget = hwndTarget;

	if (RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)) == FALSE)
		exit(EXIT_FAILURE);
}

void HandleInput(WPARAM wParam, LPARAM lParam)
{
	static int consecutiveEnterPresses = 0;
    UINT dwSize;
    RAWINPUT *raw;
    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
    raw = (RAWINPUT*)malloc(sizeof(BYTE) * dwSize);
    if (raw == NULL)
    	exit(EXIT_FAILURE);
    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize )
    	exit(EXIT_FAILURE);
    if (raw->header.dwType == RIM_TYPEKEYBOARD)
    {
    	if ((raw->data.keyboard.Flags & RI_KEY_BREAK) == RI_KEY_BREAK) {
			switch (raw->data.keyboard.VKey) {
				case VK_RETURN:
					PlaySound("SystemQuestion", NULL, SND_ASYNC | SND_NOSTOP | SND_NOWAIT );
					consecutiveEnterPresses++;
					break;

				default:
					consecutiveEnterPresses = 0;
					break;
			}
    	}
    	if (consecutiveEnterPresses > 5)
    		exit(EXIT_SUCCESS);
#if defined(DEBUG) && DEBUG
    	printf("%i Kbd: make=%04x Flags:%04x Reserved:%04x ExtraInformation:%08x, msg=%04x VK=%04x \n",
    				consecutiveEnterPresses,
    				raw->data.keyboard.MakeCode,
    	            raw->data.keyboard.Flags,
    	            raw->data.keyboard.Reserved,
    	            raw->data.keyboard.ExtraInformation,
    	            raw->data.keyboard.Message,
    	            raw->data.keyboard.VKey);
#endif
    }
}
