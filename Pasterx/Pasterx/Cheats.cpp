#include <thread>
#include "Menu.hpp"
#include "Mem.h"
#include <vector>

#include "offsets.h"
#include "config.h"
#include <iostream>

class Vector3 {
public:
    float x, y, z;

    Vector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }
};

struct view_matrix_t {
    float matrix[4][4];
};

Vector3 worldToScreen(const view_matrix_t& matrix, const Vector3& worldPos) {
    float _x = matrix.matrix[0][0] * worldPos.x + matrix.matrix[0][1] * worldPos.y + matrix.matrix[0][2] * worldPos.z + matrix.matrix[0][3];
    float _y = matrix.matrix[1][0] * worldPos.x + matrix.matrix[1][1] * worldPos.y + matrix.matrix[1][2] * worldPos.z + matrix.matrix[1][3];
    float _w = matrix.matrix[3][0] * worldPos.x + matrix.matrix[3][1] * worldPos.y + matrix.matrix[3][2] * worldPos.z + matrix.matrix[3][3];

    float inv_w = 1.f / _w;
    _x *= inv_w;
    _y *= inv_w;

    int screen_x = static_cast<int>((0.5f * _x + 0.5f) * static_cast<float>(GetSystemMetrics(SM_CXSCREEN)));
    int screen_y = static_cast<int>((-0.5f * _y + 0.5f) * static_cast<float>(GetSystemMetrics(SM_CYSCREEN)));

    return Vector3(static_cast<float>(screen_x), static_cast<float>(screen_y), _w);
}

