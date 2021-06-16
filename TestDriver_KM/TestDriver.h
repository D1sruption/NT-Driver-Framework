#pragma once

#include "includes.h"
NTSTATUS MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID
	SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T
	BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize);
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath);

NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject);