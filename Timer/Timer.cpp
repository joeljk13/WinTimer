#include "Timer.h"

void CALLBACK BringTimerUp(void *hWnd, BOOLEAN timerOrWaitFired)
{
	PostMessage((HWND)hWnd, WM_SETTIME, TMR_ADDSEC, NULL);
}

// Set the timer queue for bringing the timer up
DWORD _stdcall TimerUp(void *hWnd)
{
	HANDLE timer = NULL;
	HANDLE queue = CreateTimerQueue();
	CreateTimerQueueTimer(&timer, queue, &BringTimerUp, hWnd, 999, 1000, 0);
	PINFO info = (PINFO)GetWindowLongPtr((HWND)hWnd, GWLP_USERDATA);
	info->hTimerQueue = queue;
	return 0;
}

void CALLBACK BringTimerDown(void *hWnd, BOOLEAN timerOrWaitFired)
{
	PostMessage((HWND)hWnd, WM_SETTIME, TMR_SUBSEC, NULL);
}

// Set the timer queue for bring the timer down
DWORD _stdcall TimerDown(void *hWnd)
{
	HANDLE timer = NULL;
	HANDLE queue = CreateTimerQueue();
	CreateTimerQueueTimer(&timer, queue, &BringTimerDown, hWnd, 999, 1000, 0);
	PINFO info = (PINFO)GetWindowLongPtr((HWND)hWnd, GWLP_USERDATA);
	info->hTimerQueue = queue;
	return 0;
}

// Beep!
DWORD _stdcall Beep(void *null)
{
	Beep(988, 300);
	return 0;
}

BOOL Beep(void)
{
	HANDLE beeper = CreateThread(NULL, 16, &Beep, NULL, 0, NULL);
	CloseHandle(beeper);
	return beeper ? TRUE : FALSE;
}

// Turns a number into a character of that number
char GetNumChar(BYTE num)
{
	return (num >= 1 && num <= 9) ? num + '0' : '0';
}

// Stop the timer.
void StopTimer(PINFO info)
{
	if (info->hTimerQueue) {
		DeleteTimerQueueEx(info->hTimerQueue, INVALID_HANDLE_VALUE);
		info->hTimerQueue = NULL;
	}
	if (info->hTimer) {
		TerminateThread(info->hTimer, 1);
		CloseHandle(info->hTimer);
		info->hTimer = NULL;
	}
}

