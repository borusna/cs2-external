#define ptr uintptr_t

struct view_matrix_t {
    float matrix[16];
};

struct Vector4
{
    float x, y, z, w;

    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
};

struct Vector3
{
    float x, y, z;

    Vector3 operator-(const Vector3& other) const
    {
        return Vector3{
            x - other.x,
            y - other.y,
            z - other.z
        };
    }
};

std::ostream& operator<<(std::ostream& os, const Vector3& vec) {
    os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return os;
}

struct Vector2
{
    float x, y;
};

#define RGBA(r, g, b, a) ((DWORD)((((a)&0xff)<<24) | (((r)&0xff)<<16) | (((g)&0xff)<<8) | ((b)&0xff)))

namespace globals {
    HANDLE process;

    std::atomic<bool> timeToTypeFlag(false);

    view_matrix_t view_matrix;
    float distance;
    int width = 0;
    int height = 0;

    ptr client;
    ptr player;
    ptr entityList;
	ptr listEntry;
    std::vector<ptr> controllerCache;
    

    int playerTeam;
    int max_ent;

    bool healthEsp = false;

    bool healthbaresp = false;
    bool boxesp = false;
    bool nameesp = false;
    bool skeletonesp = false;
    bool drawaimline = false;

    int espKey;
    int triggerKey;
    int blockBotKey;

    std::atomic<bool> triggerBot = false;
    std::atomic<bool> legitTrigger = false;
    std::atomic<bool> headOnly = false;

    std::atomic<bool> bhop = false;

    bool blockBot = true;

    bool enablePrintSpectatorList = true;

    enum bone_ids
    {
            head = 6,
            neck = 5,
            spine = 4,
            spine_1 = 2,
            hip = 0,
            left_shoulder = 8,
            left_arm = 9,
            left_hand = 10,
            pelvis = 0,
            right_shoulder = 13,
            right_arm = 14,
            right_hand = 15,
            left_hip = 22,
            left_knee = 23,
            left_feet = 24,
            right_hip = 25, 
            right_knee = 26,
            right_feet = 27
    };

    std::vector<std::pair<int, int>> boneConnections = {
        {head, neck},
        {spine, neck},
        {spine, left_shoulder},
        {left_shoulder, left_arm},
        {left_arm, left_hand},
        {spine, right_shoulder},
        {right_shoulder, right_arm},
        {right_arm, right_hand},
        {spine, spine_1},
        {pelvis, spine},
        {pelvis, left_hip},
        {left_hip, left_knee},
        {left_knee, left_feet},
        {pelvis, right_hip},
        {right_hip, right_knee},
        {right_knee, right_feet},
    };

    enum specialViewMode {
        ESP_TOGGLE_MODE,
        ESP_HOLD_MODE
    };
    specialViewMode espMode;

    enum BrushStyle {
        BRUSH_HOLLOW,
        BRUSH_SOLID
    };

    int gFontSize;
    COLORREF textColor;
    std::wstring fontFamily;
}