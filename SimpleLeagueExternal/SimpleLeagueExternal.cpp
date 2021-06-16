// SimpleLeagueExternal.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <string>
#include <vector>
#include <TlHelp32.h>
#include <tchar.h>

using namespace std;

DWORD dwGetModuleBaseAddy(TCHAR* lpszModuleName, DWORD pID) {                        // FUNCTION TO GET THE BASE MODULE ADDRESS IN THIS CASE LEAGUE.EXE WHICH IS DYNAMICALLY ALLOCATED IN RAM

    DWORD dwModuleBaseAddress = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);
    MODULEENTRY32 ModuleEntry32 = { 0 };
    ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hSnapshot, &ModuleEntry32))
    {
        do {
            if (_tcscmp(ModuleEntry32.szModule, lpszModuleName) == 0)
            {
                dwModuleBaseAddress = (DWORD)ModuleEntry32.modBaseAddr;
                break;
            }
        } while (Module32Next(hSnapshot, &ModuleEntry32));
    }
    CloseHandle(hSnapshot);
    return dwModuleBaseAddress;
}


int main()
{
    cout << "Attempting to get window..." << endl;

    HWND hwnd = FindWindowA(NULL, "League of Legends (TM) Client");

    if (hwnd == NULL) {
        cout << "Error getting handle!" << endl;

    }
    else {
        DWORD procID;
        GetWindowThreadProcessId(hwnd, &procID);

        char moduleName[] = "League of Legends.exe";
        DWORD baseAddr = dwGetModuleBaseAddy(moduleName, procID);

        HANDLE handle = OpenProcess(PROCESS_VM_READ, false, procID);
        if (handle == NULL) {
            cout << "Error opening process handle!" << endl;
        }
        else {
            char pName[16];
            char pChampName[16];
            float pHP = 0;
            float pMana = 0;
            float pCurrentGold = 0;
            float pEarnedGold = 0;
            DWORD LocalPlayer;

            cout << "[+] Process ID: " << procID << endl;
            cout << "[+] Base Address: " << hex << baseAddr << endl;


            DWORD oLocalPlayer = (DWORD)0x34F489C;
            DWORD pNameAddr = (DWORD)0x006C; //UPDATE
            DWORD pHPAddr = (DWORD)0x0F88;
            DWORD pChampAddr = (DWORD)0x3594;
            DWORD pManaAddr = (DWORD)0x047C;
            DWORD pCurrentGoldAddr = (DWORD)0x1CC8;
            DWORD pEarnedGoldAddr = (DWORD)0x1CD8;

            ReadProcessMemory(handle, (void*)(baseAddr + oLocalPlayer), &LocalPlayer, sizeof(LocalPlayer), 0);
            cout << "[+] Local Player: " << hex << LocalPlayer << endl;
            ReadProcessMemory(handle, (void*)(LocalPlayer + pHPAddr), &pHP, 4, 0);
            ReadProcessMemory(handle, (void*)(LocalPlayer + pManaAddr), &pMana, 4, 0);
            ReadProcessMemory(handle, (void*)(LocalPlayer + pNameAddr), &pName, sizeof(pName), 0);
            ReadProcessMemory(handle, (void*)(LocalPlayer + pChampAddr), &pChampName, sizeof(pChampName), 0);
            cout << "Player Name: " << pName << " | HP: " << pHP << " | MANA: " << pMana << " | Champion: " << pChampName << endl;

            //while (true) {
            //    //ReadProcessMemory(handle, pNameAddr, &pName, sizeof(pName), 0);
            //    ReadProcessMemory(handle, (void*)*(baseAddr + oLocalPlayer), &LocalPlayer, sizeof(LocalPlayer), 0);
            //    cout << "Local Player: " << LocalPlayer << endl;
            //    //ReadProcessMemory(handle, pManaAddr, &pMana, 4, 0);
            //    //ReadProcessMemory(handle, pCurrentGoldAddr, &pCurrentGold, 5, 0);
            //    //ReadProcessMemory(handle, pEarnedGoldAddr, &pEarnedGold, 5, 0);

            //    //cout << "Player Name: " << pName << " | Player HP: " << pHP << " | Player Mana: " << pMana << " | Player Current Gold: " << pCurrentGold << " | Player Earned Gold: " << pEarnedGold << endl;

            //    Sleep(1000);
            //}

        }

    }
}


