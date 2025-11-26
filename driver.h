constexpr DWORD attach  = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); 
constexpr DWORD read  = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr DWORD write = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); 
//constexpr DWORD test = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); 

struct info_t { 
	uint64_t pid = 0;
	uintptr_t  dest = 0;
	uintptr_t  src = 0;
	uint64_t size = 0;
	uint64_t return_size = 0;
};

HANDLE hDriver;

void initDriver() {
    std::string drv_name = "\\\\.\\interface";
    hDriver = CreateFileA(drv_name.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if(hDriver == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open device: " << GetLastError() << std::endl;
            exit(1);
    } else {
        std::cout << "Successfully opened handle to driver @ " << drv_name << std::endl;
    }
}

