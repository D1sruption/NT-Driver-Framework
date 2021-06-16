#include "includes.h"

/* IOCTL Codes needed for our driver */

// Request to read virtual user memory (memory of a program) from kernel space
#define IO_READ_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0701 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

// Request to write virtual user memory (memory of a program) from kernel space
#define IO_WRITE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0702 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

// Request to retrieve the process id of csgo process, from kernel space
#define IO_GET_ID_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0703 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

// Request to retrieve the base address of client.dll in csgo.exe from kernel space
#define IO_GET_BASE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0704 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

// offset to local player
#define LOCAL_PLAYER 0x34E4A70
#define OBJECT_MANAGER 0x1C41DB0
#define PLAYER_NAME 0x006C
#define PLAYER_MANA 0x047C

using namespace std;


typedef struct _KERNEL_READ_REQUEST
{
	ULONG ProcessId;

	ULONG Address;
	ULONG Response;
	ULONG Size;

} KERNEL_READ_REQUEST, * PKERNEL_READ_REQUEST;

typedef struct _KERNEL_WRITE_REQUEST
{
	ULONG ProcessId;

	ULONG Address;
	ULONG Value;
	ULONG Size;

} KERNEL_WRITE_REQUEST, * PKERNEL_WRITE_REQUEST;



// interface for our driver
class KernelInterface
{
public:
	HANDLE hDriver; // Handle to driver

					// Initializer
	KernelInterface(LPCSTR RegistryPath)
	{
		hDriver = CreateFileA(RegistryPath, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	}

	template <typename type>
	type ReadProcessMemory(ULONG ProcessId, ULONG ReadAddress,
		SIZE_T Size)
	{
		if (hDriver == INVALID_HANDLE_VALUE)
			return (type)false;

		DWORD Return, Bytes;
		KERNEL_READ_REQUEST ReadRequest;

		ReadRequest.ProcessId = ProcessId;
		ReadRequest.Address = ReadAddress;
		ReadRequest.Size = Size;

		// send code to our driver with the arguments
		if (DeviceIoControl(hDriver, IO_READ_REQUEST, &ReadRequest,
			sizeof(ReadRequest), &ReadRequest, sizeof(ReadRequest), 0, 0))
			return (type)ReadRequest.Response;
		else
			return (type)false;
	}

	bool WriteProcessMemory(ULONG ProcessId, ULONG WriteAddress,
		ULONG WriteValue, SIZE_T WriteSize)
	{
		if (hDriver == INVALID_HANDLE_VALUE)
			return false;
		DWORD Bytes;

		KERNEL_WRITE_REQUEST  WriteRequest;
		WriteRequest.ProcessId = ProcessId;
		WriteRequest.Address = WriteAddress;
		WriteRequest.Value = WriteValue;
		WriteRequest.Size = WriteSize;

		if (DeviceIoControl(hDriver, IO_WRITE_REQUEST, &WriteRequest, sizeof(WriteRequest),
			0, 0, &Bytes, NULL))
			return true;
		else
			return false;
	}

	DWORD GetTargetPid()
	{
		if (hDriver == INVALID_HANDLE_VALUE)
			cout << "INVALID HANDLE VALUE" << endl;
			return false;

		ULONG Id;
		DWORD Bytes;

		if (DeviceIoControl(hDriver, IO_GET_ID_REQUEST, &Id, sizeof(Id),
			&Id, sizeof(Id), &Bytes, NULL))
			return Id;
		else
			return false;
	}

	DWORD GetBaseAddress()
	{
		if (hDriver == INVALID_HANDLE_VALUE)
			cout << "INVALID HANDLE VALUE" << endl;
			return false;

		HWND hwnd = FindWindowA(NULL, "League of Legends (TM) Client");

		DWORD procID;
		GetWindowThreadProcessId(hwnd, &procID);

		char moduleName[] = "League of Legends.exe";
		DWORD baseAddr = dwGetModuleBaseAddy((TCHAR*)moduleName, procID);

		if (baseAddr != NULL)
		{
			return baseAddr;
		}
		else
		{
			return false;
		}

	}

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
};