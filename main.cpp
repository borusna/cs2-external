#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <windows.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <tlhelp32.h>
#include <chrono>
#include <thread>
#include <array>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <atomic>
#include <map>
#include <cmath>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <intrin.h>
#include <mmsystem.h>
#include <conio.h>
#include <cfloat>
#pragma comment(lib, "winmm.lib")
#include "globals.h"

#include <d2d1.h>
#include <dwrite.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")


#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

using namespace globals;

#include "offsets.h"
#include "driver.h"
#include "util.h"
#include "overlay.h"




void mainLoop();
void triggerbotFunc();
void blockBotFunc();
void bhopFunc();
void keyExitEvent();

std::unordered_map<ptr, float> previousDirs;

#define ID_CHECKBOX_ESP 101
#define ID_CHECKBOX_NAMEESP 102
#define ID_CHECKBOX_SKELETONESP 103
#define ID_CHECKBOX_HEALTHBARESP 104
#define ID_CHECKBOX_BOXESP 105
#define ID_CHECKBOX_AIMLINEESP 106


#define ID_CHECKBOX_TRIGGERBOT 107
#define ID_CHECKBOX_LEGITTRIGGERBOT 108
#define ID_CHECKBOX_TRIGGERHEADONLY 109

#define ID_CHECKBOX_BLOCKBOT 110

#define ID_CHECKBOX_BHOP 111

#define ID_BUTTON_LOADCONFIG 150


LRESULT CALLBACK MenuProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateWindow(TEXT("BUTTON"), TEXT("Enable ESP"),
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            20, 20, 150, 30,
            hwnd, (HMENU)ID_CHECKBOX_ESP, NULL, NULL);

        CreateWindow(TEXT("BUTTON"), TEXT("Enable Name ESP"),
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            20, 50, 150, 30,
            hwnd, (HMENU)ID_CHECKBOX_NAMEESP, NULL, NULL);

        CreateWindow(TEXT("BUTTON"), TEXT("Enable Skeleton ESP"),
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            20, 80, 150, 30,
            hwnd, (HMENU)ID_CHECKBOX_SKELETONESP, NULL, NULL);

        CreateWindow(TEXT("BUTTON"), TEXT("Enable Health ESP"),
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            20, 110, 150, 30,
            hwnd, (HMENU)ID_CHECKBOX_HEALTHBARESP, NULL, NULL);

        CreateWindow(TEXT("BUTTON"), TEXT("Enable Box ESP"),
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            20, 140, 150, 30,
            hwnd, (HMENU)ID_CHECKBOX_BOXESP, NULL, NULL);

        CreateWindow(TEXT("BUTTON"), TEXT("Enable Aim Line"),
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            20, 170, 150, 30,
            hwnd, (HMENU)ID_CHECKBOX_AIMLINEESP, NULL, NULL);
        
        

        CreateWindow(TEXT("BUTTON"), TEXT("Enable Triggerbot"),
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            20, 250, 150, 30,
            hwnd, (HMENU)ID_CHECKBOX_TRIGGERBOT, NULL, NULL);
        
        CreateWindow(TEXT("BUTTON"), TEXT("Enable Delay"),
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            20, 280, 150, 30,
            hwnd, (HMENU)ID_CHECKBOX_LEGITTRIGGERBOT, NULL, NULL);

        CreateWindow(TEXT("BUTTON"), TEXT("Enable Head Only"),
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            20, 310, 150, 30,
            hwnd, (HMENU)ID_CHECKBOX_TRIGGERHEADONLY, NULL, NULL);

        

        CreateWindow(TEXT("BUTTON"), TEXT("Enable Blockbot"),
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            200, 20, 150, 30,
            hwnd, (HMENU)ID_CHECKBOX_BLOCKBOT, NULL, NULL);



        CreateWindow(TEXT("BUTTON"), TEXT("Enable Bhop"),
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            200, 50, 150, 30,
            hwnd, (HMENU)ID_CHECKBOX_BHOP, NULL, NULL);



        CreateWindow(TEXT("BUTTON"), TEXT("Reload Config"),
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            20, 420, 120, 30,       
            hwnd, (HMENU)ID_BUTTON_LOADCONFIG, NULL, NULL);

        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case ID_CHECKBOX_ESP:
                healthEsp = (IsDlgButtonChecked(hwnd, ID_CHECKBOX_ESP) == BST_CHECKED);
                break;
            case ID_CHECKBOX_NAMEESP:
                nameesp = (IsDlgButtonChecked(hwnd, ID_CHECKBOX_NAMEESP) == BST_CHECKED);
                break;
            case ID_CHECKBOX_SKELETONESP:
                skeletonesp = (IsDlgButtonChecked(hwnd, ID_CHECKBOX_SKELETONESP) == BST_CHECKED);
                break;
            case ID_CHECKBOX_HEALTHBARESP:
                healthbaresp = (IsDlgButtonChecked(hwnd, ID_CHECKBOX_HEALTHBARESP) == BST_CHECKED);
                break;
            case ID_CHECKBOX_BOXESP:
                boxesp = (IsDlgButtonChecked(hwnd, ID_CHECKBOX_BOXESP) == BST_CHECKED);
                break;
            case ID_CHECKBOX_AIMLINEESP:
                drawaimline = (IsDlgButtonChecked(hwnd, ID_CHECKBOX_AIMLINEESP) == BST_CHECKED);
                break;

            case ID_CHECKBOX_TRIGGERBOT:
                triggerBot = (IsDlgButtonChecked(hwnd, ID_CHECKBOX_TRIGGERBOT) == BST_CHECKED);
                break;
            case ID_CHECKBOX_LEGITTRIGGERBOT:
                legitTrigger = (IsDlgButtonChecked(hwnd, ID_CHECKBOX_LEGITTRIGGERBOT) == BST_CHECKED);
                break;
            case ID_CHECKBOX_TRIGGERHEADONLY:
                headOnly = (IsDlgButtonChecked(hwnd, ID_CHECKBOX_TRIGGERHEADONLY) == BST_CHECKED);
                break;

            case ID_CHECKBOX_BLOCKBOT:
                blockBot = (IsDlgButtonChecked(hwnd, ID_CHECKBOX_BLOCKBOT) == BST_CHECKED);
                break;
            
            case ID_CHECKBOX_BHOP:
                bhop = (IsDlgButtonChecked(hwnd, ID_CHECKBOX_BHOP) == BST_CHECKED);
                break;
                
            case ID_BUTTON_LOADCONFIG:
                LoadConfig();
                break;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASS wc = { };
    wc.lpfnWndProc = MenuProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("Menu");

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        TEXT("Menu"),
        WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_THICKFRAME),
        CW_USEDEFAULT, CW_USEDEFAULT, 200, 100,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

