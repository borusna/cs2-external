#ifdef DRIVER 

void attachProcess(DWORD process_id) {
	    info_t io_info;
        io_info.pid = process_id;

        DeviceIoControl(hDriver, attach, &io_info, sizeof(io_info), &io_info, sizeof(io_info), nullptr, nullptr);
        //std::cout << "+ into driver attached" << std::endl; // debug
}

#endif

template<typename T>
T readvm(HANDLE handle, uintptr_t address) {
    T buffer = {};
    #ifndef DRIVER

    //SIZE_T bytesRead;
    ReadProcessMemory(handle, reinterpret_cast<LPCVOID>(address), &buffer, sizeof(T), NULL);
    //buffer = *reinterpret_cast<T*>(address);
    #else
	info_t io_info;
	io_info.dest = address;
	io_info.src = (uintptr_t)&buffer;
	io_info.size = sizeof(T);
	DeviceIoControl(hDriver, read, &io_info, sizeof(io_info), &io_info, sizeof(io_info), nullptr, nullptr);
    #endif

    return buffer;

}

template<typename T>
bool writevm(HANDLE handle, uintptr_t address, const T& data) {
    #ifndef DRIVER

    SIZE_T bytesWritten;
    if (!WriteProcessMemory(handle, reinterpret_cast<LPVOID>(address), &data, sizeof(T), &bytesWritten)) return false;

    return bytesWritten == sizeof(T);

    #else
    info_t io_info;
	io_info.dest = address;
	io_info.src = (uintptr_t)&data;
	io_info.size = sizeof(T);
	DeviceIoControl(hDriver, write, &io_info, sizeof(io_info), &io_info, sizeof(io_info), nullptr, nullptr);
    #endif

    return true;
}

template<typename T>
T customMax(T a, T b) {
    return (a > b) ? a : b;
}

bool StopService(const std::wstring& serviceName) {
    SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!hSCManager) return false;

    SC_HANDLE hService = OpenServiceW(hSCManager, serviceName.c_str(), SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (!hService) {
        CloseServiceHandle(hSCManager);
        return false;
    }

    SERVICE_STATUS status;
    if (!ControlService(hService, SERVICE_CONTROL_STOP, &status)) {
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return false;
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return true;
}

DWORD getPID(const std::wstring& processName) {
    DWORD processId = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32W);
        if (Process32FirstW(snapshot, &processEntry)) {
            do {
                if (processName == processEntry.szExeFile) {
                    processId = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &processEntry));
        }
        CloseHandle(snapshot);
    }
    return processId;
}

uintptr_t getModuleBase(DWORD processId, const std::wstring& moduleName) {
    uintptr_t baseAddress = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if (snapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W moduleEntry;
        moduleEntry.dwSize = sizeof(MODULEENTRY32W);
        if (Module32FirstW(snapshot, &moduleEntry)) {
            do {
                if (moduleName == moduleEntry.szModule) {
                    baseAddress = reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);
                    break;
                }
            } while (Module32NextW(snapshot, &moduleEntry));
        }
        CloseHandle(snapshot);
    }
    return baseAddress;
}

uintptr_t getProcessBase(DWORD processId) {
    uintptr_t processBaseAddress = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if (snapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W moduleEntry;
        moduleEntry.dwSize = sizeof(MODULEENTRY32W);

        if (Module32FirstW(snapshot, &moduleEntry)) {
            processBaseAddress = reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);
        }
        CloseHandle(snapshot);
    }
    return processBaseAddress;
}

bool WorldToScreen(const struct Vector3 pos, Vector2* out, const struct view_matrix_t matrix, int SCREEN_WIDTH, int SCREEN_HEIGHT) {
	Vector3 temp;
	float _x = matrix.matrix[0] * pos.x + matrix.matrix[1] * pos.y + matrix.matrix[2] * pos.z + matrix.matrix[3];
	float _y = matrix.matrix[4] * pos.x + matrix.matrix[5] * pos.y + matrix.matrix[6] * pos.z + matrix.matrix[7];
	temp.z = matrix.matrix[12] * pos.x + matrix.matrix[13] * pos.y + matrix.matrix[14] * pos.z + matrix.matrix[15];
    if (temp.z < 0.0001f) return false;

	_x *= 1.f / temp.z;
	_y *= 1.f / temp.z;

    temp.x = (SCREEN_WIDTH * .5f);
    temp.y = (SCREEN_HEIGHT * .5f);

	temp.x += 0.5f * _x * SCREEN_WIDTH + 0.5f;
	temp.y -= 0.5f * _y * SCREEN_HEIGHT + 0.5f;
    if (temp.x < 0 || temp.x > SCREEN_WIDTH || temp.y < 0 || temp.y > SCREEN_HEIGHT) return false;

    out->x = temp.x;
    out->y = temp.y;

	return true;
}

