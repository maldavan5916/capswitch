#include <Windows.h>
#if _DEBUG
#include <stdio.h>
#endif

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
HHOOK hHook;

char enabled = TRUE;
WPARAM key1 = VK_MENU, key2 = VK_LSHIFT;
char keydownCapsLk = FALSE, keydownLShift = FALSE;

struct Settings
{
	// see comments in "CapsWitch.ini" for details
	char EmulatedKeystroke;
	char HideMultiInstanceError;
	char AltCapsToDisable;
	char UseSoundIndication;
}
Settings = { 1, TRUE, FALSE, 0 };



int main(int argc, char** argv)
{
	if (importSettingsFromFile(argv[1], &Settings) == -1) { return 1; }

	HANDLE hMutex = CreateMutex(0, 0, "CapsWitch");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		if (!Settings.HideMultiInstanceError) {
			MessageBox(NULL,
				"Another instance of the program is already running!\n\nThis instance will be terminated.",
				"CapsWitch", MB_OK | MB_ICONWARNING);
		}
		return 1;
	}

	HINSTANCE hinst = GetModuleHandle(NULL); // Windows XP compatibility workaround
	hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hinst, 0);
	if (hHook == NULL) {
		MessageBox(NULL,
			"Error calling \"SetWindowsHookEx(...)\"",
			"CapsWitch", MB_OK | MB_ICONERROR);
		return -1;
	}

	MSG messages;
	while (GetMessage(&messages, NULL, 0, 0)) {
		TranslateMessage(&messages);
		DispatchMessage(&messages);
	}

	UnhookWindowsHookEx(hHook);
	return 0;
}



int importSettingsFromFile(char* settingsFilePath, struct Settings* Settings)
{
	// use default path if arguments didn't provide any
	if (settingsFilePath == NULL) {
		char defaultFilePath[MAX_PATH];
		GetModuleFileName(NULL, defaultFilePath, MAX_PATH);
		// replace 3 last characters (.exe -> .ini)
		defaultFilePath[strlen(defaultFilePath)-3] = 'i';
		defaultFilePath[strlen(defaultFilePath)-2] = 'n';
		defaultFilePath[strlen(defaultFilePath)-1] = 'i';
		settingsFilePath = defaultFilePath;
	}
	// if it is provided, throw error if the file doesn't exist
	else if (GetFileAttributes(settingsFilePath) == INVALID_FILE_ATTRIBUTES) {
		int mbox = MessageBox(NULL,
			"Unable to open specified settings file!\n\nStart anyway with default settings?",
			"CapsWitch", MB_YESNO | MB_ICONWARNING);
		if (mbox == IDNO) { return -1; }
	}
#if _DEBUG
	printf("[importSettingsFromFile] Trying to read %s ...\n", settingsFilePath);
#endif

	int settingsValueBuffer;
	settingsValueBuffer = GetPrivateProfileInt("general", "EmulatedKeystroke", 1, settingsFilePath);
	Settings->EmulatedKeystroke = settingsValueBuffer;
	settingsValueBuffer = GetPrivateProfileInt("general", "HideMultiInstanceError", 1, settingsFilePath);
	Settings->HideMultiInstanceError = settingsValueBuffer;
	settingsValueBuffer = GetPrivateProfileInt("general", "AltCapsToDisable", 0, settingsFilePath);
	Settings->AltCapsToDisable = settingsValueBuffer;
	settingsValueBuffer = GetPrivateProfileInt("general", "UseSoundIndication", 0, settingsFilePath);
	Settings->UseSoundIndication = settingsValueBuffer;

	switch (Settings->EmulatedKeystroke) {
	case 2:
		key1 = VK_CONTROL, key2 = VK_SHIFT;
		break;
	case 3:
		key1 = VK_LWIN, key2 = VK_SPACE;
		break;
	default:
		key1 = VK_LMENU, key2 = VK_SHIFT;
		break;
	}

#if _DEBUG
		printf("[importSettingsFromFile] Settings import finished, values: %d %d %d %d\n",
			Settings->EmulatedKeystroke, Settings->HideMultiInstanceError,
			Settings->AltCapsToDisable, Settings->UseSoundIndication);
#endif
	return 0;
}



