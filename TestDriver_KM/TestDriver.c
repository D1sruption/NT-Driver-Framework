#include "includes.h"

// Request to read virtual user memory (memory of a program) from kernel space
#define IO_READ_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0701 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

// Request to write virtual user memory (memory of a program) from kernel space
#define IO_WRITE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0702 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

// Request to retrieve the process id of csgo process, from kernel space
#define IO_GET_ID_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0703 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

// Request to retrieve the base address of league of legends.exe from kernel space
#define IO_GET_BASE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0704 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define IO_GET_IMAGE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0705, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define IMAGE_FILE_NAME 0x450

#define ACTIVE_PROCESS_LINKS_FLINK 0x2f0

PDEVICE_OBJECT pDeviceObject; // our driver object
UNICODE_STRING dev, dos; // Driver registry paths

ULONG ProcID, BaseAddress;

// datatype for read request
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

NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject);
NTSTATUS CreateCall(PDEVICE_OBJECT DeviceObject, PIRP irp);
NTSTATUS CloseCall(PDEVICE_OBJECT DeviceObject, PIRP irp);
ULONG get_process_id_by_name(PEPROCESS start_process, const char* process_name);

PLOAD_IMAGE_NOTIFY_ROUTINE ImageLoadCallback(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo)
{
	//this will print all images...perhaps remove later after debug
	DebugMessage("ImageLoaded: %ls \n", FullImageName->Buffer);

	return STATUS_SUCCESS;
}

NTSTATUS KeReadProcessMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	PSIZE_T Bytes;
	if (NT_SUCCESS(MmCopyVirtualMemory(Process, SourceAddress, PsGetCurrentProcess(),
		TargetAddress, Size, KernelMode, &Bytes)))
		return STATUS_SUCCESS;
	else
		return STATUS_ACCESS_DENIED;
}

NTSTATUS KeWriteProcessMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	PSIZE_T Bytes;
	if (NT_SUCCESS(MmCopyVirtualMemory(PsGetCurrentProcess(), SourceAddress, Process,
		TargetAddress, Size, KernelMode, &Bytes)))
		return STATUS_SUCCESS;
	else
		return STATUS_ACCESS_DENIED;
}

// IOCTL Call Handler function
NTSTATUS IoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	NTSTATUS Status;
	ULONG BytesIO = 0;

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

	// Code received from user space
	ULONG ControlCode = stack->Parameters.DeviceIoControl.IoControlCode;

	if (ControlCode == IO_READ_REQUEST)
	{
		DebugMessage("Received CTL_CODE: %lu \n", ControlCode);
		// Get the input buffer & format it to our struct
		PKERNEL_READ_REQUEST ReadInput = (PKERNEL_READ_REQUEST)Irp->AssociatedIrp.SystemBuffer;
		PKERNEL_READ_REQUEST ReadOutput = (PKERNEL_READ_REQUEST)Irp->AssociatedIrp.SystemBuffer;

		PEPROCESS Process;
		// Get our process
		if (NT_SUCCESS(PsLookupProcessByProcessId(ReadInput->ProcessId, &Process)))
			KeReadProcessMemory(Process, ReadInput->Address,
				&ReadInput->Response, ReadInput->Size);

		DebugMessage("PROCESS_ID:  %lu | ADDRESS: %#010x \n", ReadInput->ProcessId, ReadInput->Address);
		DebugMessage("ADDRESS VALUE: %lu \n", ReadOutput->Response);

		Status = STATUS_SUCCESS;
		BytesIO = sizeof(KERNEL_READ_REQUEST);
	}
	else if (ControlCode == IO_WRITE_REQUEST)
	{
		DebugMessage("Received CTL_CODE: %lu \n", ControlCode);
		// Get the input buffer & format it to our struct
		PKERNEL_WRITE_REQUEST WriteInput = (PKERNEL_WRITE_REQUEST)Irp->AssociatedIrp.SystemBuffer;

		PEPROCESS Process;
		// Get our process
		if (NT_SUCCESS(PsLookupProcessByProcessId(WriteInput->ProcessId, &Process)))
			KeWriteProcessMemory(Process, &WriteInput->Value,
				WriteInput->Address, WriteInput->Size);

		DebugMessage("WRITING_VALUE:  %lu | ADDRESS: %#010x \n", WriteInput->Value, WriteInput->Address);

		Status = STATUS_SUCCESS;
		BytesIO = sizeof(KERNEL_WRITE_REQUEST);
	}
	else if (ControlCode == IO_GET_ID_REQUEST)
	{
		PULONG Output = (PULONG)Irp->AssociatedIrp.SystemBuffer;
		*Output = ProcID;

		DebugMessage("PROCESS ID: %#010x", ProcID);
		Status = STATUS_SUCCESS;
		BytesIO = sizeof(*Output);
	}
	else if (ControlCode == IO_GET_BASE_REQUEST)
	{
		PULONG Output = (PULONG)Irp->AssociatedIrp.SystemBuffer;
		*Output = BaseAddress;

		DebugMessage("BASE ADDRESS: %#010x", BaseAddress);
		Status = STATUS_SUCCESS;
		BytesIO = sizeof(*Output);
	}
	else if (ControlCode == IO_GET_IMAGE)
	{
		//@TODO
	}
	else
	{
		// if the code is unknown
		Status = STATUS_INVALID_PARAMETER;
		BytesIO = 0;
	}

	// Complete the request
	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = BytesIO;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Status;
}

