#pragma once

#pragma warning( disable : 4996)
#include "includes.h"


using namespace std;

int main()
{
    string windowName;
    cout << "Enter Window Name: ";
    cin >> windowName;
    

    LPTSTR longstring = new TCHAR[windowName.size() + 1];
    strcpy(longstring, windowName.c_str());

    cout << "\nAttempting to get " << longstring << endl;


	KernelInterface Driver("\\\\.\\TestDriver");

    //HWND hwnd = FindWindowA(NULL, "League of Legends (TM) Client");

    HWND hwnd = FindWindowA(NULL, (LPCSTR)longstring);

    if (hwnd == NULL) {
        cout << "Error getting handle!" << endl;
    }
    else {
        //get procID from handle here
        Driver.GetTargetPid();
    }

	return 0;
}