// Handles windows messages.
LRESULT _stdcall WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Define variables
	LRESULT lResult = 0;
	PAINTSTRUCT ps;
	PINFO info;
	PTIME time;
	char *lpcTime;
	switch (msg)
	{
	case WM_SETTIME:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_RESET:
	case WM_PAINT:
	case WM_CLOSE:
	case WM_CHAR:
		// All of these messages need this info
		info = (PINFO)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}
	switch (msg)
	{
	case WM_SETTIME:
		// Sets the time on the timer.
		// wParam says whether to add or subtract a second.
		if (wParam == TMR_ADDSEC) {
			// Make sure the timer is set to ON
			info->isSet = TRUE;
			// Set the new time
			time = info->pTime;
			if (time->seconds + 1 < 60)
				time->seconds++;
			else {
				time->seconds = 0;
				if (time->minutes + 1 < 60)
					time->minutes++;
				else {
					time->minutes = 0;
					if (time->hours + 1 < 100)
						time->hours++;
					else SendMessage(hWnd, WM_SETTIME, TMR_RESET, NULL);
				}
			}
			// If the timer is done, beep
			if (time->seconds == 0 && time->minutes == 0 && time->hours == 0)
				Beep();
		}
		else if (wParam == TMR_SUBSEC) {
			// Make sure the timer is ON
			info->isSet = TRUE;
			// Set the new time
			time = info->pTime;
			if (time->seconds > 0)
				time->seconds--;
			else {
				time->seconds = 59;
				if (time->minutes > 0)
					time->minutes--;
				else {
					time->minutes = 59;
					if (time->hours > 0)
						time->hours--;					
				}
			}
			// If the timer is done, beep, set the timer to OFF, and reset the timer
			if (time->seconds == 0 && time->minutes == 0 && time->hours == 0) {
				Beep();
				info->isOn = FALSE;
				SendMessage(hWnd, WM_RESET, NULL, NULL);
			}
		}
		else {
			// If wParam is not a valid value,
			// leave the function and report an error (with lResult)
			if (wParam != TMR_RESET && wParam != TMR_SET) {
				lResult = 2;
				break;
			}
			// Releas the memory from the old time, it's no longer needed
			if (info->pTime)
				free(info->pTime);
			// If the time is being set to a specific
			// time, get that time from lParam
			if (wParam == TMR_SET) {
				time = (PTIME)lParam;
				if (info->pBaseTime)
					free(info->pBaseTime);
				info->pBaseTime = time;
			}
			else time = info->pBaseTime;
			if (!time) {
				time = NEW(TIME);
				if (!time) {
					lResult = 1;
					break;
				}
				if (info->pBaseTime)
					free(info->pBaseTime);
				info->pBaseTime = time;
			}
			time = NEW(TIME);
			if (!time) {
				lResult = 1;
				break;
			}
			time->hours = info->pBaseTime->hours;
			time->minutes = info->pBaseTime->minutes;
			time->seconds = info->pBaseTime->seconds;
			// check if it should go down or up, and set it to OFF
			info->isGoingDown = time->hours != 0 || time->minutes != 0 || time->seconds != 0;
			info->isSet = FALSE;
		}
		// Take the time, and convert it into a string
		info->pTime = time;
		lpcTime = new char[9];
		if (!lpcTime) {
			lResult = 1;
			break;
		}
		if (info->lpcTime)
			delete [] info->lpcTime;
		lpcTime[0] = GetNumChar(time->hours / 10);
		lpcTime[1] = GetNumChar(time->hours % 10);
		lpcTime[2] = ':';
		lpcTime[3] = GetNumChar((time->minutes / 10) % 6);
		lpcTime[4] = GetNumChar(time->minutes % 10);
		lpcTime[5] = ':';
		lpcTime[6] = GetNumChar((time->seconds / 10) % 6);
		lpcTime[7] = GetNumChar(time->seconds % 10);
		lpcTime[8] = '\0';
		// info->lpcTime is what WM_PAINT uses to print the time
		info->lpcTime = lpcTime;
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_PAINT:
		TextOut(BeginPaint(hWnd, &ps), 5, 5, info->lpcTime, 8);
		EndPaint(hWnd, &ps);
		break;
	case WM_LBUTTONDOWN:
		// If the timer is on, stop it; otherwise, start it up
		if (info->isOn) {
			StopTimer(info);
			info->isOn = FALSE;
		}
		else {
			info->hTimer = CreateThread(NULL, 0, info->isGoingDown ? &TimerDown : &TimerUp, hWnd, 0, NULL);
			info->isSet = TRUE;
			info->isOn = TRUE;
		}
		break;
	case WM_RBUTTONDOWN:
		// If the timer is not is it's reset state, reset it
		if (!info->isSet)
			break;
	case WM_RESET:
		// Stop the timer and reset it; if it was going, keep it going
		StopTimer(info);
		if (info->isOn) {
			info->hTimer = CreateThread(NULL, 0, info->isGoingDown ? &TimerDown : &TimerUp, hWnd, 0, NULL);
		}
		SendMessage(hWnd, WM_SETTIME, TMR_RESET, NULL);
		break;
	case WM_CHAR:
		// Get a char typed by the user; see if it's a number.
		// If it is, set the next timer value to it.
		if (wParam < '0' || wParam > '9' || (wParam > '5' && (info->wTypedTimePlace == 2 ||
			info->wTypedTimePlace == 4)))
			break;
		wParam -= '0';
		time = NEW(TIME);
		if (!time) {
			lResult = 1;
			break;
		}
		time->hours = info->pTime->hours;
		time->minutes = info->pTime->minutes;
		time->seconds = info->pTime->seconds;
		// Change the time based on the next place
		switch (info->wTypedTimePlace)
		{
		case 0:
			time->hours = time->hours % 10;
			time->hours += wParam * 10;
			break;
		case 1:
			time->hours -= time->hours % 10;
			time->hours += wParam;
			break;
		case 2:
			time->minutes = time->minutes % 10;
			time->minutes += wParam * 10;
			break;
		case 3:
			time->minutes -= time->minutes % 10;
			time->minutes += wParam;
			break;
		case 4:
			time->seconds = time->seconds % 10;
			time->seconds += wParam * 10;
			break;
		case 5:
			time->seconds -= time->seconds % 10;
			time->seconds += wParam;
		}
		if (info->wTypedTimePlace > 4)
			info->wTypedTimePlace = 0;
		else info->wTypedTimePlace++;
		SendMessage(hWnd, WM_SETTIME, TMR_SET, (LPARAM)time);
		SendMessage(hWnd, WM_RESET, NULL, NULL);
		break;
	case WM_CLOSE:
		// Clean up
		if (info->pTime)
			free(info->pTime);
		if (info->pBaseTime)
			free(info->pBaseTime);
		StopTimer(info);
		free(info);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	if (lResult == 1)
		MessageBox(hWnd, "There was an error allocating memory.", "Timer", MB_ICONERROR);
	return lResult;
}