template <size_t N>
std::string atos(const std::array<char, N>& arr) {
    return std::string(arr.data(), strnlen(arr.data(), N));
}

float distance3D(const Vector3& pos1, const Vector3& pos2) {
    float dx = pos1.x - pos2.x;
    float dy = pos1.y - pos2.y;
    float dz = pos1.z - pos2.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

void PopulatecontrollerCache() {
    controllerCache.clear();  
    for (int i = 0; i <= 64; i++) {
        ptr controller = readvm<ptr>(process, listEntry + (i * 0x78));
        if (!controller) continue;
        controllerCache.push_back(controller);
    }
}

void update() {
    player = readvm<ptr>(process, client + offsets::dwLocalPlayerPawn);
    entityList = readvm<ptr>(process, client + offsets::dwEntityList);
	listEntry = readvm<ptr>(process, entityList + 0x10);
    playerTeam = readvm<int>(process, player + offsets::m_iTeamNum);

    PopulatecontrollerCache();
}

inline void press(WORD key) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;
    input.ki.wScan = 0;
    input.ki.dwFlags = 0;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    SendInput(1, &input, sizeof(INPUT));
    Sleep(1);
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

void keyDown(WORD vk) {
    INPUT i = { INPUT_KEYBOARD };
    i.ki.wVk = vk;
    i.ki.dwFlags = 0;          // key down
    SendInput(1, &i, sizeof(i));
}

void keyUp(WORD vk) {
    INPUT i = { INPUT_KEYBOARD };
    i.ki.wVk = vk;
    i.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &i, sizeof(i));
}

inline void clickMouse(WORD mouseFlagDown, WORD mouseFlagUp) {
    INPUT input[2] = {};

    input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = mouseFlagDown;

    input[1].type = INPUT_MOUSE;
    input[1].mi.dwFlags = mouseFlagUp;

    SendInput(2, input, sizeof(INPUT));
}

void LoadConfig() {
    std::ifstream configFile("config.ini");
    if (!configFile.is_open()) return;

    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream is_line(line);
        std::string key;
        if (std::getline(is_line, key, '=')) {
            std::string value;
            if (std::getline(is_line, value)) {
                key.erase(0, key.find_first_not_of(" \t\n\r"));
                key.erase(key.find_last_not_of(" \t\n\r") + 1);
                value.erase(0, value.find_first_not_of(" \t\n\r"));
                value.erase(value.find_last_not_of(" \t\n\r") + 1);

                try {
                    if (key == "Font Size") {
                        gFontSize = std::stoi(value);
                    } else if (key == "Text Color") {
                        int r, g, b;
                        if (sscanf_s(value.c_str(), "%d, %d, %d", &r, &g, &b) == 3) {
                            textColor = RGB(r, g, b);
                        } else {
                            std::cerr << "Error parsing Text Color: " << value << std::endl;
                        }
                    } else if (key == "Font Family") {
                        fontFamily = std::wstring(value.begin(), value.end());
                    } else if (key == "ESPKey") {
                        if (value.find("0x") == 0) {
                            espKey = std::stoi(value.substr(2), nullptr, 16);
                        } else {
                            std::cerr << "Invalid ESPKey value: " << value << std::endl;
                        }
                    } else if (key == "ESP") {
                        if (value == "Toggle") {
                            espMode = ESP_TOGGLE_MODE;
                        } else if (value == "Hold") {
                            espMode = ESP_HOLD_MODE;
                        } else {
                            std::cerr << "Invalid ESP mode: " << value << std::endl;
                        }
					} else if (key == "ESPKey") { // esp key
                        if (value.find("0x") == 0) {
                            espKey = std::stoi(value.substr(2), nullptr, 16);
                        } else {
                            std::cerr << "Invalid ESPKey value: " << value << std::endl;
                        }
					} else if (key == "TriggerKey") {
                        if (value.find("0x") == 0) {
                            triggerKey = std::stoi(value.substr(2), nullptr, 16);
                        } else {
                            std::cerr << "Invalid TriggerKey value: " << value << std::endl;
                        }
                    } else if (key == "BlockBotKey") {
                        if (value.find("0x") == 0) {
                            blockBotKey = std::stoi(value.substr(2), nullptr, 16);
                        } else {
                            std::cerr << "Invalid BlockBotKey value: " << value << std::endl;
                        }
                    }
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid argument: " << e.what() << " for key: " << key << " with value: " << value << std::endl;
                } catch (const std::out_of_range& e) {
                    std::cerr << "Out of range: " << e.what() << " for key: " << key << " with value: " << value << std::endl;
                }
            }
        }
    }
}