inline void PressKey(WPARAM keyCode)
{
	keybd_event(keyCode, 0, 0, 0);
}

inline void ReleaseKey(WPARAM keyCode)
{
	keybd_event(keyCode, 0, KEYEVENTF_KEYUP, 0);
}

void PlaySoundIndication()
{
	switch (Settings.UseSoundIndication) {
	case 1:
		Beep(40, 40);
		break;
	case 2:
		Beep(2000, 160);
		break;
	default:
		break;
	}
}



LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT* key = (KBDLLHOOKSTRUCT*)lParam;
	if (nCode != HC_ACTION || (key->flags & LLKHF_INJECTED)) {
		return CallNextHookEx(hHook, nCode, wParam, lParam);
	}

	if (key->vkCode == VK_CAPITAL) {
		if (Settings.AltCapsToDisable == TRUE && wParam == WM_SYSKEYDOWN) {
			enabled = !enabled;
#if _DEBUG
			printf("[LowLevelKeyboardProc] CapsWitch has been %s.\n", enabled ? "enabled" : "disabled");
#endif
			PlaySoundIndication();
			PlaySoundIndication();
			keydownLShift = TRUE; // fix for language switching right after enabling
			return 1; // prevent default
		}

		if (!enabled) {
			return CallNextHookEx(hHook, nCode, wParam, lParam);
		}
#if _DEBUG
		printf("[LowLevelKeyboardProc] CapsLk is %s\n", (wParam == WM_KEYUP) ? "UP" : "DOWN");
#endif

		if (wParam == WM_KEYUP) {
			keydownCapsLk = FALSE;

			// special behaviour for Win+Space keystroke
			// as the language pop-up is currently opened
			// and Space key is down, we only need to release
			// it to switch languages, or cancel it using Esc
			// (doesn't work sometimes, not sure how to fix))
			if (Settings.EmulatedKeystroke == 3) {
				if (!keydownLShift) {
					ReleaseKey(key1);
				}
				else {
					keydownLShift = FALSE;
					PressKey(VK_ESCAPE);
					ReleaseKey(VK_ESCAPE);
					ReleaseKey(key1);
				}
				PlaySoundIndication();
			}
			// classic keystrokes 
			else if (!keydownLShift) {
				PressKey(key1);
				PressKey(key2);
				ReleaseKey(key1);
				ReleaseKey(key2);
				PlaySoundIndication();
			}
			else {
				keydownLShift = FALSE;
			}
		}

		if (wParam == WM_KEYDOWN && !keydownCapsLk) {
			keydownCapsLk = TRUE;

			if (keydownLShift) {
				PressKey(VK_CAPITAL);
				ReleaseKey(VK_CAPITAL);
#if _DEBUG
				printf("[LowLevelKeyboardProc] Caps Lock state has been toggled.\n");
#endif
			}
			// special behaviour for Win+Space keystroke
			// (we want language pop-up to show up, but
			// don't close it until CapsLk is released)
			else if (Settings.EmulatedKeystroke == 3) {
				PressKey(key1);
				PressKey(key2);
				ReleaseKey(key2);
			}
		}
		return 1; // prevent default
	}

	else if (key->vkCode == VK_LSHIFT) {
		if (!enabled) {
			return CallNextHookEx(hHook, nCode, wParam, lParam);
		}
#if _DEBUG
		printf("[LowLevelKeyboardProc] LShift is %s\n", (wParam == WM_KEYUP) ? "UP" : "DOWN");
#endif

		if (wParam == WM_KEYUP && !keydownCapsLk) {
			keydownLShift = FALSE;
		}

		if (wParam == WM_KEYDOWN && !keydownLShift) {
			keydownLShift = TRUE;

			if (keydownCapsLk) {
				PressKey(VK_CAPITAL);
				ReleaseKey(VK_CAPITAL);
#if _DEBUG
				printf("[LowLevelKeyboardProc] Caps Lock state has been toggled.\n");
#endif
			}
		}
		return 0;
	}

	return CallNextHookEx(hHook, nCode, wParam, lParam);
}