void CreateMenuWindow(HINSTANCE hInstance) {
    WNDCLASS wc = { };
    wc.lpfnWndProc = MenuProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("MenuClass");

    RegisterClass(&wc);
    std::string windowTitleStr = generateRandomString(10);
    const char* windowTitle = windowTitleStr.c_str();

    HWND hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        TEXT(windowTitle),
        WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_THICKFRAME),
        CW_USEDEFAULT, CW_USEDEFAULT, 750, 500,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

int main() {
	timeBeginPeriod(1);
	
    DWORD pid = getPID(L"cs2.exe"); 

    while (!pid) {
        std::cout << "Could not find cs2.exe!\n";
        Sleep(2000);
        pid = getPID(L"cs2.exe");
        if (pid) {
            std::cout << "Found CS2. One moment." << std::endl;
        }
    }

    HWND gameWindow = FindWindowW(NULL, L"Counter-Strike 2");

    if (gameWindow == NULL) {
        std::cerr << "Could not find the window" << std::endl;
        return 1;
    }

    RECT rect;

    if (GetWindowRect(gameWindow, &rect)) {
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
    } else {
        std::cerr << "Failed to get window dimensions" << std::endl;
        return 1;
    }

    #ifndef DRIVER
    int result = MessageBoxA(
        nullptr,
        "WARNING - YOU ARE NOT USING KERNEL DRIVER\n\n"
        "Press OK to continue, or Cancel to exit.",
        "Warning",
        MB_ICONWARNING | MB_OKCANCEL
    );

    if (result == IDOK) {
        // do nothing
    } else {
        return 0;
    }


    process = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
    if (!process) {
        std::cerr << "Failed to open process: " << GetLastError() << std::endl;
        return 1;
    }
    #else
    initDriver();
    attachProcess(pid);
    #endif

    client = getModuleBase(pid, L"client.dll");

    if (!client) {
        return 1;
    }

    std::cout << "Updated at: " << __DATE__ << " | " << __TIME__ << " EST" << std::endl;

    if (!hInstance) {
        hInstance = GetModuleHandle(NULL);
    }

    std::thread t_drawLoop(CreateOverlayWindow);
    std::thread t_triggerbotLoop(triggerbotFunc);
    std::thread t_blockbotLoop(blockBotFunc);
    std::thread t_bhopLoop(bhopFunc);
    std::thread t_keyExitEventLoop(keyExitEvent);

    t_drawLoop.detach();
    t_triggerbotLoop.detach();
    t_blockbotLoop.detach();
    t_bhopLoop.detach();
    t_keyExitEventLoop.detach();


    std::thread guiThread(CreateMenuWindow, hInstance);
    guiThread.detach();
    
    LoadConfig();
    mainLoop();

    return 0;
}