// Driver Entrypoint
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,
	PUNICODE_STRING pRegistryPath)
{
	DebugMessage("Loading Driver...");

	//remove later after debug??
	//PsSetLoadImageNotifyRoutine(ImageLoadCallback);

	//PsSetLoadImageNotifyRoutine(ImageLoadCallback);

	RtlInitUnicodeString(&dev, L"\\Device\\TestDriver");
	RtlInitUnicodeString(&dos, L"\\DosDevices\\TestDriver");

	IoCreateDevice(pDriverObject, 0, &dev, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
	IoCreateSymbolicLink(&dos, &dev);

	pDriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCall;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseCall;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;
	pDriverObject->DriverUnload = UnloadDriver;

	pDeviceObject->Flags |= DO_DIRECT_IO;
	pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	return STATUS_SUCCESS;
}



NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject)
{
	DebugMessage("Unloading Driver...");

	//PsRemoveLoadImageNotifyRoutine(ImageLoadCallback);
	IoDeleteSymbolicLink(&dos);
	IoDeleteDevice(pDriverObject->DeviceObject);

	//PsRemoveLoadImageNotifyRoutine(ImageLoadCallback);

	return STATUS_SUCCESS;
}

NTSTATUS CreateCall(PDEVICE_OBJECT DeviceObject, PIRP irp)
{
	DebugMessage("Created Call Channel!");

	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS CloseCall(PDEVICE_OBJECT DeviceObject, PIRP irp)
{
	DebugMessage("Terminated Call Channel!");

	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

ULONG GET_PROC_ID_BY_NAME(PEPROCESS start_process, const char* process_name)
{
	PLIST_ENTRY active_process_links;
	PEPROCESS current_process = start_process;

	do
	{
		PKPROCESS kproc = (PKPROCESS)current_process;
		PDISPATCHER_HEADER header = (PDISPATCHER_HEADER)kproc;
		LPSTR current_process_name = (LPSTR)((PUCHAR)current_process + IMAGE_FILE_NAME);

		if (header->SignalState == 0 && strcmp(current_process_name, process_name) == 0)
		{
			return (ULONG)PsGetProcessId(current_process);
		}

		active_process_links = (PLIST_ENTRY)((PUCHAR)current_process + ACTIVE_PROCESS_LINKS_FLINK);
		current_process = (PEPROCESS)(active_process_links->Flink);
		current_process = (PEPROCESS)((PUCHAR)current_process - ACTIVE_PROCESS_LINKS_FLINK);

	} while (start_process != current_process);

	return 0;
}

ULONG GET_MODULE_BASE(PEPROCESS process, wchar_t* module_name)
{
	if (!process) { return 0; }

	__try
	{
		PPEB32 peb32 = (PPEB32)PsGetProcessWow64Process(process);
		if (!peb32 || !peb32->Ldr) { return 0; }

		for (PLIST_ENTRY32 plist_entry = (PLIST_ENTRY32)((PPEB_LDR_DATA32)peb32->Ldr)->InLoadOrderModuleList.Flink;
			plist_entry != &((PPEB_LDR_DATA32)peb32->Ldr)->InLoadOrderModuleList;
			plist_entry = (PLIST_ENTRY32)plist_entry->Flink)
		{
			PLDR_DATA_TABLE_ENTRY32 pentry = CONTAINING_RECORD(plist_entry, LDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);

			if (wcscmp((PWCH)pentry->BaseDllName.Buffer, module_name) == 0)
			{
				return pentry->DllBase;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{

	}

	return 0;
}