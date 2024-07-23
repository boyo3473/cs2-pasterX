#include <cstddef>
namespace offsets {
	//Buttons.hpp
	constexpr std::ptrdiff_t Force_Jump = 0x181D670;

	//offsets.hpp
	constexpr std::ptrdiff_t dwEntityList = 0x19BEEB0;
	constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x1824A08;
	constexpr std::ptrdiff_t dwViewAngles = 0x1A2E248;
	constexpr std::ptrdiff_t m_hPlayerPawn = 0x7DC; // CHandle<C_CSPlayerPawn>
	constexpr std::ptrdiff_t dwViewMatrix = 0x1A20CD0;

	//client.dll
	constexpr std::ptrdiff_t m_iIDEntIndex = 0x13A8;
	constexpr std::ptrdiff_t m_iTeamNum = 0x3C3;
	constexpr std::ptrdiff_t m_iHealth = 0x324;
	constexpr std::ptrdiff_t fFlags = 0x3CC;
	constexpr std::ptrdiff_t vecOrigin = 0x1274;
	constexpr std::ptrdiff_t m_flFlashDuration = 0x135C;
	constexpr std::ptrdiff_t m_entitySpottedState = 0x2288; // EntitySpottedState_t
	constexpr std::ptrdiff_t m_bSpottedByMask = 0xC;
	constexpr std::ptrdiff_t m_iszPlayerName = 0x630; // char[128]
	constexpr std::ptrdiff_t m_iBoneIndex = 0xF00; // int32
	constexpr std::ptrdiff_t m_vecViewOffset = 0xC50; // CNetworkViewOffsetVector
	constexpr std::ptrdiff_t m_pGameSceneNode = 0x308; // CGameSceneNode*
	constexpr std::ptrdiff_t m_modelState = 0x170; // CModelState

	

	constexpr std::ptrdiff_t m_iAccount = 0x40; // int32
	constexpr std::ptrdiff_t m_pInGameMoneyServices = 0x6F0; // CCSPlayerController_InGameMoneyServices*



}