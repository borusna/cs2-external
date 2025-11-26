#pragma once

namespace offsets{
    constexpr auto dwEntityList = 0x1D15F88;
    constexpr auto dwGameEntitySystem_highestEntityIndex = 0x20F0;
    constexpr auto dwGlobalVars = 0x1BE77F0;
    constexpr auto dwLocalPlayerController = 0x1E1F1E8;
    constexpr auto dwLocalPlayerPawn = 0x1BF2490;
    constexpr auto dwViewAngles = 0x1E3DC20;
    constexpr auto dwViewMatrix = 0x1E330F0;

    constexpr auto m_vOldOrigin = 0x15B0;
    constexpr auto m_iIDEntIndex = 0x1630;
    constexpr auto m_lifestate = 0x350;
    constexpr auto m_iTeamNum = 0x3EB;
    constexpr auto m_iHealth = 0x34C;
    constexpr auto m_vecOrigin = 0x5F8;
    constexpr auto m_vecAbsOrigin = 0xD0;
    constexpr auto m_pObserverServices = 0x1418;
    constexpr auto m_fFlags = 0x3F8;

    constexpr auto m_angEyeAngles = 0x3E50;

    constexpr auto m_skeletonInstance = 0x80;
    constexpr auto m_modelState = 0x190;

    constexpr auto m_pGameSceneNode = 0x330;

    constexpr auto m_iszPlayerName = 0x6E8;
    constexpr auto m_hPawn = 0x6B4;
    constexpr auto m_hObserverTarget = 0x44;

    constexpr auto m_hPlayerPawn = 0x8FC;
    constexpr auto m_lifeState = 0x350;
    constexpr auto m_bPawnIsAlive = 0x904;

    constexpr auto m_pNext = 0x60;
    constexpr auto m_pPrev = 0x58;
    constexpr auto m_vecVelocity = 0x430;

    constexpr auto jump = 0x1BEBE20;
}
