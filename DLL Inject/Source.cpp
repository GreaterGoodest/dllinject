#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>

using namespace std;

const int kb = 1024;


DWORD MyGetProcessId(LPCTSTR ProcessName) // non-conflicting function name
{
	PROCESSENTRY32 pt;
	HANDLE hsnap = CreateToolhelp32Snapshot(0x2, 0);
	pt.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hsnap, &pt)) { // must call this first
		do {
			if (!lstrcmpi((LPCWSTR) pt.szExeFile, ProcessName)) {
				CloseHandle(hsnap);
				return pt.th32ProcessID;
			}
		} while (Process32Next(hsnap, &pt));
	}
	CloseHandle(hsnap); // close handle on failure
	return 0;
}

int main(int argc, char **argv)
{
	if (argc > 1 && strncmp(argv[1], "-h", 2) == 0)
	{
		puts("Usage: DLL_Inject.exe [-i InjectedDLL] [-t targetProcessName]");
		return 0;
	}

	LPCSTR DllPath = "C:\\temp\\boom.dll"; // The Path to our DLL

	if (argc > 2 && strncmp(argv[1], "-i", 2) == 0)
	{
		DllPath = argv[2];
	} else if (argc > 4 && strncmp(argv[3], "-i", 2) == 0)
	{
		DllPath = argv[4];
	}
	cout << "source dll: " << DllPath << endl;

	const wchar_t *procName = L"notepad++.exe";

	LPWSTR cliW = GetCommandLineW();
	LPWSTR *wargv = CommandLineToArgvW(cliW, &argc);
	if (argc > 2 && strncmp(argv[1], "-t", 2) == 0)
	{
		procName = wargv[2];
	} else if (argc > 4 && strncmp(argv[3], "-t", 2) == 0)
	{
		procName = wargv[4];
	}
	wcout << "injecting into: " << procName << endl;

	DWORD procID = MyGetProcessId(procName); // A 32-bit unsigned integer, DWORDS are mostly used to store Hexadecimal Addresses
	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID); // Opening the Process with All Access

	// Allocate memory for the dllpath in the target process, length of the path string + null terminator
	LPVOID pDllPath = VirtualAllocEx(handle, 0, strlen(DllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
	if (!pDllPath)
	{
		cout << "Failed to alloc dll path" << endl;
		return 1;
	}

	// Write the path to the address of the memory we just allocated in the target process
	WriteProcessMemory(handle, pDllPath, (LPVOID)DllPath, strlen(DllPath) + 1, 0);
	cout << "handle: " << handle << endl;

	HMODULE kernel32 = GetModuleHandleA("Kernel32.dll");
	if (!kernel32)
	{
		cout << "Failed to find kernel32" << endl;
		return 1;
	}
	FARPROC lla = GetProcAddress(kernel32, "LoadLibraryA");
	if (!lla)
	{
		cout << "Failed to find lla" << endl;
		return 1;
	}

	// Create a Remote Thread in the target process which calls LoadLibraryA as our dllpath as an argument -> program loads our dll
	HANDLE hLoadThread = CreateRemoteThread(handle, 0, 0,
		(LPTHREAD_START_ROUTINE)lla, pDllPath, 0, 0);

	if (!hLoadThread)
	{
		cout << "Injection failed" << endl;
		return 1;
	}

	WaitForSingleObject(hLoadThread, INFINITE); // Wait for the execution of our loader thread to finish

	cout << "Dll path allocated at: " << hex << pDllPath << endl;
	cin.get();

	return 0;
}