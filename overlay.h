#include "random_util.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

struct ThreadParams {
    HINSTANCE hInstance;
    int nCmdShow;
};

HWND hwndOverlay;
HINSTANCE hInstance = 0;


HFONT hFont;
std::map<int, HFONT> fontCache;

HFONT GetFont(int size) {
    if (fontCache.find(size) != fontCache.end()) {
        return fontCache[size];
    }

    HFONT hFont = ::CreateFontW(
        size,
        0,
        0,
        0,
        FW_HEAVY,
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        DEFAULT_PITCH | FF_SWISS,
        fontFamily.c_str()
    );

    fontCache[size] = hFont;
    return hFont;
}

DWORD WINAPI CreateOverlayWindow() {
    std::string classNameStr = generateRandomString(12);
    std::string windowTitleStr = generateRandomString(10);

    const char* className = classNameStr.c_str();
    const char* windowTitle = windowTitleStr.c_str();

    WNDCLASSEX wcex = { 0 };
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);
    wcex.lpszClassName = className;
    ::RegisterClassEx(&wcex);

    hwndOverlay = ::CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        className,
        windowTitle,
        WS_POPUP | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        NULL, NULL,
        hInstance,
        NULL);

    ::SetLayeredWindowAttributes(hwndOverlay, 0, 255, LWA_ALPHA);

    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwndOverlay, &margins);

    ::ShowWindow(hwndOverlay, SW_SHOW);
    ::UpdateWindow(hwndOverlay);

    MSG msg = { 0 };
    while (::GetMessageW(&msg, NULL, 0, 0) > 0) {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}

int lineNum;

void printScreenText(HDC frame, const std::string& strBuffer, int x, int y, int size) {
    hFont = GetFont(size);
    SelectObject(frame, hFont);

    RECT rc;
    GetClientRect(hwndOverlay, &rc);
    rc.left = x;
    rc.top = y;
    DrawTextEx(frame, (LPSTR)strBuffer.c_str(), -1, &rc, DT_SINGLELINE, NULL);
    lineNum++;
}

void drawScreenText(HDC frame, const std::string& strBuffer, int x, int y, int size) {
    hFont = GetFont(size);
    SelectObject(frame, hFont);

    RECT rc;
    GetClientRect(hwndOverlay, &rc);
    rc.left = x;
    rc.top = y;
    DrawTextEx(frame, (LPSTR)strBuffer.c_str(), -1, &rc, DT_SINGLELINE, NULL);
}

void drawBox(HDC frame, int x, int y, int width, int height, COLORREF color, BrushStyle brushStyle) {

    HPEN hPen = CreatePen(PS_SOLID, 1, color);
    HBRUSH hBrush;
    SelectObject(frame, hPen);

    switch(brushStyle) {
        case BRUSH_HOLLOW:
            SelectObject(frame, GetStockObject(NULL_BRUSH));
            break;
        case BRUSH_SOLID:
            hBrush = CreateSolidBrush(color);
            SelectObject(frame, hBrush);
            break;
        default:
            break;
    }

    Rectangle(frame, x, y, x + width, y + height);
    DeleteObject(hPen);
    DeleteObject(hBrush);
}

void drawAimLine(HDC frame, uintptr_t entity) {
    Vector3 angles = readvm<Vector3>(process, entity + offsets::m_angEyeAngles);

    Vector3 forward;
    float pitch = angles.x * (3.14159265f / 180.0f);
    float yaw = angles.y * (3.14159265f / 180.0f);

    forward.x = cosf(pitch) * cosf(yaw);
    forward.y = cosf(pitch) * sinf(yaw);
    forward.z = -sinf(pitch);

    uintptr_t playerNode = readvm<uintptr_t>(process, entity + offsets::m_pGameSceneNode);
    Vector3 eyePos = readvm<Vector3>(process, playerNode + offsets::m_vecAbsOrigin);

    ptr bonearray = readvm<ptr>(process, playerNode + offsets::m_modelState + 0x80);
    Vector3 headOrigin = readvm<Vector3>(process, bonearray + (6 * 32));

    eyePos.z = headOrigin.z;

    constexpr float maxRange = 250.0f;
    constexpr float stepSize = 5.0f;
    Vector3 dst = {
        eyePos.x + forward.x * maxRange,
        eyePos.y + forward.y * maxRange,
        eyePos.z + forward.z * maxRange
    };

    for (float traveled = 0.0f; traveled < maxRange; traveled += stepSize) {
        dst.x += forward.x * stepSize;
        dst.y += forward.y * stepSize;
        dst.z += forward.z * stepSize;
    }

    Vector2 screenSrc, screenDst;
    if (WorldToScreen(eyePos, &screenSrc, view_matrix, width, height) &&
        WorldToScreen(dst, &screenDst, view_matrix, width, height)) {

        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
        HPEN hOldPen = (HPEN)SelectObject(frame, hPen);

        MoveToEx(frame, static_cast<int>(screenSrc.x), static_cast<int>(screenSrc.y), NULL);
        LineTo(frame, static_cast<int>(screenDst.x), static_cast<int>(screenDst.y));
        
        constexpr int radius = 5;
        Ellipse(frame, 
                static_cast<int>(screenDst.x) - radius, 
                static_cast<int>(screenDst.y) - radius, 
                static_cast<int>(screenDst.x) + radius, 
                static_cast<int>(screenDst.y) + radius);

        SelectObject(frame, hOldPen);
        DeleteObject(hPen);
    }
}