// Checks if code is an argument in the given command line.
// Syntax: "Timer.exe -[arg]"
BOOL IsArg(char *lpCmdLine, char *code)
{
	UINT nCode = 0;
	while (code[nCode])
		nCode++;
	if (nCode > 0)
		for (char c; c = *lpCmdLine++; )
			if (c == '-' && *lpCmdLine == *code) {
				WORD nArg = 0;
				while (lpCmdLine[nArg] && lpCmdLine[nArg] != ' ')
					nArg++;
				if (nCode != nArg)
					continue;
				BOOL eq = TRUE;
				char *codeCpy = code;
				char *lpCmdLineCpy = lpCmdLine;
				for (char c; c = *++codeCpy; )
					if (c != *++lpCmdLineCpy) {
						eq = FALSE;
						break;
					}
				if (eq)
					return TRUE;
			}
	return FALSE;
}

// Since this program is based off a GUI, it uses 
// WinMain instead of int main for the Windows OS.
int _stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, char *lpCmdLine, int nCmdShow)
{
	// Create the window
	HBRUSH hBg = CreateSolidBrush(RGB(255, 255, 255));
	HCURSOR hCur = LoadCursor(NULL, IDC_HAND);
	HICON hIconSm = (HICON)LoadImage(hInst, "IDI_ICON", IMAGE_ICON, 16, 16, LR_SHARED);
	HICON hIcon = LoadIcon(hInst, "IDI_ICON");
	if (!hBg || !hCur || !hIconSm || !hIcon) {
		MessageBox(NULL, "There was an error creating an object.", "Timer", MB_ICONERROR);
		return 1;
	}
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = hBg;
	wc.hCursor = hCur;
	wc.lpfnWndProc = WndProc;
	wc.hIcon = hIcon;
	wc.hIconSm = hIconSm;
	wc.hInstance = hInst;
	wc.lpszClassName = "Timer";
	wc.lpszMenuName = NULL;
	wc.style = NULL;
	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "There was an error registering the class.", "Timer", MB_ICONERROR);
		return 2;
	}
	HWND hWnd = CreateWindow("Timer", "Timer", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, 0,
		165, 55, NULL, NULL, hInst, NULL);
	if (!hWnd) {
		MessageBox(NULL, "There was an error creating the window.", "Timer", MB_ICONERROR);
		return 3;
	}
	// Create the thread that changes the time, but don't start it yet
	HANDLE thread = CreateThread(NULL, 0, &TimerUp, (HWND)hWnd, CREATE_SUSPENDED, NULL);
	if (!thread) {
		MessageBox(NULL, "There was an error creating the timer.", "Timer", MB_ICONERROR);
		return 4;
	}
	// Set up the class holding all the info about the timer
	PINFO info = NEW(INFO);
	PTIME time = NEW(TIME);
	if (!info || !time) {
		MessageBox(NULL, "There was an error allocating memory.", "Timer", MB_ICONERROR);
		return 5;
	}
	info->pBaseTime = time;
	info->hTimer = thread;
	// So that this data can be accessed as long as the HWND of the GUI is
	// known, set the timer info as the user data for the window.
	SetWindowLongPtr((HWND)hWnd, GWLP_USERDATA, (LONG)info);
	// Check for arguments for fading.
	// -f = fade both in and out
	// -fi = fade in
	// -fo = fade out
	// Note: this feature was designed on Windows 7 starter,
	// which doesn't fade windows in or out.
	BOOL bFade = IsArg(lpCmdLine, "f");
	if ((bFade || IsArg(lpCmdLine, "fi")) && AnimateWindow(hWnd, 200, AW_BLEND | AW_ACTIVATE))
		SendMessage(hWnd, WM_SETTIME, TMR_RESET, NULL);
	else {
		SendMessage(hWnd, WM_SETTIME, TMR_RESET, NULL);
		ShowWindow(hWnd, nCmdShow);
	}
	// Set up the message loop.
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (!bFade && !IsArg(lpCmdLine, "fo") || !AnimateWindow(hWnd, 200, AW_BLEND | AW_HIDE))
		ShowWindow(hWnd, SW_HIDE);
	// Clean up
	DeleteObject(hBg);
	DestroyCursor(hCur);
	DestroyIcon(hIcon);
	DestroyIcon(hIconSm);
	return msg.wParam;
}
