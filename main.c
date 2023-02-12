#include <Windows.h>
#include <stdio.h>

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
	BOOL ShowMultiInstanceError;
	BOOL AltCapsToDisable;
	BOOL UseSoundIndication;	/// not yet implemented
};

struct Settings Settings = { 1, FALSE, TRUE, FALSE };



int main(int argc, char** argv)
{
	if (importSettingsFromFile(argv[1], &Settings) == -1) { return 1; }

	//printf("%s", argv[0]);
	HANDLE hMutex = CreateMutex(0, 0, "CapsWitch");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		if (Settings.ShowMultiInstanceError == TRUE) {
			MessageBox(NULL,
				L"Another instance of the program is already running!\nThis instance will be terminated.",
				L"CapsWitch", MB_OK | MB_ICONWARNING);
		}
		return 1;
	}

	HINSTANCE hinst = GetModuleHandle(NULL);	// Windows XP compatibility workaround
	hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hinst, 0);
	if (hHook == NULL) {
		MessageBox(NULL,
			L"Error calling \"SetWindowsHookEx(...)\"",
			L"CapsWitch", MB_OK | MB_ICONERROR);
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
	FILE* settingsFile = NULL;
	if (settingsFileName != NULL) {
		fopen_s(&settingsFile, settingsFileName, "r");
	}
	else {
		fopen_s(&settingsFile, "CapsWitch.ini", "r");
	}

	if (settingsFile != NULL) {
#if _DEBUG
		printf("[importSettingsFromFile] settingsFile found, settings import started...\n");
#endif
		while (!feof(settingsFile)) {
			char currString[256];
			fgets(currString, 255, settingsFile);

			if (currString[0] == ';' || currString[0] == '\n') { continue; }	// skip if string is commented or empty
			
			char* ctx = NULL;
			char* settingName = strtok_s(currString, "=", &ctx);
			int settingValue = atoi(strtok_s(NULL, "=", &ctx));

			if (strcmp(settingName, "EmulatedKeystroke") == 0) {
				Settings->EmulatedKeystroke = settingValue;
			}
			else if (strcmp(settingName, "ShowMultiInstanceError") == 0) {
				Settings->ShowMultiInstanceError = settingValue;
			}
			else if (strcmp(settingName, "AltCapsToDisable") == 0) {
				Settings->AltCapsToDisable = settingValue;
			}
			else if (strcmp(settingName, "UseSoundIndication") == 0) {
				Settings->UseSoundIndication = settingValue;
			}
			else {
#if _DEBUG
				printf("[importSettingsFromFile] Ignored unknown setting: %s=%d\n", settingName, settingValue);
#endif
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
			Settings->EmulatedKeystroke, Settings->ShowMultiInstanceError, Settings->AltCapsToDisable, Settings->UseSoundIndication);
#endif
	}

	else {
#if _DEBUG
		printf("[importSettingsFromFile] settingsFile not found, proceeding with default settings...\n");
#endif
		// we only want to throw error if custom settings file was specified, 
		// and continue in portable mode otherwise.
		if (settingsFileName != NULL) {
			int mbox = MessageBox(NULL,
				L"Unable to open specified settings file!\nPress OK to continue, or Cancel to terminate.",
				L"CapsWitch", MB_OKCANCEL | MB_ICONWARNING);

			if (mbox == IDCANCEL) { return -1; }
		}
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
				printf("key1 UP\n");

				// special behaviour for Win+Space keystroke
				// (as the language pop-up is currently opened
				// but Space key is down, we only need to release it)
				if (Settings.EmulatedKeystroke == 3) {
					ReleaseKey(key1);
					//key2Processed = FALSE;
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
				printf("key1 DOWN\n");

				if (key2Processed == TRUE) {
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
			if ((wParam == WM_KEYUP || wParam == WM_SYSKEYUP) && !key1Processed) {
				key2Processed = FALSE;
				printf("key2 UP\n");
			}

			if (!enabled) { return CallNextHookEx(hHook, nCode, wParam, lParam); }

			if (wParam == WM_KEYDOWN && !key2Processed) {
				key2Processed = TRUE;
				printf("key2 DOWN\n");

				if (key1Processed == TRUE) {
					ToggleCapsLockState();
					return 0;
				}
			}
			return 0;
		}
	}

	return CallNextHookEx(hHook, nCode, wParam, lParam);
}