void drawPlayerBones(HDC frame, ptr entity) {
    struct BoneJointData {
        Vector3 Pos;
        char pad[0x14];
    };

    ptr playerNode = readvm<ptr>(process, entity + offsets::m_pGameSceneNode);
    if (!playerNode)
        return;

    ptr boneArrayAddress = readvm<ptr>(process, playerNode + offsets::m_modelState + 0x80);
    if (!boneArrayAddress)
        return;

    constexpr int boneCount = 30;
    std::array<BoneJointData, boneCount> boneArray;
    for (int i = 0; i < boneCount; ++i) {
        boneArray[i] = readvm<BoneJointData>(process, boneArrayAddress + i * sizeof(BoneJointData));
    }

    std::vector<Vector2> screenBonePositions;
    screenBonePositions.reserve(boneCount);
    for (int i = 0; i < boneCount; ++i) {
        Vector2 screenPos;
        if (WorldToScreen(boneArray[i].Pos, &screenPos, view_matrix, width, height)) {
            screenBonePositions.push_back(screenPos);
        } else {
            screenBonePositions.push_back({ -1, -1 });
        }
    }

    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HPEN hOldPen = (HPEN)SelectObject(frame, hPen);
    for (const auto& connection : boneConnections) {
        int parentIndex = connection.first;
        int childIndex  = connection.second;
        if (parentIndex >= 0 && parentIndex < (int)screenBonePositions.size() &&
            childIndex  >= 0 && childIndex  < (int)screenBonePositions.size())
        {
            const Vector2 &parentPos = screenBonePositions[parentIndex];
            const Vector2 &childPos  = screenBonePositions[childIndex];
            if (parentPos.x != -1 && childPos.x != -1) {
                MoveToEx(frame, parentPos.x, parentPos.y, NULL);
                LineTo(frame, childPos.x, childPos.y);
            }
        }
    }
    SelectObject(frame, hOldPen);
    DeleteObject(hPen);
}

/*void printSpectatorList() {
    uintptr_t localPlayer = readvm<uintptr_t>(process, client + offsets::dwLocalPlayerController);
    if (!localPlayer) return;

    int targetHandle = readvm<int>(process, localPlayer + offsets::m_hObserverTarget);
    if (targetHandle <= 0 || targetHandle == -1) return;

    uintptr_t entityList = readvm<uintptr_t>(process, client + offsets::dwEntityList);
    uintptr_t listEntry = readvm<uintptr_t>(process, entityList + 0x8 * ((targetHandle & 0x7FFF) >> 9) + 16);
    uintptr_t targetEntity = readvm<uintptr_t>(process, listEntry + 0x70 * (targetHandle & 0x1FF));
    //std::cout << "on";

    std::vector<std::string> spectators;

    for (ptr controller : controllerCache) {
        int pawnHandle = readvm<int>(process, controller + offsets::m_hPlayerPawn);
        if (!pawnHandle) continue;

        uintptr_t localController = readvm<uintptr_t>(process, client + offsets::dwLocalPlayerController);
        if (!localController) return;

        int myPawnHandle = readvm<int>(process, localController + offsets::m_hPlayerPawn);
        if (myPawnHandle <= 0) return;

        ptr pawnEntry = readvm<ptr>(process, entityList + 0x8 * ((pawnHandle & 0x7FFF) >> 9) + 16);
        ptr pawn = readvm<ptr>(process, pawnEntry + 0x78 * (pawnHandle & 0x1FF));
        if (!pawn) continue;

        bool isAlive = readvm<bool>(process, controller + offsets::m_bPawnIsAlive);
        if (isAlive) continue;
        

        int obsTarget = readvm<int>(process, controller + offsets::m_hObserverTarget);
        if ((obsTarget & 0x7FFF) == (targetHandle & 0x7FFF)) {
            auto nameData = readvm<std::array<char, 32>>(process, controller + offsets::m_iszPlayerName);
            spectators.emplace_back(atos(nameData));
        }
    }

    std::cout << "[Spectators on you] (" << spectators.size() << "):" << std::endl;
    for (const auto& name : spectators) {
        std::cout << " - " << name << std::endl;
    }
}*/


