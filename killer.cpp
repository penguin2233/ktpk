#include "pch.h"
#include "killer.h"

// APPLICATION DEFINED MESSAGES
// 0x0420 = start killer
// 0x0421 = exit killer
// 0x0422 = pause killer
// 0x0423 = callback message for events regarding the notification icon (unused in this file)

DWORD WINAPI killerMain(LPVOID lpParam) {
	// setup message queue
	MSG message;
	PeekMessage(&message, NULL, 0, 0, PM_NOREMOVE);

	// wait for start or exit
	while (true) {
		BOOL getMsgRet = GetMessage(&message, NULL, 0x0420, 0x421);
		if (getMsgRet == 0 || getMsgRet == -1) {
			return -1;
		}
		else {
			if (message.message == 0x0420) {
				break;
			}
			if (message.message == 0x0421) {
				return 0;
			}
		}
	}

	// retrieve the hitlist
	std::vector<std::string> hitlistTL; // thread local hitlist
	hitlistTL = *(std::vector<std::string>*)lpParam;

	// main loop
	while (true) {
		if (PeekMessage(&message, NULL, 0x0421, 0x0422, PM_NOREMOVE) != 0) {
			BOOL getMsgRet = GetMessage(&message, NULL, 0x0421, 0x0422);
			if (getMsgRet == 0 || getMsgRet == -1) {
				return -1;
			}
			switch (message.message) {
			case 0x0421: { // exit
				return 0;
			}
			case 0x0422: { // pause
				BOOL getMsgRet = GetMessage(&message, NULL, 0x0420, 0x420);
				if (getMsgRet == 0 || getMsgRet == -1) {
					return -1;
				}
				hitlistTL = *(std::vector<std::string>*)lpParam; // hitlist may have been updated, so get it again
				GetAsyncKeyState(VK_SCROLL); // clear key state
			}
		}
		} else {
			if (GetAsyncKeyState(VK_SCROLL) != 0) { // hit once
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(VK_SCROLL) != 0) { // hit twice (within 1 second of the 1st hit), so start the killing
					std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
					std::vector<DWORD> PIDs;

					// get a snapshot of the current processes
					HANDLE processSnapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
					PROCESSENTRY32 pe32;
					pe32.dwSize = sizeof(PROCESSENTRY32);
					bool searching = true;
					while (searching) {
						for (size_t i = 0; i < hitlistTL.size(); i++) {
							if (converter.to_bytes(pe32.szExeFile) == hitlistTL[i].c_str()) {
								PIDs.push_back(pe32.th32ProcessID);
							}
						}
						if (Process32Next(processSnapshotHandle, &pe32) == FALSE) {
							searching = false; // done searching
						}
					}

					// now kill those PIDs
					for (size_t i = 0; i < PIDs.size(); i++) {
						HANDLE processHandle = OpenProcess(PROCESS_TERMINATE, FALSE, PIDs[i]);
						TerminateProcess(processHandle, 0);
					}
				}
			}
		}
	}
	return 0;
}