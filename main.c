#include <Windows.h>
#if _DEBUG
#include <stdio.h>
#endif

typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

void PressKey(int keyCode);
void ReleaseKey(int keyCode);
void ToggleCapsLockState();
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
HHOOK hHook;

BOOL enabled = TRUE;
WPARAM key1 = VK_MENU, key2 = VK_LSHIFT;
BOOL key1Processed = FALSE, key2Processed = FALSE;



struct Settings
{
	// see comments in original "CapsWitch.ini" file
	// (available on GitHub) for details
	BYTE EmulatedKeystroke;
	BOOL HideMultiInstanceError;
	BOOL AltCapsToDisable;
	BOOL UseSoundIndication;	/// not implemented yet
};

struct Settings Settings = { 1, TRUE, TRUE, TRUE };



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

	HINSTANCE hinst = GetModuleHandle(NULL);	// Windows XP compatibility workaround
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



int importSettingsFromFile(char* settingsFileName, struct Settings* Settings)
{
	// use default name if arguments didn't provide any
	if (settingsFileName == NULL) {
		settingsFileName = "CapsWitch.ini";
	}
#if _DEBUG
	printf("[importSettingsFromFile] Trying to open %s...\n", settingsFileName);
#endif
	HANDLE settingsFile = CreateFile(settingsFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (settingsFile == INVALID_HANDLE_VALUE) {
#if _DEBUG
		printf("[importSettingsFromFile] settingsFile not found, proceeding with default settings...\n");
#endif
		// we only want to throw error if custom settings file was specified, 
		// and continue in portable mode otherwise.
		if (settingsFileName != "CapsWitch.ini") {
			int mbox = MessageBox(NULL,
				"Unable to open specified settings file!\n\nPress OK to continue, or Cancel to terminate.",
				"CapsWitch", MB_OKCANCEL | MB_ICONWARNING);

			if (mbox == IDCANCEL) { return -1; }
		}
		return 0;
	}

	else {
#if _DEBUG
		printf("[importSettingsFromFile] settingsFile found, settings import started...\n");
#endif
		// read the first 1KB of data of config file
		// (terrible solution, but should be enough for a long time)
		char currString[1024]; int currStringLen;
		ReadFile(settingsFile, currString, 1023, &currStringLen, NULL);
		CloseHandle(settingsFile);

		// \n and = are required to make sure this is not a part of some comment!
		char availableSettings[][32] = {
			"\nEmulatedKeystroke=", "\nHideMultiInstanceError=", "\nAltCapsToDisable=", "\nUseSoundIndication="
		};

		for (size_t i = 0; i < sizeof(availableSettings) / sizeof(availableSettings)[0]; i++) {
			char* ptr = strstr(currString, availableSettings[i]);
			if (ptr != NULL) {
				ptr = strstr(ptr, "=");

				if (ptr[1]-'0' >= 0 && ptr[1]-'0' <= 9) {	// validate value to prevent errors
				switch (i) {
					case 0:
						Settings->EmulatedKeystroke = ptr[1]-'0'; break;

					case 1:
						Settings->HideMultiInstanceError = ptr[1]-'0'; break;

					case 2:
						Settings->AltCapsToDisable = ptr[1]-'0'; break;

					case 3:
						Settings->UseSoundIndication = ptr[1]-'0'; break;
				}
				}
			}
		}

		// now, change the keys we want to look for
		// to match the given "EmulatedKeystroke" setting
		switch (Settings->EmulatedKeystroke) {
			case 2:
				key1 = VK_CONTROL, key2 = VK_SHIFT; break;

			case 3:
				key1 = VK_LWIN, key2 = VK_SPACE; break;

			default:
				key1 = VK_LMENU, key2 = VK_SHIFT; break;
		}

#if _DEBUG
		printf("[importSettingsFromFile] Settings import finished, values: %d %d %d %d\n",
			Settings->EmulatedKeystroke, Settings->HideMultiInstanceError, Settings->AltCapsToDisable, Settings->UseSoundIndication);
#endif
		return 0;
	}
}



void PressKey(WPARAM keyCode)
{
	keybd_event(keyCode, 0, 0, 0);
}

void ReleaseKey(WPARAM keyCode)
{
	keybd_event(keyCode, 0, KEYEVENTF_KEYUP, 0);
}

void ToggleCapsLockState()
{
	PressKey(VK_CAPITAL);
	ReleaseKey(VK_CAPITAL);
#if _DEBUG
	printf("[ToggleCapsLockState] CapsLock state has been toggled\n");
#endif
}



LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT* key = (KBDLLHOOKSTRUCT*)lParam;
	if (nCode == HC_ACTION && !(key->flags & LLKHF_INJECTED)) {
		if (key->vkCode == VK_CAPITAL) {
			if (Settings.AltCapsToDisable == TRUE && wParam == WM_SYSKEYDOWN && !key1Processed) {
				key1Processed = TRUE;
				enabled = !enabled;
#if _DEBUG
				printf("[LowLevelKeyboardProc] CapsWitch has been %s\n", enabled ? "enabled" : "disabled");
#endif
				return 1;
			}

			if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
				key1Processed = FALSE;
#if _DEBUG
				printf("key1 UP\n");
#endif

				// special behaviour for Win+Space keystroke
				// (as the language pop-up is currently opened
				// but Space key is down, we only need to release it)
				if (Settings.EmulatedKeystroke == 3) {
					ReleaseKey(key1);
				}

				if (enabled) {
					if (!key2Processed) {
						if (Settings.EmulatedKeystroke != 3) {
							PressKey(key1);
							PressKey(key2);
							ReleaseKey(key1);
							ReleaseKey(key2);
						}
					}
					else { key2Processed = FALSE; }
				}
			}

			if (!enabled) { return CallNextHookEx(hHook, nCode, wParam, lParam); }

			if (wParam == WM_KEYDOWN && !key1Processed) {
				key1Processed = TRUE;
#if _DEBUG
				printf("key1 DOWN\n");
#endif

				if (key2Processed) {
					ToggleCapsLockState();
					return 1;
				}

				// special behaviour for Win+Space keystroke
				// (we want language pop-up to show up,
				// but won't get closed until CapsLock is released)
				else if (Settings.EmulatedKeystroke == 3) {
					PressKey(key1);
					PressKey(key2);
					ReleaseKey(key2);
				}
			}
			return 1;
		}

		else if (key->vkCode == VK_LSHIFT) {
			if (!enabled) { return CallNextHookEx(hHook, nCode, wParam, lParam); }

			if ((wParam == WM_KEYUP || wParam == WM_SYSKEYUP) && !key1Processed) {
				key2Processed = FALSE;
#if _DEBUG
				printf("key2 UP\n");
#endif
			}

			if (wParam == WM_KEYDOWN && !key2Processed) {
				key2Processed = TRUE;
#if _DEBUG
				printf("key2 DOWN\n");
#endif

				if (key1Processed) {
					ToggleCapsLockState();
					return 0;
				}
			}
			return 0;
		}
	}

	return CallNextHookEx(hHook, nCode, wParam, lParam);
}