void mainLoop() {
    HDC hdc = GetDC(NULL);
    if (hdc) ReleaseDC(NULL, hdc);

    DEVMODE devMode = {};
    devMode.dmSize = sizeof(devMode);
    
    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode)) {
        std::cout << "Screen refresh rate: " << devMode.dmDisplayFrequency << " Hz" << std::endl;
    } else {
        std::cerr << "Failed to get display settings." << std::endl;
    }

    int refreshRate;
    refreshRate = devMode.dmDisplayFrequency;

    const auto interval = std::chrono::milliseconds(1000 / refreshRate);
    auto lastTime = std::chrono::steady_clock::now();
    
    while (true) {
        Sleep(1);
        view_matrix = readvm<view_matrix_t>(process, client + offsets::dwViewMatrix);

        if(!controllerCache.size()) update();

        auto currentTime = std::chrono::steady_clock::now();
		
		auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();
        //std::cout << "Overlay frame time: " << delta << " ms\n";
		
        if (currentTime - lastTime >= interval) {
            InvalidateRect(hwndOverlay, NULL, TRUE);
            lastTime = currentTime;
        }
    }
}

void triggerbotFunc() {
    while (true) {
        Sleep(1);
        if (triggerBot && GetAsyncKeyState(triggerKey)) {
            uintptr_t localPlayer = readvm<uintptr_t>(process, client + offsets::dwLocalPlayerPawn);
            if (!localPlayer) continue;

            int entityId = readvm<int>(process, localPlayer + offsets::m_iIDEntIndex);
            if (entityId <= 0) continue;

            uintptr_t entityList = readvm<uintptr_t>(process, client + offsets::dwEntityList);
            uintptr_t entityEntry = readvm<uintptr_t>(process, entityList + 0x8 * (entityId >> 9) + 0x10);
            uintptr_t entity = readvm<uintptr_t>(process, entityEntry + 120 * (entityId & 0x1FF));

            int entityTeam = readvm<int>(process, entity + offsets::m_iTeamNum);
            int playerTeam = readvm<int>(process, localPlayer + offsets::m_iTeamNum);

            if (entityTeam != playerTeam) {
                int entityHp = readvm<int>(process, entity + offsets::m_iHealth);
                if (entityHp > 0) {
                    ptr playerNode = readvm<ptr>(process, entity + offsets::m_pGameSceneNode);
                    if (!playerNode) return;

                    ptr boneArrayAddress = readvm<ptr>(process, playerNode + offsets::m_modelState + 0x80);
                    if (!boneArrayAddress) return;

                    Vector3 headWorldPos = readvm<Vector3>(process, boneArrayAddress + bone_ids::head * 0x20);
                    Vector2 headScreenPos;
                    if (!WorldToScreen(headWorldPos, &headScreenPos, view_matrix, width, height)) return;

                    float dx = headScreenPos.x - (width / 2);
                    float dy = headScreenPos.y - (height / 2);
                    float distance = sqrtf(dx * dx + dy * dy);

                    if(headOnly) {
                        if (distance < 20.5f) {
                            //std::cout << "[DEBUG] GameSceneNode: " << std::hex << playerNode << "\n";
                            //std::cout << "[DEBUG] Bone Array Address: " << std::hex << boneArrayAddress << "\n";
                            if (legitTrigger) {
                                Sleep(rand() % 13 + 21);
                                clickMouse(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP);
                                Sleep(rand() % 6 + 7);
                            } else {
                                clickMouse(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP);
                                Sleep(1);
                            }
                        }
                    } else {
                            if (legitTrigger) {
                                Sleep(rand() % 13 + 21);
                                clickMouse(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP);
                                Sleep(rand() % 6 + 7);
                            } else {
                                clickMouse(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP);
                                Sleep(1);
                            }
                    }
                }
            }
        }
    }   
}

void setKey(char key, bool wantDown, bool &isDown) {
    if (wantDown && !isDown) { keyDown(key); isDown = true; }
    else if (!wantDown && isDown) { keyUp(key); isDown = false; }
}

