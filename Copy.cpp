#include <ntifs.h>
#include <windef.h>
#include <cstdint>
#include <cstddef>
#include <ntimage.h>
#include <string>


#define COMMAND_MAGIC 0xCAFEBABECAFEBABE

#define SECTION_SIZE 1024
#define SECTION_NAME L"\\BaseNamedObjects\\MySection"

#define DebugPrint(fmt, ...) DbgPrintEx(0, 0, "[memrw] " fmt, ##__VA_ARGS__)

typedef enum _COMMAND_TYPE {
    write,
    read,
    attach,
    detach,
    getBase,
    reply,
} COMMAND_TYPE;

typedef struct _COMMAND {
    ULONG64 magic;
    COMMAND_TYPE type;
    ULONG64 arg1;
    ULONG64 arg2;
    ULONG64 arg3;
    ULONG64 dataBuffer;
} COMMAND, * PCOMMAND;

PVOID sectionAddress;

extern "C" {
    NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(PEPROCESS Process);
    NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName, PDRIVER_INITIALIZE InitializationFunction);
    NTKERNELAPI NTSTATUS MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess,
                                             PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, 
                                             PSIZE_T ReturnSize);
}

#include "rw.h"


VOID ThreadFunction(PVOID sectionHandle) {
    SIZE_T viewSize = SECTION_SIZE;
    NTSTATUS viewOfSectionStatus = ZwMapViewOfSection(sectionHandle, ZwCurrentProcess(), &sectionAddress, 0, 0, NULL, &viewSize, ViewUnmap, 0, PAGE_READWRITE);
    if (!NT_SUCCESS(viewOfSectionStatus))
    {
        DebugPrint("ZwMapViewOfSection failed: 0x%X\n", viewOfSectionStatus);
        ZwClose(sectionHandle);
        PsTerminateSystemThread(viewOfSectionStatus);
    }
    
    char* data = (char*)sectionAddress;
    RtlZeroMemory(data, SECTION_SIZE);
    PCOMMAND action = (PCOMMAND)data;

    ULONG pid = NULL, size;
    uint64_t dest, src;
    NTSTATUS status = STATUS_SUCCESS;
    

    PEPROCESS process = NULL;
    

	LARGE_INTEGER interval;
	interval.QuadPart = -10000LL;

    //DebugPrint("buffer: %s", data);
    while (true) {
	KeDelayExecutionThread(KernelMode, FALSE, &interval);
	if(action->magic == COMMAND_MAGIC) {
        switch (action->type) {
        case write:
            if (pid == NULL) break;
            src = action->arg1; dest = action->arg2; size = action->arg3;
            status = WriteToUserMemory((HANDLE)pid, &src, (PVOID)dest, size, process);
            if (!NT_SUCCESS(status)) { DebugPrint("WriteToUserMemory Failed: 0x%X\n", status); RtlZeroMemory(data, SECTION_SIZE); continue; }
            RtlZeroMemory(data, SECTION_SIZE);
            break;
        case read:
            if (pid == NULL) break;
            dest = action->arg1; size = action->arg2;
	    DebugPrint("dest: %p, size: %d\n", dest, size);
            status = ReadFromUserMemory((HANDLE)pid, (PVOID)dest, &src, size, process);
            if (!NT_SUCCESS(status)) { DebugPrint("ReadFromUserMemory Failed: 0x%X\nOffending address: %p\n", status, dest); RtlZeroMemory(data, SECTION_SIZE);}
            action->type = reply;
            action->dataBuffer = src;
	    DebugPrint("result: %llu\n", src);
            RtlCopyMemory(data, action, SECTION_SIZE);
            break;
        case attach:
            pid = action->arg1;
            status = PsLookupProcessByProcessId((HANDLE)pid, &process);
            if (!NT_SUCCESS(status)) { DebugPrint("PsLookupProcessByProcessId Failed, does %lu exist?\n", pid); RtlZeroMemory(data, SECTION_SIZE); pid = NULL; continue; }
            DebugPrint("Successfully attched to pid %lu\n", pid);
            RtlZeroMemory(data, SECTION_SIZE);
            break;
	}
	dest = 0, src = 0;
        }

        /*char* splitBuffer[SECTION_SIZE];
        split_by_space(data, splitBuffer, SECTION_SIZE);
        if (strcmp(splitBuffer[0], "USER") != 0 && sizeof(splitBuffer[0]) > 3) {
            if (strcmp(splitBuffer[0], "write") == 0) { //write memory
                if (pid == NULL) continue;
                src = NULL; dest = NULL;
                status = StringToInt64(splitBuffer[1], &src);
                status = StringToInt64(splitBuffer[2], &dest);
                status = RtlCharToInteger(splitBuffer[3], 10, &size);
                
                if (NT_SUCCESS(status)) status = WriteToUserMemory((HANDLE)pid, &src, (PVOID)dest, size, process);
                if (!NT_SUCCESS(status)) { DebugPrint("WriteToUserMemory Failed: 0x%X\n", status); RtlZeroMemory(data, SECTION_SIZE); continue; }
                RtlZeroMemory(data, SECTION_SIZE);
            }
            if (strcmp(splitBuffer[0], "read") == 0) { //read memory
                if (pid == NULL) continue;
                src = NULL; dest = NULL;
                status = StringToInt64(splitBuffer[1], &dest);
                status = RtlCharToInteger(splitBuffer[2], 10, &size);
                
                if (NT_SUCCESS(status)) status = ReadFromUserMemory((HANDLE)pid, (PVOID)dest, &src, size, process);
                if (!NT_SUCCESS(status)) { DebugPrint("ReadFromUserMemory Failed: 0x%X\n", status); RtlZeroMemory(data, SECTION_SIZE); continue; }

                char response[SECTION_SIZE];
                format_user_int64(src, response, SECTION_SIZE);
                DebugPrint("%s", response);
                RtlCopyMemory(data, response, SECTION_SIZE);
            }
            if (strcmp(splitBuffer[0], "attach") == 0) { //attach process pid
                status = RtlCharToInteger(splitBuffer[1], 10, &pid);
                status = PsLookupProcessByProcessId((HANDLE)pid, &process);
                if (!NT_SUCCESS(status)) { DebugPrint("PsLookupProcessByProcessId Failed, does %lu exist?\n", pid); RtlZeroMemory(data, SECTION_SIZE); pid = NULL; continue; }
                DebugPrint("Successfully attched to pid %lu\n", pid);
                RtlZeroMemory(data, SECTION_SIZE);
            }
            if (strcmp(splitBuffer[0], "detach") == 0) { //detach process pid
                pid == NULL, process == NULL;
                DebugPrint("Successfully detached from pid %lu\n", pid);
                RtlZeroMemory(data, SECTION_SIZE);
            }
            if (strcmp(splitBuffer[0], "getbase") == 0) { //get base address of process pid
                if (pid == NULL) continue;

                PVOID BaseAddress = PsGetProcessSectionBaseAddress(process);
                DebugPrint("Base address %lu\n", BaseAddress);
                char response[SECTION_SIZE];
                format_user_int64((LONG64)BaseAddress, response, SECTION_SIZE);
                DebugPrint("%s\n", response);
                RtlCopyMemory(data, response, SECTION_SIZE);
            }*/
        }


    }

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{
    UNREFERENCED_PARAMETER(pRegistryPath);
    DebugPrint("r/w module successfully loaded!\n");
   
    HANDLE sectionHandle;
    OBJECT_ATTRIBUTES attr;
    UNICODE_STRING sectionName;
    IO_STATUS_BLOCK ioStatus;
    LARGE_INTEGER sectionSize;

    RtlInitUnicodeString(&sectionName, SECTION_NAME);
    InitializeObjectAttributes(&attr, &sectionName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    sectionSize.QuadPart = SECTION_SIZE;
    NTSTATUS createSectionStatus = ZwCreateSection(&sectionHandle, SECTION_ALL_ACCESS, &attr, &sectionSize, PAGE_READWRITE, SEC_COMMIT, NULL);

    if (!NT_SUCCESS(createSectionStatus)) {
        DebugPrint("ZwCreateSection failed: 0x%X\n", createSectionStatus);
        return createSectionStatus;
    }


    HANDLE hThread;
    NTSTATUS threadStatus = PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS, NULL, NULL, NULL, ThreadFunction, sectionHandle);

    if (!NT_SUCCESS(threadStatus)) {
        DebugPrint("PsCreateSystemThread failed: 0x%X\n", threadStatus);
        return threadStatus;
    }
    return STATUS_SUCCESS;
}
