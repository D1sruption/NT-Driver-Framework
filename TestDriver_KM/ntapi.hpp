#pragma once

#include "ntstructs.hpp"

#ifdef __cplusplus
extern "C" {
#endif
	UCHAR* PsGetProcessImageFileName(PEPROCESS Process);

	NTKERNELAPI PVOID NTAPI PsGetProcessWow64Process(
		_In_ PEPROCESS Process
	);

	NTSYSAPI NTSTATUS NTAPI ObReferenceObjectByName(
		_In_ PUNICODE_STRING ObjectName,
		_In_ ULONG Attributes,
		_In_opt_ PACCESS_STATE AccessState,
		_In_opt_ ACCESS_MASK DesiredAccess,
		_In_ POBJECT_TYPE ObjectType,
		_In_ KPROCESSOR_MODE AccessMode,
		_Inout_opt_ PVOID ParseContext,
		_Out_ PVOID* Object
	);

#ifdef __cplusplus
}
#endif