void drawESP(HDC frame, int entityTeam, int playerTeam, ptr entity, ptr controller) {
    if (!healthbaresp && !boxesp && !nameesp && !skeletonesp) {
        return;
    }

    if (entityTeam != playerTeam) {
        if (healthEsp) {
            ptr playerNode = readvm<ptr>(process, entity + offsets::m_pGameSceneNode);
            if (!playerNode) return;

            ptr bonearray = readvm<ptr>(process, playerNode + offsets::m_modelState + 0x80);
            if (!bonearray) return;

            Vector3 headOrigin = readvm<Vector3>(process, bonearray + (6 * 32));
            Vector3 playerOrigin = readvm<Vector3>(process, playerNode + offsets::m_vecAbsOrigin);

            Vector2 screenBase;
            Vector2 screenHead;

            headOrigin.z += 8;
            playerOrigin.z -= 7;

            if (!WorldToScreen(playerOrigin, &screenBase, view_matrix, width, height)) return;
            if (!WorldToScreen(headOrigin, &screenHead, view_matrix, width, height)) return;

            float boxHeight = abs(screenBase.y - (screenHead.y));
            float boxWidth = boxHeight / 1.8f;
            int lFontSize = std::max(12 + static_cast<int>(boxHeight / 10), 15);

            if (boxesp) {
                drawBox(frame, screenBase.x - (boxWidth / 2), screenHead.y, boxWidth, boxHeight, RGB(255, 255, 255), BRUSH_HOLLOW);
            }

            if (healthbaresp) {
                int health = readvm<int>(process, entity + offsets::m_iHealth);
                int red = 255, green = 255;
                if (health > 50) red = 255 * (100 - health) / 50;
                else green = 255 * health / 50;
                drawBox(frame, screenBase.x - (boxWidth / 1.7), screenBase.y, -boxWidth / 21, -boxHeight * ((float)health / (100)), RGB(red, green, 0), BRUSH_SOLID);
            }

            if (nameesp) {
                headOrigin.z += 7.5;
                if (!WorldToScreen(headOrigin, &screenHead, view_matrix, width, height)) return;

                std::array<char, 32> entityName = readvm<std::array<char, 32>>(process, controller + offsets::m_iszPlayerName);
                std::string entityNameStr = atos(entityName);
                float textOffset = 10.0f;
                float textYPosition = screenHead.y - textOffset;
                drawScreenText(frame, entityNameStr, screenBase.x - (boxWidth / 2), textYPosition, lFontSize);
            }
            
            if (skeletonesp) {
                drawPlayerBones(frame, entity);
            }

            if (drawaimline) {
                drawAimLine(frame, entity);
            }
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        lineNum = 0;
        PAINTSTRUCT ps = { 0 };
        HDC frame = ::BeginPaint(hWnd, &ps);

        SelectObject(frame, hFont);

        SetTextColor(frame, textColor);
        SetBkMode(frame, TRANSPARENT);
        int start = height / 2;

        for (ptr controller : controllerCache) {
            int pawnHandle = readvm<int>(process, controller + offsets::m_hPlayerPawn);
            if (!pawnHandle) {
                update();
                continue;
            }

            bool isAlive = readvm<bool>(process, controller + offsets::m_bPawnIsAlive);
            if(!isAlive) continue;

            ptr listEntry2 = readvm<ptr>(process, entityList + (0x8 * ((pawnHandle & 0x7FFF) >> 9) + 16));
            ptr entity = readvm<ptr>(process, listEntry2 + (0x70 * (pawnHandle & 0x1FF)));
            if (!entity) continue;
            
            int entityTeam = readvm<int>(process, entity + offsets::m_iTeamNum);
            
            drawESP(frame, entityTeam, playerTeam, entity, controller);
        }

        EndPaint(hWnd, &ps);
    }
    return 0;

    case WM_NCHITTEST:
        return HTCLIENT;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;

    default:
        break;
    }
    return ::DefWindowProc(hWnd, message, wParam, lParam);
}