void blockBotFunc() {
    bool wDown = false, aDown = false, sDown = false, dDown = false;

    while (true) {
        Sleep(1);

        if (blockBot && GetAsyncKeyState(blockBotKey)) {
            uintptr_t localPlayerPawn = readvm<uintptr_t>(process, client + offsets::dwLocalPlayerPawn);
            ptr playerNode = readvm<ptr>(process, localPlayerPawn + offsets::m_pGameSceneNode);
            Vector3 playerPos = readvm<Vector3>(process, playerNode + offsets::m_vecAbsOrigin);

            uintptr_t localController = readvm<uintptr_t>(process, client + offsets::dwLocalPlayerController);
            int playerTeam = readvm<int>(process, localPlayerPawn + offsets::m_iTeamNum);

            ptr closestEntity = 0;
            float closestDistSqr = FLT_MAX;
            Vector3 closestDiff = {};

            for (ptr controller : controllerCache) {
                int pawnHandle = readvm<int>(process, controller + offsets::m_hPlayerPawn);
                if (!pawnHandle) continue;

                ptr listEntry2 = readvm<ptr>(process, entityList + (0x8 * ((pawnHandle & 0x7FFF) >> 9) + 16));
                ptr entity = readvm<ptr>(process, listEntry2 + (0x78 * (pawnHandle & 0x1FF)));
                if (!entity) continue;

                int entityTeam = readvm<int>(process, entity + offsets::m_iTeamNum);
                if (entityTeam != playerTeam || entity == localPlayerPawn) continue;

                ptr enemyNode = readvm<ptr>(process, entity + offsets::m_pGameSceneNode);
                Vector3 enemyPos = readvm<Vector3>(process, enemyNode + offsets::m_vecAbsOrigin);

                Vector3 diff = {
                    enemyPos.x - playerPos.x,
                    enemyPos.y - playerPos.y,
                    0.0f
                };

                float distSqr = diff.x * diff.x + diff.y * diff.y;
                if (distSqr < closestDistSqr) {
                    closestDistSqr = distSqr;
                    closestEntity = entity;
                    closestDiff = diff;
                }
            }

            bool shouldMoveForward = false;
            bool shouldMoveBackward = false;
            bool shouldMoveLeft = false;
            bool shouldMoveRight = false;

            if (closestEntity) {
                ptr enemyNode = readvm<ptr>(process, closestEntity + offsets::m_pGameSceneNode);
                Vector3 enemyPos = readvm<Vector3>(process, enemyNode + offsets::m_vecAbsOrigin);

                float playerYaw = readvm<float>(process, localPlayerPawn + offsets::m_angEyeAngles + 4);
                float yawRadians = -playerYaw * (3.14159265f / 180.f);
                float cosYaw = cosf(yawRadians);
                float sinYaw = sinf(yawRadians);

                float localForward = closestDiff.x * cosYaw - closestDiff.y * sinYaw;
                float localRight   = -(closestDiff.x * sinYaw + closestDiff.y * cosYaw);

                float verticalThreshold = 35.f;
                float verticalDiff = playerPos.z - enemyPos.z;
                bool verticalCondition = fabsf(verticalDiff) > verticalThreshold;

                float threshold = 2.f;

                shouldMoveForward = verticalCondition && (localForward > threshold);
                shouldMoveBackward = verticalCondition && (localForward < -threshold);
                shouldMoveRight = localRight > threshold;
                shouldMoveLeft  = localRight < -threshold;
            }

            setKey('W', shouldMoveForward, wDown);
            setKey('S', shouldMoveBackward, sDown);
            setKey('A', shouldMoveLeft, aDown);
            setKey('D', shouldMoveRight, dDown);
        } else {
            setKey('W', false, wDown);
            setKey('S', false, sDown);
            setKey('A', false, aDown);
            setKey('D', false, dDown);
        }
    }
}

void bhopFunc() {
    while (true) {
        //Sleep(1);
        if (bhop && GetAsyncKeyState(VK_SPACE)) {
            int onGround = (readvm<int>(process, player + offsets::m_fFlags) & 1 << 0);
            if (onGround) {
                press(VK_F20);
                Sleep(1);
                //std::cout << onGround;
            }
        } else {
            Sleep(0);
        }
    }
}

void keyExitEvent() {
    while (1) {
        Sleep(10);

        
        if (GetAsyncKeyState(0xA1)) exit(1);
    }
}