void Trigger_bot() {
    while (true) {
        if (GetAsyncKeyState(VK_SHIFT) && config::trigger_bot) {
            const auto localPlayer = VARS::memRead<LONGLONG>(VARS::baseAddress + offsets::dwLocalPlayerPawn);
            const auto playerTeam = VARS::memRead<int>(localPlayer + offsets::m_iTeamNum);
            const auto entityId = VARS::memRead<int>(localPlayer + offsets::m_iIDEntIndex);

            if (localPlayer) {
                auto health = VARS::memRead<LONGLONG>(localPlayer + offsets::m_iHealth);

                if (health && entityId > 0) {
                    auto entList = VARS::memRead<LONGLONG>(VARS::baseAddress + offsets::dwEntityList);
                    auto entEntry = VARS::memRead<LONGLONG>(entList + 0x8 * (entityId >> 9) + 0x10);
                    auto entity = VARS::memRead<LONGLONG>(entEntry + 120 * (entityId & 0x1FF));
                    auto entityTeam = VARS::memRead<int>(entity + offsets::m_iTeamNum);

                    bool shouldShoot = true;

                    if (config::team_check) {
                        if (entityTeam == playerTeam) {
                            shouldShoot = false;
                        }
                    }

                    if (shouldShoot) {
                        auto entityHp = VARS::memRead<int>(entity + offsets::m_iHealth);
                        if (entityHp > 0) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                            Sleep(14);
                        }
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(config::triggerdelay));
    }
}

void boxesp() {
    if (config::Boxesp) {

        view_matrix_t matrix = VARS::memRead<view_matrix_t>(VARS::baseAddress + offsets::dwViewMatrix);

        uintptr_t localPlayer = VARS::memRead<uintptr_t>(VARS::baseAddress + offsets::dwLocalPlayerPawn);
        if (!localPlayer)
            return;

        int localPlayerTeam = VARS::memRead<int>(localPlayer + offsets::m_iTeamNum);

        for (int i = 1; i < 64; i++) {
            uintptr_t entityList = VARS::memRead<uintptr_t>(VARS::baseAddress + offsets::dwEntityList);

            uintptr_t listEntry1 = VARS::memRead<uintptr_t>(entityList + ((8 * (i & 0x7FFF) >> 9) + 16));
            if (!listEntry1)
                continue;

            uintptr_t playerController = VARS::memRead<uintptr_t>(listEntry1 + 120 * (i & 0x1FF));
            if (!playerController)
                continue;

            uint32_t PlayerPawn1 = VARS::memRead<uint32_t>(playerController + offsets::m_hPlayerPawn);
            if (!PlayerPawn1)
                continue;

            uintptr_t listEntry2 = VARS::memRead<uintptr_t>(entityList + 0x8 * ((PlayerPawn1 & 0x1FFF) >> 9) + 16);
            if (!listEntry2)
                continue;

            uintptr_t entityPawn = VARS::memRead<uintptr_t>(listEntry2 + 120 * (PlayerPawn1 & 0x1FF));
            if (!entityPawn)
                continue;

            if (entityPawn == localPlayer)
                continue;

            int playerHealth = VARS::memRead<int>(entityPawn + offsets::m_iHealth);
            if (playerHealth <= 0)
                continue;

            const auto playerTeam = VARS::memRead<int>(entityPawn + offsets::m_iTeamNum);

            ImVec4 snaplineColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

            Vector3 playerPos = VARS::memRead<Vector3>(entityPawn + offsets::vecOrigin);

            Vector3 screenPos = worldToScreen(matrix, playerPos);

            if (config::team_check && playerTeam == VARS::memRead<int>(localPlayer + offsets::m_iTeamNum))
                continue;

            Vector3 viewOffset = VARS::memRead<Vector3>(entityPawn + offsets::m_vecViewOffset);

            Vector3 topPos = { playerPos.x, playerPos.y, playerPos.z + viewOffset.z + 6 };
            Vector3 bottomPos = { playerPos.x, playerPos.y, playerPos.z - 6 };

            Vector3 screenTop = worldToScreen(matrix, topPos);
            Vector3 screenBottom = worldToScreen(matrix, bottomPos);

            if (screenPos.z > 0.001f) {

                float boxHeight = fabs(screenBottom.y - screenTop.y);
                float boxWidth = boxHeight / 2.0f;

                ImVec2 boxMin = ImVec2(screenTop.x - boxWidth / 2.0f, screenTop.y);
                ImVec2 boxMax = ImVec2(screenTop.x + boxWidth / 2.0f, screenBottom.y);

                ImDrawList* drawList = ImGui::GetBackgroundDrawList();

                drawList->AddRect(boxMin, boxMax, IM_COL32(255, 255, 255, 255), 5.5f, 0, 2.0f);

                drawList->AddText(boxMin, IM_COL32(255, 0, 0, 255), "Your gay");

            }
        }
    }
}

Vector3 CalcAngle(Vector3 src, Vector3 dst) {
    Vector3 angles;
    angles.x = atan2(dst.z - src.z, sqrt(pow(dst.x - src.x, 2) + pow(dst.y - src.y, 2))) * 180.0f / 3.14159265358979323846f;
    angles.y = atan2(dst.y - src.y, dst.x - src.x) * 180.0f / 3.14159265358979323846f;
    angles.z = 0.0f;
    return angles;
}

void drawFov() {
    if (config::drawFov) {
        ImGuiIO& io = ImGui::GetIO();
        float screenMidX = io.DisplaySize.x * 0.5f;
        float screenMidY = io.DisplaySize.y * 0.5f;

        float fovRadius = (config::FOV / 65.0f) * screenMidX;

        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        if (drawList) {

            drawList->AddCircle(
                ImVec2(screenMidX, screenMidY),
                fovRadius,
                IM_COL32(255, 255, 255, 255),
                32,
                2.0f
            );
        }
    }
}
void Aimbot() {
    Vector3 localPlayerPos;

    while (true) {

        uintptr_t localPlayer = VARS::memRead<uintptr_t>(VARS::baseAddress + offsets::dwLocalPlayerPawn);
        if (!localPlayer) {
            Sleep(10);
            continue;
        }

        localPlayerPos = VARS::memRead<Vector3>(localPlayer + offsets::vecOrigin);
        Vector3 viewAngles = VARS::memRead<Vector3>(VARS::baseAddress + offsets::dwViewAngles);

        float closestAngleDifference = FLT_MAX;
        uintptr_t closestEntityPawn = 0;

        for (int i = 1; i < 64; i++) {
            uintptr_t entityList = VARS::memRead<uintptr_t>(VARS::baseAddress + offsets::dwEntityList);

            uintptr_t listEntry1 = VARS::memRead<uintptr_t>(entityList + ((8 * (i & 0x7FFF) >> 9) + 16));
            if (!listEntry1)
                continue;

            uintptr_t playerController = VARS::memRead<uintptr_t>(listEntry1 + 120 * (i & 0x1FF));
            if (!playerController)
                continue;

            uint32_t PlayerPawn1 = VARS::memRead<uint32_t>(playerController + offsets::m_hPlayerPawn);
            if (!PlayerPawn1)
                continue;

            uintptr_t listEntry2 = VARS::memRead<uintptr_t>(entityList + 0x8 * ((PlayerPawn1 & 0x1FFF) >> 9) + 16);
            if (!listEntry2)
                continue;

            uintptr_t entityPawn = VARS::memRead<uintptr_t>(listEntry2 + 120 * (PlayerPawn1 & 0x1FF));
            if (!entityPawn || entityPawn == localPlayer)
                continue;

            int playerHealth = VARS::memRead<int>(entityPawn + offsets::m_iHealth);
            if (playerHealth <= 0)
                continue;

            const auto playerTeam = VARS::memRead<int>(entityPawn + offsets::m_iTeamNum);
            int localPlayerTeam = VARS::memRead<int>(localPlayer + offsets::m_iTeamNum);

            if (config::team_check && playerTeam == localPlayerTeam) {
                continue;
            }

            Vector3 enemyPos = VARS::memRead<Vector3>(entityPawn + offsets::vecOrigin);

            Vector3 angleToEnemy = CalcAngle(localPlayerPos, enemyPos);
            Vector3 currentViewAngles = VARS::memRead<Vector3>(VARS::baseAddress + offsets::dwViewAngles);

            float angleDifference = fabs(angleToEnemy.y - currentViewAngles.y);
            angleDifference = min(angleDifference, 360.0f - angleDifference);

            if (angleDifference < closestAngleDifference) {
                closestAngleDifference = angleDifference;
                closestEntityPawn = entityPawn;
            }
        }

        int closestEntityHealth = VARS::memRead<int>(closestEntityPawn + offsets::m_iHealth);
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000 && config::aimbot && closestEntityPawn && closestEntityHealth > 0 && closestEntityHealth <= 100) {
            Vector3 closestEnemyPos = VARS::memRead<Vector3>(closestEntityPawn + offsets::vecOrigin);
            Vector3 angleToEnemy = CalcAngle(localPlayerPos, closestEnemyPos);

            Vector3 currentViewAngles = VARS::memRead<Vector3>(VARS::baseAddress + offsets::dwViewAngles);
            angleToEnemy.x = currentViewAngles.x;
            angleToEnemy.z = currentViewAngles.z;

            if (closestAngleDifference <= config::FOV) {
                VARS::memWrite<Vector3>(VARS::baseAddress + offsets::dwViewAngles, angleToEnemy);
            }
        }

        Sleep(1);
    }
}

void noflash() {
    while (true)
    {
        if (config::noflash)
        {
            uintptr_t localPlayer = VARS::memRead<uintptr_t>(VARS::baseAddress + offsets::dwLocalPlayerPawn);
            VARS::memWrite<float>(localPlayer + offsets::m_flFlashDuration, 0.f);
        }
        Sleep(100);
    }

}

void bunny_hop() {
    while (true) {
        if (config::bunny_hop)
        {

            const auto localPlayer = VARS::memRead<LONGLONG>(VARS::baseAddress + offsets::dwLocalPlayerPawn);
            if (localPlayer) {
                int32_t m_fFlags = VARS::memRead<int32_t>(localPlayer + offsets::fFlags);
                if (GetAsyncKeyState(VK_SPACE) && (m_fFlags & (1 << 0))) {
                    VARS::memWrite<int>(VARS::baseAddress + offsets::Force_Jump, 65537);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    VARS::memWrite<int>(VARS::baseAddress + offsets::Force_Jump, 256);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

int main() {
    config::baseaddress = VARS::baseAddress;

    overlay.shouldRun = true;
    overlay.RenderMenu = false;

    overlay.CreateOverlay();
    overlay.CreateDevice();
    overlay.CreateImGui();

    overlay.SetForeground(GetConsoleWindow());

  
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Bahnschrift.ttf", 24.0f);
    IM_ASSERT(font != NULL);

    io.Fonts->Build();

    std::thread trigger(Trigger_bot);
    std::thread Aim(Aimbot);
    std::thread nf(noflash);
    std::thread Bhop(bunny_hop);

    while (overlay.shouldRun) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        overlay.StartRender();

        if (overlay.RenderMenu) {
            overlay.Render();
        }

        boxesp();
        drawFov();

        overlay.EndRender();
    }

    trigger.join();
    Aim.join();
    nf.join();
    Bhop.join();

    overlay.DestroyImGui();
    overlay.DestroyDevice();
    overlay.DestroyOverlay();

    return 0;
}
