#pragma once



NTSTATUS WriteToUserMemory(HANDLE pid, PVOID src, PVOID dest, SIZE_T size, PEPROCESS process)
{
    //PEPROCESS process;
    KAPC_STATE apcState;
    SIZE_T bytes;
    NTSTATUS status;


    __try {
        KeStackAttachProcess(process, &apcState);
        
        status = MmCopyVirtualMemory(PsGetCurrentProcess(), src, process, dest, size, KernelMode, &bytes);
    }
    __finally {
        KeUnstackDetachProcess(&apcState);
    }

    return status;
}


NTSTATUS ReadFromUserMemory(HANDLE pid, PVOID src, PVOID dest, SIZE_T size, PEPROCESS process)
{
    //PEPROCESS process;
    KAPC_STATE apcState;
    SIZE_T bytes;
    NTSTATUS status;


    

    __try {
        KeStackAttachProcess(process, &apcState);

        status = MmCopyVirtualMemory(process, src, PsGetCurrentProcess(), dest, size, KernelMode, &bytes);
    }
    __finally {
        KeUnstackDetachProcess(&apcState);
    }

    return status;
}