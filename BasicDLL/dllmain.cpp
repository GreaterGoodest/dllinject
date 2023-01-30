#include <fstream>
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

using namespace std;

extern "C" DWORD Main()
{
    ofstream myfile;
    myfile.open("C:\\temp\\example.txt");
    myfile << "Writing this to a file.\n";
    myfile.close();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD  reason_for_call, LPVOID reserved)
{
    switch (reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Main();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}