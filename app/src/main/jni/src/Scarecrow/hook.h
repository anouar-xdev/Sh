#ifndef HOOK_H
#define HOOK_H

#include "bool.h"
#include "offsets.h"
#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <map>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

struct Vec2 {
  float x, y;
  Vec2() : x(0), y(0) {}
  Vec2(float _x, float _y) : x(_x), y(_y) {}
};

struct CustomQuaternion {
  float X, Y, Z, W;
  CustomQuaternion() : X(0), Y(0), Z(0), W(1) {}
  CustomQuaternion(float x, float y, float z, float w)
      : X(x), Y(y), Z(z), W(w) {}
};

extern float g_ThemeColorArr[4];

extern bool IsFingerDown(int slot);
extern bool GetFingerScreenPos(int slot, float& outX, float& outY);
extern void Touch_Down_Slot(int slot, float xt, float yt);
extern void Touch_Move_Slot(int slot, float x, float y);
extern void Touch_Up_Slot(int slot);

bool is3600 = false;
bool track = true;
bool SpeedHackV2 = false;
float SpeedMultiplier = 4.8f;
bool GhostHack = false;
bool NoReload = false;
bool NoRecoil = false;

bool Enable = true;
bool showLine = false;
bool showDistance = false;
bool showBox = false;
bool showName = false;
bool showHealth = false;
bool showCount = false;
bool showAimLine = false;
bool showSkeleton = false;

bool AimLock = false;
bool Aimbot = false;
bool AimVisible = false;
bool AimSilent = false;
bool Ignorknock = false;
bool IgnoreBot = false;
float AimFov = 100.0f;
float SilentAimFov = 20.0f;
int SilentAimPercentage = 100;
float SilentRecoilFactor = 0.05f;
bool showAimFov = false;
int textSize = 15.0f;
bool aimVisibilityCheck = true;

struct TouchAimbotConfig {
    bool  EnableAim        = false;
    bool  ShowTouchPoint   = true;
    bool  LockTouchPoint   = false;
    float TouchPointX      = 0.80f;
    float TouchPointY      = 0.60f;
    bool  ShowFireButton   = false;
    bool  LockFireButton   = false;
    float FireButtonX      = 0.85f;
    float FireButtonY      = 0.50f;
    float FireButtonSize   = 55.0f;
    float FovRange         = 150.0f;
    float AimSpeed         = 18.0f;
    int   TargetBone       = 0;
    bool  IgnoreKnocked    = false;
    float TouchSize        = 350.0f;
    float AimZ             = 0.0f;
    float aimpredic        = 1.0f;
    float BulletSpeed      = 600.0f;
    int   TriggerMode      = 0;
    int   ScreenPos        = 0;
    int   AimTarget        = 0;
};
TouchAimbotConfig TouchCfg;

struct TouchAimStruct {
    uintptr_t PawnAddr = 0;
    Vector3   ObjAim;
    Vector3   Velocity;
    float     ScreenDistance = 0;
    float     WodDistance    = 0;
};

TouchAimStruct TouchAim[100];
int TouchAimCount = 0;
uintptr_t g_TouchLockedPawn = 0;
std::mutex g_TouchAimMtx;
bool g_TouchAimFire = false;
D3DMatrix g_TouchViewMatrix;

static std::mt19937_64 g_rng(1337);
static std::uniform_real_distribution<float> g_jitterDist(-0.0012f, 0.0012f);

int globalPlayerCount = 0;
int globalBotCount = 0;

int boxStyle = 0;
float boxThickness = 2.0f;
float boxCornerRound = 4.0f;
float boxFillAlpha = 0.3f;
int boxColorMode = 0;
ImVec4 boxCustomColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

int lineStyle = 0;
float lineThickness = 2.0f;
int lineColorMode = 0;
ImVec4 lineCustomColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
int lineStartPosition = 0;

int healthBarStyle = 0;
float healthBarWidth = 7.0f;
float healthBarRounding = 2.0f;
bool showHealthText = false;
bool showHealthColorGradient = true;
bool animateHealthBar = false;

char PlayerName[64];

void UpdateNoReload(uintptr_t localPlayer) {
    if (!IsValidPointer(localPlayer)) return;
    uintptr_t attrs = Read<uintptr_t>(localPlayer + Offsets::PlayerAttributes);
    if (!IsValidPointer(attrs)) return;
    static uintptr_t lastAttrs = 0;
    static bool origSaved = false;
    static bool origValue = false;
    if (attrs != lastAttrs) { lastAttrs = attrs; origSaved = false; origValue = false; }
    if (NoReload) {
        if (!origSaved) { origValue = Read<bool>(attrs + Offsets::ShootNoReload); origSaved = true; }
        Write<bool>(attrs + Offsets::ShootNoReload, true);
    } else {
        if (origSaved) { Write<bool>(attrs + Offsets::ShootNoReload, origValue); origSaved = false; }
    }
}

inline void UpdatePlayerAttributes(uintptr_t localPlayer) {
    if (!IsValidPointer(localPlayer)) return;
    uintptr_t attrs = Read<uintptr_t>(localPlayer + Offsets::PlayerAttributes);
    if (!IsValidPointer(attrs)) return;
    // No player attribute features currently enabled
}

void UpdateNoRecoil(uintptr_t localPlayer) {
    if (!NoRecoil) return;
    if (!IsValidPointer(localPlayer)) return;
    uintptr_t invMgr = Read<uintptr_t>(localPlayer + Offsets::InventoryManager);
    if (!IsValidPointer(invMgr)) return;
    uintptr_t activeItem = Read<uintptr_t>(invMgr + Offsets::Inventory_ActiveItem);
    if (!IsValidPointer(activeItem)) return;
    uintptr_t wcomp = Read<uintptr_t>(activeItem + Offsets::Inventory_WeaponComponent);
    if (!IsValidPointer(wcomp)) return;
    Write<float>(wcomp + Offsets::Inventory_NoRecoil, 0.0f);
}

std::atomic<uintptr_t> g_LocalPlayer{0};
std::atomic<uintptr_t> g_CurrentTargetObj{0};
std::atomic<bool> g_HasData{false};

int selectedBoneIndex = 0;
const char *boneItems[] = {"HEAD", "BODY"};

uintptr_t GetPlayerHeadTF(uintptr_t player) {
  if (!IsValidPointer(player)) return 0;
  return Read<uintptr_t>(player + Offsets::Player_HeadTF);
}

uintptr_t GetPlayerPeTF(uintptr_t player) {
  if (!IsValidPointer(player)) return 0;
  return Read<uintptr_t>(player + Offsets::Player_FootTF);
}

uintptr_t GetPlayerMainCamera(uintptr_t player) {
  if (!IsValidPointer(player)) return 0;
  return Read<uintptr_t>(player + Offsets::Player_MainCamera);
}

struct Matrix {
  Vector4 Position;
  Quaternion Rotation;
  Vector4 Scale;
};

static auto GetPosition(uintptr_t Transform) {
  auto pos = Vector3::Zero();
  if (!IsValidPointer(Transform)) return pos;
  auto transformObjValue = Read<uintptr_t>(Transform + Offsets::Transform_TransformToObj);
  if (!IsValidPointer(transformObjValue)) return pos;
  auto indexValue = Read<uintptr_t>(transformObjValue + Offsets::Transform_ObjToIndex);
  auto matrixValue = Read<uintptr_t>(transformObjValue + Offsets::Transform_ObjToMatrix);
  if (!IsValidPointer(matrixValue)) return pos;
  auto matrixListValue = Read<uintptr_t>(matrixValue + Offsets::Transform_MatrixList);
  auto matrixIndicesValue = Read<uintptr_t>(matrixValue + Offsets::Transform_MatrixIndices);
  if (!IsValidPointer(matrixListValue) || !IsValidPointer(matrixIndicesValue)) return pos;

  auto resultValue = Read<Vector3>(matrixListValue + (indexValue * Offsets::Transform_MatrixSize));
  auto maxTries = 50;
  auto tries = 0;
  auto transformIndexValue = Read<int>(matrixIndicesValue + (indexValue * Offsets::Transform_IndexSize));

  while (transformIndexValue >= 0 && tries < maxTries) {
    tries++;
    uintptr_t matrixItemPtr = matrixListValue + (transformIndexValue * Offsets::Transform_MatrixSize);
    if (!IsValidPointer(matrixItemPtr)) break;
    auto tMatrixValue = Read<Matrix>(matrixItemPtr);
    auto rotX = tMatrixValue.Rotation.X;
    auto rotY = tMatrixValue.Rotation.Y;
    auto rotZ = tMatrixValue.Rotation.Z;
    auto rotW = tMatrixValue.Rotation.W;
    auto scaleX = resultValue.X * tMatrixValue.Scale.X;
    auto scaleY = resultValue.Y * tMatrixValue.Scale.Y;
    auto scaleZ = resultValue.Z * tMatrixValue.Scale.Z;

    resultValue.X = tMatrixValue.Position.X + scaleX +
                    (scaleX * ((rotY * rotY * -2.0f) - (rotZ * rotZ * 2.0f))) +
                    (scaleY * ((rotW * rotZ * -2.0f) - (rotY * rotX * -2.0f))) +
                    (scaleZ * ((rotZ * rotX * 2.0f) - (rotW * rotY * -2.0f)));
    resultValue.Y = tMatrixValue.Position.Y + scaleY +
                    (scaleX * ((rotX * rotY * 2.0f) - (rotW * rotZ * -2.0f))) +
                    (scaleY * ((rotZ * rotZ * -2.0f) - (rotX * rotX * 2.0f))) +
                    (scaleZ * ((rotW * rotX * -2.0f) - (rotZ * rotY * -2.0f)));
    resultValue.Z = tMatrixValue.Position.Z + scaleZ +
                    (scaleX * ((rotW * rotY * -2.0f) - (rotX * rotZ * -2.0f))) +
                    (scaleY * ((rotY * rotZ * 2.0f) - (rotW * rotX * -2.0f))) +
                    (scaleZ * ((rotX * rotX * -2.0f) - (rotY * rotY * 2.0f)));

    uintptr_t indexItemPtr = matrixIndicesValue + (transformIndexValue * Offsets::Transform_IndexSize);
    if (!IsValidPointer(indexItemPtr)) break;
    transformIndexValue = Read<int>(indexItemPtr);
  }
  if (tries < maxTries) pos = resultValue;
  return pos;
}

static auto GetNodePosition(uintptr_t nodeTransform) {
  if (!IsValidPointer(nodeTransform)) return Vector3::Zero();
  auto transformValue = Read<uintptr_t>(nodeTransform + Offsets::Transform_TransformToObj);
  if (!IsValidPointer(transformValue)) return Vector3::Zero();
  return GetPosition(transformValue);
}

bool IsPlayerKnocked(uintptr_t player) {
    if (!IsValidPointer(player)) return false;
    uintptr_t headTF = Read<uintptr_t>(player + Offsets::Bones_Head);
    uintptr_t feetTF = Read<uintptr_t>(player + Offsets::Bones_Feet);
    if (!IsValidPointer(headTF) || !IsValidPointer(feetTF)) return false;
    Vector3 headPos = GetNodePosition(headTF);
    Vector3 feetPos = GetNodePosition(feetTF);
    if (feetPos.X == 0.0f && feetPos.Y == 0.0f && feetPos.Z == 0.0f) return false;
    float headFeetHeight = fabsf(headPos.Y - feetPos.Y);
    uintptr_t handTF = Read<uintptr_t>(player + Offsets::Bones_LeftHand);
    if (!IsValidPointer(handTF)) handTF = Read<uintptr_t>(player + Offsets::Bones_RightHand);
    uintptr_t chestTF = Read<uintptr_t>(player + Offsets::Bones_Chest);
    float handFeetHeight = 999.0f;
    if (IsValidPointer(handTF)) {
        Vector3 handPos = GetNodePosition(handTF);
        if (handPos.X != 0.0f || handPos.Y != 0.0f) handFeetHeight = fabsf(handPos.Y - feetPos.Y);
    }
    float headChestYDiff = 999.0f;
    if (IsValidPointer(chestTF)) {
        Vector3 chestPos = GetNodePosition(chestTF);
        if (chestPos.X != 0.0f || chestPos.Y != 0.0f) headChestYDiff = fabsf(headPos.Y - chestPos.Y);
    }
    if (headFeetHeight < 0.55f && handFeetHeight < 0.40f && headChestYDiff < 0.20f) return true;
    return false;
}

Vector3 GetBonePosition(uintptr_t player, uintptr_t offset) {
  if (!IsValidPointer(player)) return Vector3::Zero();
  uintptr_t bone = Read<uintptr_t>(player + offset);
  if (!IsValidPointer(bone)) return Vector3::Zero();
  return GetNodePosition(bone);
}

static auto WorldToScreenPoint(D3DMatrix viewMatrix, Vector3 ScreenPos) {
  auto result = Vector3(-1, -1, -1);
  auto v9 = (ScreenPos.X * viewMatrix._11) + (ScreenPos.Y * viewMatrix._21) +
            (ScreenPos.Z * viewMatrix._31) + viewMatrix._41;
  auto v10 = (ScreenPos.X * viewMatrix._12) + (ScreenPos.Y * viewMatrix._22) +
             (ScreenPos.Z * viewMatrix._32) + viewMatrix._42;
  auto v12 = (ScreenPos.X * viewMatrix._14) + (ScreenPos.Y * viewMatrix._24) +
             (ScreenPos.Z * viewMatrix._34) + viewMatrix._44;
  if (v12 >= 0.001f) {
    auto v13 = (float)abs_ScreenX / 2.0f;
    auto v14 = (float)abs_ScreenY / 2.0f;
    result.X = v13 + (v13 * v9) / v12;
    result.Y = v14 - (v14 * v10) / v12;
  }
  return result;
}

ImU32 GetHealthColor(int health) {
  if (health >= 160) return IM_COL32(58, 245, 24, 255);
  if (health >= 100) return IM_COL32(255, 230, 0, 255);
  if (health >= 50) return IM_COL32(255, 125, 0, 255);
  return IM_COL32(255, 30, 40, 255);
}

ImU32 GetDistanceColor(float distance) {
  if (distance < 50) return IM_COL32(255, 30, 40, 255);
  if (distance < 150) return IM_COL32(255, 230, 0, 255);
  return IM_COL32(58, 245, 24, 255);
}

ImU32 GetRainbowColor(float offset = 0.0f) {
  static float hue = 0.0f;
  hue += 0.005f;
  if (hue > 1.0f) hue -= 1.0f;
  ImColor color = ImColor::HSV(fmodf(hue + offset, 1.0f), 1.0f, 1.0f);
  return color;
}

ImU32 GetBoxColor(int health, float distance, bool isKnocked) {
  if (isKnocked) return IM_COL32(255, 0, 0, 255);
  switch (boxColorMode) {
  case 0: return ImGui::ColorConvertFloat4ToU32(boxCustomColor);
  case 1: return GetHealthColor(health);
  case 2: return GetDistanceColor(distance);
  case 3: return GetRainbowColor(0.0f);
  case 4: return IM_COL32(0, 255, 255, 255);
  default: return IM_COL32(255, 255, 255, 255);
  }
}

void DrawCornerBox(ImDrawList *draw, float x, float y, float width,
                   float height, ImU32 color, float thickness) {
  float cornerLen = width * 0.25f;
  draw->AddLine(ImVec2(x, y), ImVec2(x + cornerLen, y), color, thickness);
  draw->AddLine(ImVec2(x, y), ImVec2(x, y + cornerLen), color, thickness);
  draw->AddLine(ImVec2(x + width - cornerLen, y), ImVec2(x + width, y), color, thickness);
  draw->AddLine(ImVec2(x + width, y), ImVec2(x + width, y + cornerLen), color, thickness);
  draw->AddLine(ImVec2(x, y + height), ImVec2(x + cornerLen, y + height), color, thickness);
  draw->AddLine(ImVec2(x, y + height - cornerLen), ImVec2(x, y + height), color, thickness);
  draw->AddLine(ImVec2(x + width - cornerLen, y + height), ImVec2(x + width, y + height), color, thickness);
  draw->AddLine(ImVec2(x + width, y + height - cornerLen), ImVec2(x + width, y + height), color, thickness);
}

void DrawDashedBox(ImDrawList *draw, float x, float y, float width,
                   float height, ImU32 color, float thickness) {
  float dashLen = 8.0f;
  for (float tx = x; tx < x + width; tx += dashLen * 2) {
    draw->AddLine(ImVec2(tx, y), ImVec2(ImMin(tx + dashLen, x + width), y), color, thickness);
    draw->AddLine(ImVec2(tx, y + height), ImVec2(ImMin(tx + dashLen, x + width), y + height), color, thickness);
  }
  for (float ty = y; ty < y + height; ty += dashLen * 2) {
    draw->AddLine(ImVec2(x, ty), ImVec2(x, ImMin(ty + dashLen, y + height)), color, thickness);
    draw->AddLine(ImVec2(x + width, ty), ImVec2(x + width, ImMin(ty + dashLen, y + height)), color, thickness);
  }
}

void DrawDoubleBox(ImDrawList *draw, float x, float y, float width,
                   float height, ImU32 color, float thickness) {
  draw->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), color, 0.0f, 0, thickness);
  draw->AddRect(ImVec2(x + 3, y + 3), ImVec2(x + width - 3, y + height - 3), color, 0.0f, 0, thickness * 0.7f);
}

void Draw3DBox(ImDrawList *draw, float x, float y, float width, float height, ImU32 color) {
  ImU32 darkColor = (color & 0x00FFFFFF) | (int)((color >> 24) * 0.5f) << 24;
  draw->AddRectFilled(ImVec2(x, y), ImVec2(x + width, y + height), color);
  draw->AddRectFilled(ImVec2(x + 2, y + 2), ImVec2(x + width - 2, y + height - 2), darkColor);
  draw->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), IM_COL32(255, 255, 255, 255), 0.0f, 0, 1.0f);
}

void DrawMainOutlineBox(ImDrawList *draw, float x, float y, float width,
                        float height, int health, float distance, bool isKnocked) {
  ImU32 finalColor = GetBoxColor(health, distance, isKnocked);
  switch (boxStyle) {
  case 0: draw->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), finalColor, 0.0f, 0, boxThickness); break;
  case 1:
    draw->AddRectFilled(ImVec2(x, y), ImVec2(x + width, y + height), (finalColor & 0x00FFFFFF) | ((int)(boxFillAlpha * 255) << 24));
    draw->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), finalColor, 0.0f, 0, boxThickness); break;
  case 2:
    draw->AddRect(ImVec2(x - 1, y - 1), ImVec2(x + width + 1, y + height + 1), IM_COL32(0, 0, 0, 200), 0.0f, 0, boxThickness + 1);
    draw->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), finalColor, 0.0f, 0, boxThickness); break;
  case 3: DrawCornerBox(draw, x, y, width, height, finalColor, boxThickness); break;
  case 4: draw->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), finalColor, 0.0f, 0, boxThickness); break;
  case 5: DrawDashedBox(draw, x, y, width, height, finalColor, boxThickness); break;
  case 6: DrawDoubleBox(draw, x, y, width, height, finalColor, boxThickness); break;
  case 7: draw->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), finalColor, boxCornerRound, 0, boxThickness); break;
  case 8: Draw3DBox(draw, x, y, width, height, finalColor); break;
  }
}

void DrawAdvancedHealthBar(ImDrawList *draw, float x, float y, float width,
                           float height, int health) {
  float healthPercent = ImClamp(health / 200.0f, 0.0f, 1.0f);
  float fillHeight = height * healthPercent;
  float fillY = y + (height - fillHeight);

  draw->AddRectFilled(ImVec2(x, y), ImVec2(x + width, y + height),
                      IM_COL32(10, 11, 15, 180), healthBarRounding);
  ImU32 healthColor = showHealthColorGradient ? GetHealthColor(health)
                                              : IM_COL32(58, 245, 24, 255);

  if (fillHeight > 0.0f) {
    draw->AddRectFilled(ImVec2(x, fillY), ImVec2(x + width, fillY + fillHeight),
                        healthColor, healthBarRounding);
    draw->AddLine(ImVec2(x + 1.0f, fillY + 1.0f),
                  ImVec2(x + 1.0f, y + height - 1.0f),
                  IM_COL32(255, 255, 255, 140), 1.0f);
  }

  draw->AddRect(ImVec2(x, y), ImVec2(x + width, y + height),
                IM_COL32(5, 5, 8, 220), healthBarRounding, 0, 1.0f);

  if (showHealthText && health > 0) {
    char ht[16];
    sprintf(ht, "%d", health);
    draw->AddText(NULL, 15.0f, ImVec2(x - 24.0f, y + (height / 2.0f) - 6.0f),
                  IM_COL32(255, 255, 255, 255), ht);
  }
}

ImVec2 GetLineStartPosition(float sw) {
  switch (lineStartPosition) {
  case 1: return ImVec2(sw / 2.0f, sw / 2.0f);
  case 2: return ImVec2(sw / 2.0f, sw - 80.0f);
  default: return ImVec2(sw / 2.0f, 80.0f);
  }
}

ImU32 GetLineColor(int health, float distance, bool isKnocked) {
  if (isKnocked) return IM_COL32(255, 0, 0, 255);
  switch (lineColorMode) {
  case 0: return ImGui::ColorConvertFloat4ToU32(lineCustomColor);
  case 1: return GetHealthColor(health);
  case 2: return GetDistanceColor(distance);
  case 3: return GetRainbowColor(0.3f);
  case 4: return IM_COL32(255, 255, 0, 255);
  default: return IM_COL32(255, 255, 255, 255);
  }
}

void DrawAdvancedLine(ImDrawList *draw, ImVec2 from, ImVec2 to, float distance,
                      int health, bool isKnocked) {
  ImU32 finalColor = GetLineColor(health, distance, isKnocked);
  switch (lineStyle) {
  case 0: draw->AddLine(from, to, finalColor, lineThickness); break;
  case 1: {
    float length = sqrt(pow(to.x - from.x, 2) + pow(to.y - from.y, 2));
    for (float t = 0; t < length; t += 8.0f) {
      float tx = from.x + (to.x - from.x) * (t / length);
      float ty = from.y + (to.y - from.y) * (t / length);
      draw->AddCircleFilled(ImVec2(tx, ty), 2.0f, finalColor);
    }
  } break;
  case 2: {
    float dashLen = 15.0f;
    float totalLen = sqrt(pow(to.x - from.x, 2) + pow(to.y - from.y, 2));
    for (int i = 0; i < (int)(totalLen / dashLen); i++) {
      float t1 = i * dashLen / totalLen;
      float t2 = (i * dashLen + dashLen / 2) / totalLen;
      if (t2 > 1.0f) t2 = 1.0f;
      draw->AddLine(ImVec2(from.x + (to.x - from.x) * t1, from.y + (to.y - from.y) * t1),
                    ImVec2(from.x + (to.x - from.x) * t2, from.y + (to.y - from.y) * t2),
                    finalColor, lineThickness);
    }
  } break;
  case 4:
    for (float t = 0; t <= 1.0f; t += 0.05f) {
      draw->AddCircleFilled(ImVec2(from.x + (to.x - from.x) * t, from.y + (to.y - from.y) * t),
                            lineThickness, GetRainbowColor(t));
    } break;
  case 5: draw->AddLine(from, to, finalColor, lineThickness); break;
  case 6:
    draw->AddLine(from, to, finalColor, lineThickness);
    {
      ImVec2 dir = ImVec2(to.x - from.x, to.y - from.y);
      float len = sqrt(dir.x * dir.x + dir.y * dir.y);
      if (len > 0) {
        dir.x /= len; dir.y /= len;
        ImVec2 perp = ImVec2(-dir.y, dir.x);
        ImVec2 arrowBase = ImVec2(to.x - dir.x * 15, to.y - dir.y * 15);
        draw->AddLine(to, ImVec2(arrowBase.x + perp.x * 8, arrowBase.y + perp.y * 8), finalColor, lineThickness);
        draw->AddLine(to, ImVec2(arrowBase.x - perp.x * 8, arrowBase.y - perp.y * 8), finalColor, lineThickness);
      }
    } break;
  case 7:
    draw->AddLine(from, to, finalColor, lineThickness);
    draw->AddLine(ImVec2(from.x + 2, from.y + 2), ImVec2(to.x + 2, to.y + 2), finalColor, lineThickness * 0.5f);
    break;
  case 8:
    draw->AddLine(from, to, finalColor, lineThickness);
    draw->AddLine(ImVec2(from.x - 1, from.y - 1), ImVec2(to.x - 1, to.y - 1), IM_COL32(0, 255, 255, 100), lineThickness);
    draw->AddLine(ImVec2(from.x + 1, from.y + 1), ImVec2(to.x + 1, to.y + 1), IM_COL32(255, 0, 255, 100), lineThickness);
    break;
  case 9:
    draw->AddLine(from, to, finalColor, 1.0f);
    draw->AddLine(ImVec2(from.x, from.y + 1), ImVec2(to.x, to.y + 1), IM_COL32(255, 0, 0, 200), 1.0f);
    break;
  }
}

bool isOutsideScreen(const Vec2 &pos, const Vec2 &screen) {
  if (pos.y < 0) return true;
  if (pos.x > screen.x) return true;
  if (pos.y > screen.y) return true;
  return pos.x < 0;
}

Vec2 pushToScreenBorder(const Vec2 &Pos, const Vec2 &screen, float offset) {
  float x = Pos.x, y = Pos.y;
  if (Pos.y < 0) y = -offset;
  if (Pos.x > screen.x) x = screen.x + offset;
  if (Pos.y > screen.y) y = screen.y + offset;
  if (Pos.x < 0) x = -offset;
  return Vec2(x, y);
}

void Draw360Alert(ImDrawList *draw, Vector3 headPos, float distance, float sw,
                  float sh, bool isKnocked) {
  if (!is3600) return;
  if (headPos.X < 0 || headPos.Y < 0) return;
  Vec2 screen(sw, sh);
  Vec2 headPos2D(headPos.X, headPos.Y);
  if (isOutsideScreen(headPos2D, screen)) {
    Vec2 hintDotRenderPos = pushToScreenBorder(headPos2D, screen, -50);
    float radius = 35.0f;
    ImU32 circleColor = IM_COL32(255, 0, 0, 200);
    draw->AddCircleFilled(ImVec2(hintDotRenderPos.x, hintDotRenderPos.y), radius, circleColor);
    draw->AddCircle(ImVec2(hintDotRenderPos.x, hintDotRenderPos.y), radius, IM_COL32(255, 255, 255, 255), 0, 2.5f);
    std::string strDistance = "[" + std::to_string((int)distance) + "M]";
    ImVec2 textSize = ImGui::CalcTextSize(strDistance.c_str());
    draw->AddText(NULL, 18.0f, ImVec2(hintDotRenderPos.x - textSize.x / 2, hintDotRenderPos.y - textSize.y / 2),
                  IM_COL32(255, 255, 255, 255), strDistance.c_str());
    float angle = atan2(headPos2D.y - sh / 2, headPos2D.x - sw / 2);
    float arrowX = hintDotRenderPos.x + cos(angle) * (radius + 10);
    float arrowY = hintDotRenderPos.y + sin(angle) * (radius + 10);
    draw->AddTriangleFilled(ImVec2(arrowX, arrowY), ImVec2(arrowX - 8, arrowY - 5), ImVec2(arrowX - 8, arrowY + 5),
                            IM_COL32(255, 255, 255, 255));
  }
}

Vector3 GetTargetPosition(uintptr_t enemy) {
  if (!IsValidPointer(enemy)) return Vector3::Zero();
  uintptr_t boneBase = 0;
  Vector3 offset(0, 0, 0);
  switch (selectedBoneIndex) {
  case 0: boneBase = Read<uintptr_t>(enemy + Offsets::Bones_Head); break;
  case 1: boneBase = Read<uintptr_t>(enemy + Offsets::Bones_Hip); offset = Vector3(0, 0.1f, 0); break;
  default: boneBase = Read<uintptr_t>(enemy + Offsets::Bones_Head); break;
  }
  if (!IsValidPointer(boneBase)) return Vector3::Zero();
  Vector3 bonePos = GetNodePosition(boneBase);
  return bonePos + offset;
}

float GetFOVRadius(float screenHeight, float fovValue) {
  return (fovValue / 360.0f) * (screenHeight / 2.0f);
}

float GetFOVRadius(float screenHeight) {
  return GetFOVRadius(screenHeight, AimFov);
}

void DrawFOVCircle(ImDrawList *draw, ImVec2 center, float radius) {
  constexpr int NUM_SEGMENTS = 100;
  draw->AddCircle(center, radius, IM_COL32(255, 255, 255, 220), NUM_SEGMENTS, 2.5f);
}

void DrawSkeletonESP(ImDrawList *draw, uintptr_t enemy, D3DMatrix matrix, bool isKnocked) {
  if (!showSkeleton || !IsValidPointer(enemy)) return;

  Vector3 head = GetBonePosition(enemy, Offsets::Bones_Head);
  Vector3 chest = GetBonePosition(enemy, Offsets::Bones_Chest);
  Vector3 hip = GetBonePosition(enemy, Offsets::Bones_Hip);
  Vector3 leftShoulder = GetBonePosition(enemy, Offsets::Bones_LeftShoulder);
  Vector3 rightShoulder = GetBonePosition(enemy, Offsets::Bones_RightShoulder);
  Vector3 leftHand = GetBonePosition(enemy, Offsets::Bones_LeftHand);
  Vector3 rightHand = GetBonePosition(enemy, Offsets::Bones_RightHand);
  Vector3 leftAnkle = GetBonePosition(enemy, Offsets::Bones_LeftAnkle);
  Vector3 rightAnkle = GetBonePosition(enemy, Offsets::Bones_RightAnkle);

  Vector3 sHead = WorldToScreenPoint(matrix, head);
  Vector3 sChest = WorldToScreenPoint(matrix, chest);
  Vector3 sHip = WorldToScreenPoint(matrix, hip);
  Vector3 sLeftShoulder = WorldToScreenPoint(matrix, leftShoulder);
  Vector3 sRightShoulder = WorldToScreenPoint(matrix, rightShoulder);
  Vector3 sLeftHand = WorldToScreenPoint(matrix, leftHand);
  Vector3 sRightHand = WorldToScreenPoint(matrix, rightHand);
  Vector3 sLeftAnkle = WorldToScreenPoint(matrix, leftAnkle);
  Vector3 sRightAnkle = WorldToScreenPoint(matrix, rightAnkle);

  ImU32 skColor = isKnocked ? IM_COL32(255, 0, 0, 255) : IM_COL32(255, 30, 40, 255);
  float thick = 1.8f;

  auto DrawBoneSegment = [&](ImVec2 from, ImVec2 to) {
    if (from.x <= 0 || from.y <= 0 || to.x <= 0 || to.y <= 0) return;
    if (from.x > abs_ScreenX || from.y > abs_ScreenY || to.x > abs_ScreenX || to.y > abs_ScreenY) return;
    draw->AddLine(from, to, skColor, thick);
  };

  auto DrawJointDot = [&](ImVec2 pos) {
    if (pos.x <= 0 || pos.y <= 0) return;
    if (pos.x > abs_ScreenX || pos.y > abs_ScreenY) return;
    draw->AddCircleFilled(pos, 3.2f, IM_COL32(255, 255, 255, 255));
    draw->AddCircle(pos, 3.2f, isKnocked ? IM_COL32(255, 0, 0, 220) : IM_COL32(255, 30, 40, 220), 0, 1.0f);
  };

  DrawBoneSegment(ImVec2(sHead.X, sHead.Y), ImVec2(sChest.X, sChest.Y));
  DrawBoneSegment(ImVec2(sChest.X, sChest.Y), ImVec2(sHip.X, sHip.Y));
  DrawBoneSegment(ImVec2(sChest.X, sChest.Y), ImVec2(sLeftShoulder.X, sLeftShoulder.Y));
  DrawBoneSegment(ImVec2(sChest.X, sChest.Y), ImVec2(sRightShoulder.X, sRightShoulder.Y));
  DrawBoneSegment(ImVec2(sLeftShoulder.X, sLeftShoulder.Y), ImVec2(sLeftHand.X, sLeftHand.Y));
  DrawBoneSegment(ImVec2(sRightShoulder.X, sRightShoulder.Y), ImVec2(sRightHand.X, sRightHand.Y));
  DrawBoneSegment(ImVec2(sHip.X, sHip.Y), ImVec2(sLeftAnkle.X, sLeftAnkle.Y));
  DrawBoneSegment(ImVec2(sHip.X, sHip.Y), ImVec2(sRightAnkle.X, sRightAnkle.Y));

  DrawJointDot(ImVec2(sHead.X, sHead.Y));
  DrawJointDot(ImVec2(sChest.X, sChest.Y));
  DrawJointDot(ImVec2(sHip.X, sHip.Y));
  DrawJointDot(ImVec2(sLeftShoulder.X, sLeftShoulder.Y));
  DrawJointDot(ImVec2(sRightShoulder.X, sRightShoulder.Y));
  DrawJointDot(ImVec2(sLeftHand.X, sLeftHand.Y));
  DrawJointDot(ImVec2(sRightHand.X, sRightHand.Y));
  DrawJointDot(ImVec2(sLeftAnkle.X, sLeftAnkle.Y));
  DrawJointDot(ImVec2(sRightAnkle.X, sRightAnkle.Y));
}

namespace aimsilent {
    inline bool enabled = false;
    inline std::atomic<bool> thread_started{false};

    inline bool IsValidPointer(uintptr_t ptr) {
        return (ptr >= 0x10000000 && ptr <= 0x7FFFFFFFFFFFFFFF);
    }

    static void Run() {
        if (thread_started.exchange(true)) return;

        static uintptr_t lockedEnemy = 0;
        static bool isHeadshot = true;
        static auto lastSwitchTime = std::chrono::steady_clock::now();

        while (track) {
            if (!enabled || !AimSilent) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            uintptr_t lp = g_LocalPlayer.load();
            uintptr_t currentTarget = g_CurrentTargetObj.load();

            if (!IsValidPointer(lp) || !IsValidPointer(currentTarget) || Read<bool>(currentTarget + Offsets::Player_IsDead)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            uintptr_t aimingInfo = Read<uintptr_t>(lp + Offsets::Player_AimingInfo);
            if (!IsValidPointer(aimingInfo)) aimingInfo = Read<uintptr_t>(lp + Offsets::sAim2);

            if (IsValidPointer(aimingInfo)) {
                int hitRate = SilentAimPercentage;
                if (hitRate > 100) hitRate = 100;
                if (hitRate < 0) hitRate = 0;

                auto now = std::chrono::steady_clock::now();
                if (currentTarget != lockedEnemy || std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSwitchTime).count() > 200) {
                    isHeadshot = (rand() % 100) < hitRate;
                    lockedEnemy = currentTarget;
                    lastSwitchTime = now;
                }

                uintptr_t targetNode = isHeadshot ? Read<uintptr_t>(currentTarget + Offsets::Bones_Head)
                                                  : Read<uintptr_t>(currentTarget + Offsets::Bones_Chest);

                if (IsValidPointer(targetNode)) {
                    Vector3 targetPos = GetNodePosition(targetNode);
                    if (targetPos.X != 0 || targetPos.Y != 0 || targetPos.Z != 0) {
                        if (isHeadshot) targetPos.Y += 0.04f;
                        else targetPos.Y -= 0.08f;

                        Vector3 startPos = Read<Vector3>(aimingInfo + Offsets::Player_AimingInfoStartPos);
                        if (startPos.X == 0 && startPos.Y == 0 && startPos.Z == 0)
                            startPos = Read<Vector3>(aimingInfo + Offsets::sAim3);

                        Vector3 hookDir;
                        hookDir.X = targetPos.X - startPos.X;
                        hookDir.Y = targetPos.Y - startPos.Y;
                        hookDir.Z = targetPos.Z - startPos.Z;

                        Write<Vector3>(aimingInfo + Offsets::Player_AimingInfoDirection, hookDir);
                        Write<Vector3>(aimingInfo + Offsets::sAim4, hookDir);
                    }
                }
            }
            std::this_thread::yield();
        }
    }

    inline void Start() {
        static bool started = false;
        if (!started) { started = true; std::thread(Run).detach(); }
    }
}

inline void StartAimSilent() { aimsilent::Start(); }

static int TouchFindMinTarget() {
    std::lock_guard<std::mutex> lock(g_TouchAimMtx);
    if (g_TouchLockedPawn != 0) {
        for (int i = 0; i < TouchAimCount; i++) {
            if (TouchAim[i].PawnAddr == g_TouchLockedPawn) {
                if (TouchAim[i].ScreenDistance <= TouchCfg.FovRange) return i;
                g_TouchLockedPawn = 0;
                break;
            }
        }
    }

    float min = 1.0e30f;
    int minAt = -1;

    for (int i = 0; i < TouchAimCount; i++) {
        if (TouchAim[i].ScreenDistance <= TouchCfg.FovRange) {
            float score = (TouchCfg.AimTarget == 0) ? TouchAim[i].ScreenDistance : TouchAim[i].WodDistance;
            if (score < min) { min = score; minAt = i; }
        }
    }

    if (minAt != -1) g_TouchLockedPawn = TouchAim[minAt].PawnAddr;
    else             g_TouchLockedPawn = 0;
    return minAt;
}

void TouchAimbotThread() {
    bool isDown = false;
    float tx = 0.0f, ty = 0.0f;
    static float accumX = 0.0f, accumY = 0.0f;
    static uintptr_t lastLockedPawn = 0;

    while (main_thread_flag) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if (!TouchCfg.EnableAim) {
            if (isDown) { Touch_Up_Slot(6); isDown = false; accumX = accumY = 0; }
            g_TouchLockedPawn = 0;
            lastLockedPawn = 0;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        D3DMatrix localMatrix;
        {
            std::lock_guard<std::mutex> lock(g_TouchAimMtx);
            localMatrix = g_TouchViewMatrix;
        }

        bool isScopeActive = (fabsf(localMatrix._13) > 1.5f ||
                              fabsf(localMatrix._11) > 2.0f ||
                              fabsf(localMatrix._22) > 2.0f);

        bool shouldAim = false;
        if      (TouchCfg.TriggerMode == 0) shouldAim = true;
        else if (TouchCfg.TriggerMode == 1) shouldAim = g_TouchAimFire;
        else if (TouchCfg.TriggerMode == 2) shouldAim = isScopeActive;
        else if (TouchCfg.TriggerMode == 3) shouldAim = g_TouchAimFire || isScopeActive;

        if (!shouldAim) {
            if (isDown) { Touch_Up_Slot(6); isDown = false; accumX = accumY = 0; }
            continue;
        }

        int targetIdx = TouchFindMinTarget();
        if (targetIdx == -1) {
            if (isDown) { Touch_Up_Slot(6); isDown = false; accumX = accumY = 0; }
            lastLockedPawn = 0;
            continue;
        }

        uintptr_t currentPawn = 0;
        Vector3 targetPos;
        {
            std::lock_guard<std::mutex> lock(g_TouchAimMtx);
            if (targetIdx >= 0 && targetIdx < TouchAimCount) {
                currentPawn = TouchAim[targetIdx].PawnAddr;
                targetPos   = TouchAim[targetIdx].ObjAim;
            }
        }

        if (currentPawn == 0) {
            if (isDown) { Touch_Up_Slot(6); isDown = false; accumX = accumY = 0; }
            lastLockedPawn = 0;
            continue;
        }

        if (currentPawn != lastLockedPawn) {
            if (isDown) {
                Touch_Up_Slot(6);
                isDown = false;
                accumX = accumY = 0;
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
            lastLockedPawn = currentPawn;
        }

        targetPos.Y += TouchCfg.AimZ;

        Vector3 screenPos = WorldToScreenPoint(localMatrix, targetPos);

        if (screenPos.X > 0 && screenPos.Y > 0) {
            float screenCenterX = (float)abs_ScreenX / 2.0f;
            float screenCenterY = (float)abs_ScreenY / 2.0f;

            float errorX    = screenPos.X - screenCenterX;
            float errorY    = screenPos.Y - screenCenterY;
            float errorDist = sqrtf(errorX * errorX + errorY * errorY);

            if (errorDist > TouchCfg.FovRange) {
                if (isDown) { Touch_Up_Slot(6); isDown = false; accumX = accumY = 0; }
                lastLockedPawn = 0;
                g_TouchLockedPawn = 0;
                continue;
            }

            float anchorX = TouchCfg.TouchPointX * abs_ScreenX;
            float anchorY = TouchCfg.TouchPointY * abs_ScreenY;

            if (!isDown) {
                tx = anchorX;
                ty = anchorY;
                Touch_Down_Slot(6, tx, ty);
                isDown = true;
                accumX = accumY = 0;
                std::this_thread::sleep_for(std::chrono::milliseconds(3));
            }

            float smoothness = TouchCfg.AimSpeed;
            if (smoothness < 1.0f) smoothness = 1.0f;
            float stepFactor = 1.0f / (smoothness * 2.5f);

            if (errorDist < 12.0f) stepFactor *= (errorDist / 12.0f);
            if (errorDist < 0.8f)  stepFactor = 0.0f;

            float targetStepX = errorX * stepFactor;
            float targetStepY = errorY * stepFactor;

            accumX += targetStepX;
            accumY += targetStepY;
            int moveX = (int)accumX;
            int moveY = (int)accumY;
            accumX -= (float)moveX;
            accumY -= (float)moveY;

            tx += (float)moveX;
            ty += (float)moveY;

            float activeTouchArea = (TouchCfg.TouchSize > 50.0f) ? TouchCfg.TouchSize : 350.0f;
            if (tx > anchorX + activeTouchArea || tx < anchorX - activeTouchArea ||
                ty > anchorY + activeTouchArea || ty < anchorY - activeTouchArea) {
                Touch_Up_Slot(6);
                tx = anchorX;
                ty = anchorY;
                Touch_Down_Slot(6, tx, ty);
                accumX = accumY = 0;
            } else {
                if (moveX != 0 || moveY != 0) {
                    Touch_Move_Slot(6, tx, ty);
                }
            }
        } else {
            if (isDown) { Touch_Up_Slot(6); isDown = false; accumX = accumY = 0; }
        }
    }
}

void DrawTouchPointVisual() {
    if (!TouchCfg.ShowTouchPoint || !TouchCfg.EnableAim) return;
    ImDrawList* draw = ImGui::GetForegroundDrawList();
    ImVec2 center(abs_ScreenX * TouchCfg.TouchPointX, abs_ScreenY * TouchCfg.TouchPointY);

    float pulse  = 0.5f + 0.5f * sinf(ImGui::GetTime() * 4.0f);
    float radius = 34.0f + pulse * 5.0f;

    draw->AddCircleFilled(center, radius + 10.0f, IM_COL32(0, 180, 255, 18));
    draw->AddCircle(center, radius + 8.0f, IM_COL32(0, 180, 255, 70), 48, 2.0f);
    draw->AddCircle(center, radius, IM_COL32(0, 220, 255, 220), 48, 2.5f);
    draw->AddCircle(center, 7.0f, IM_COL32(255, 255, 255, 230), 24, 2.0f);
    draw->AddCircleFilled(center, 3.0f, IM_COL32(0, 220, 255, 255));

    float line = radius + 15.0f;
    ImU32 guide = IM_COL32(0, 220, 255, 180);
    draw->AddLine(ImVec2(center.x - line, center.y), ImVec2(center.x - radius - 4.0f, center.y), guide, 2.0f);
    draw->AddLine(ImVec2(center.x + radius + 4.0f, center.y), ImVec2(center.x + line, center.y), guide, 2.0f);
    draw->AddLine(ImVec2(center.x, center.y - line), ImVec2(center.x, center.y - radius - 4.0f), guide, 2.0f);
    draw->AddLine(ImVec2(center.x, center.y + radius + 4.0f), ImVec2(center.x, center.y + line), guide, 2.0f);
    draw->AddText(ImVec2(center.x - 42.0f, center.y + line + 5.0f), IM_COL32(220, 245, 255, 220), "AIM TOUCH");
}

void DrawFireButtonVisual() {
    if (!TouchCfg.ShowFireButton || !TouchCfg.EnableAim) return;
    ImDrawList* draw = ImGui::GetForegroundDrawList();
    ImVec2 center(abs_ScreenX * TouchCfg.FireButtonX, abs_ScreenY * TouchCfg.FireButtonY);

    float pulse  = 0.5f + 0.5f * sinf(ImGui::GetTime() * 5.0f);
    float radius = TouchCfg.FireButtonSize + pulse * 3.0f;

    ImU32 colMain = g_TouchAimFire ? IM_COL32(0, 255, 120, 230) : IM_COL32(255, 255, 255, 160);
    ImU32 colGlow = g_TouchAimFire ? IM_COL32(0, 255, 120, 40)  : IM_COL32(0, 200, 255, 20);

    draw->AddCircleFilled(center, radius + 15.0f, colGlow);
    draw->AddCircle(center, radius + 5.0f,
                    g_TouchAimFire ? IM_COL32(0, 255, 100, 180) : IM_COL32(200, 200, 200, 100), 64, 2.0f);
    draw->AddCircleFilled(center, radius, IM_COL32(20, 20, 20, 150));
    draw->AddCircle(center, radius, colMain, 64, 3.5f);

    const char* icon = g_TouchAimFire ? "FIRE" : "OFF";
    ImVec2 iconSize = ImGui::CalcTextSize(icon);
    draw->AddText(ImVec2(center.x - iconSize.x / 2.0f, center.y - iconSize.y / 2.0f), colMain, icon);

    const char* label = "FIRE TRIGGER";
    ImVec2 labelSize = ImGui::CalcTextSize(label);
    draw->AddText(ImVec2(center.x - labelSize.x / 2.0f, center.y + radius + 10.0f), IM_COL32(255, 255, 255, 200), label);
}

void DrawTouchFovCircle() {
    if (!TouchCfg.EnableAim) return;
    ImDrawList* draw = ImGui::GetForegroundDrawList();
    draw->AddCircle(ImVec2(abs_ScreenX / 2.0f, abs_ScreenY / 2.0f), TouchCfg.FovRange,
                    IM_COL32(255, 255, 255, 150), 100, 2.0f);
}

void HandleTouchAimbotDragging(bool is_menu_open) {
    if (!TouchCfg.EnableAim) return;

    ImGuiIO& io = ImGui::GetIO();
    static bool draggingFire  = false;
    static bool draggingTouch = false;

    ImVec2 fireCenter(abs_ScreenX * TouchCfg.FireButtonX, abs_ScreenY * TouchCfg.FireButtonY);
    ImVec2 touchCenter(abs_ScreenX * TouchCfg.TouchPointX, abs_ScreenY * TouchCfg.TouchPointY);

    float distToFire  = sqrtf(pow(io.MousePos.x - fireCenter.x, 2) + pow(io.MousePos.y - fireCenter.y, 2));
    float distToTouch = sqrtf(pow(io.MousePos.x - touchCenter.x, 2) + pow(io.MousePos.y - touchCenter.y, 2));

    if (io.MouseDown[0]) {
        if (is_menu_open && !TouchCfg.LockFireButton && distToFire < 60.0f && !draggingTouch) draggingFire = true;
        if (is_menu_open && !TouchCfg.LockTouchPoint && distToTouch < 60.0f && !draggingFire) draggingTouch = true;

        if (draggingFire) {
            TouchCfg.FireButtonX = io.MousePos.x / (float)abs_ScreenX;
            TouchCfg.FireButtonY = io.MousePos.y / (float)abs_ScreenY;
        } else if (draggingTouch) {
            TouchCfg.TouchPointX = io.MousePos.x / (float)abs_ScreenX;
            TouchCfg.TouchPointY = io.MousePos.y / (float)abs_ScreenY;
        }
    } else {
        draggingFire  = false;
        draggingTouch = false;
    }
}

void UpdateTouchFireState(bool is_menu_open) {
    if (!TouchCfg.EnableAim) { g_TouchAimFire = false; return; }

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 fireCenter(abs_ScreenX * TouchCfg.FireButtonX, abs_ScreenY * TouchCfg.FireButtonY);
    float distToFire = sqrtf(pow(io.MousePos.x - fireCenter.x, 2) + pow(io.MousePos.y - fireCenter.y, 2));

    bool fireTriggered = false;

    if (!is_menu_open) {
        for (int i = 0; i < 10; i++) {
            if (IsFingerDown(i)) {
                float fx, fy;
                if (GetFingerScreenPos(i, fx, fy)) {
                    float fDistFire = sqrtf(pow(fx - fireCenter.x, 2) + pow(fy - fireCenter.y, 2));
                    if (TouchCfg.ShowFireButton && fDistFire < (TouchCfg.FireButtonSize + 50.0f)) {
                        fireTriggered = true;
                        break;
                    }
                }
            }
        }

        if (!fireTriggered && io.MouseDown[0]) {
            if (TouchCfg.ShowFireButton && distToFire < (TouchCfg.FireButtonSize + 50.0f))
                fireTriggered = true;
        }
    }

    g_TouchAimFire = fireTriggered;
}

void TouchAimbotTick(bool is_menu_open) {
    HandleTouchAimbotDragging(is_menu_open);
    UpdateTouchFireState(is_menu_open);
    DrawTouchPointVisual();
    DrawFireButtonVisual();
    DrawTouchFovCircle();
}

void InitTouchAimbot() {
    static bool started = false;
    if (!started) { started = true; std::thread(TouchAimbotThread).detach(); }
}

void ApplySpeedHack(uintptr_t BaseGame) {
  if (!IsValidPointer(BaseGame)) return;
  auto TimeService = Read<uintptr_t>(BaseGame + Offsets::Engine_TimeService);
  if (!IsValidPointer(TimeService)) return;

  if (SpeedHackV2) {
    float mappedSpeed = 0.033f + ((SpeedMultiplier - 1.0f) / 4.0f) * 0.026f;
    Write<float>(TimeService + Offsets::Engine_TimeServiceSpeed, mappedSpeed);
  } else {
    Write<float>(TimeService + Offsets::Engine_TimeServiceSpeed, 0.033f);
  }
}

void AimbotMenu(float sw, float sh, uintptr_t libAddress) {
  ImDrawList *draw = ImGui::GetBackgroundDrawList();
  if (!Enable) return;

  if (!IsValidPointer(libAddress)) return;
  auto GameFacade_c = Read<uintptr_t>(libAddress + Offsets::GameFacade);
  if (!IsValidPointer(GameFacade_c)) return;

  auto P2 = Read<uintptr_t>(GameFacade_c + Offsets::Engine_BaseGamePtr);
  if (!IsValidPointer(P2)) return;
  auto BaseGame = Read<uintptr_t>(P2);
  if (!IsValidPointer(BaseGame)) return;

  ApplySpeedHack(BaseGame);

  auto m_Match = Read<uintptr_t>(BaseGame + Offsets::Engine_Match);
  if (!IsValidPointer(m_Match)) return;

  int MatchIsRunning = Read<int>(m_Match + Offsets::MatchIsRunning);
  if (MatchIsRunning != 1) return;

  auto localPlayer = Read<uintptr_t>(m_Match + Offsets::Match_LocalPlayer);

  static uintptr_t s_stableLocalPlayerAim = 0;
  static uintptr_t s_lastBaseGameAim = 0;

  if (BaseGame != s_lastBaseGameAim) {
    s_lastBaseGameAim = BaseGame;
    s_stableLocalPlayerAim = 0;
  }

  if (localPlayer) {
    if (s_stableLocalPlayerAim == 0) {
      s_stableLocalPlayerAim = localPlayer;
    } else {
      auto oldAvatar = Read<uintptr_t>(s_stableLocalPlayerAim + Offsets::Player_AvatarManager);
      if (IsValidPointer(oldAvatar)) {
        localPlayer = s_stableLocalPlayerAim;
      } else {
        s_stableLocalPlayerAim = localPlayer;
      }
    }
  } else {
    s_stableLocalPlayerAim = 0;
  }

  if (!IsValidPointer(localPlayer)) return;

  auto Ghost = localPlayer + Offsets::GhostHack;
  if (GhostHack) {
    Write<bool>(Ghost, true);
  } else {
    Write<bool>(Ghost, false);
  }

  UpdateNoReload(localPlayer);
  UpdatePlayerAttributes(localPlayer);
  UpdateNoRecoil(localPlayer);

  auto FollowCamera = Read<uintptr_t>(localPlayer + Offsets::Cam_FollowCamera);
  if (!IsValidPointer(FollowCamera)) return;
  auto Camera = Read<uintptr_t>(FollowCamera + Offsets::Cam_CameraObject);
  if (!IsValidPointer(Camera)) return;
  auto IntPtrCam = Read<uintptr_t>(Camera + Offsets::Cam_RawCameraData);
  if (!IsValidPointer(IntPtrCam)) return;
  auto matrix = Read<D3DMatrix>(IntPtrCam + Offsets::Cam_ViewMatrix);

  {
    std::lock_guard<std::mutex> lock(g_TouchAimMtx);
    g_TouchViewMatrix = matrix;
  }

  auto dictionary = Read<uintptr_t>(BaseGame + Offsets::Engine_Dictionary);
  if (!IsValidPointer(dictionary)) return;
  auto entitylist = Read<uintptr_t>(dictionary + Offsets::Engine_EntityList);
  if (!IsValidPointer(entitylist)) return;

  Vector3 LocalPosition = GetNodePosition(GetPlayerHeadTF(localPlayer));
  uintptr_t bestTarget = 0;
  Vector3 bestPos;
  float bestDist = FLT_MAX;

  for (int i = 0; i < 1000; i++) {
    uintptr_t enemyPtr = entitylist + i * 8;
    if (!IsValidPointer(enemyPtr)) break;

    auto enemy = Read<uintptr_t>(enemyPtr);
    if (!IsValidPointer(enemy) || enemy == localPlayer) continue;

    auto AvatarManager = Read<uintptr_t>(enemy + Offsets::Player_AvatarManager);
    if (!IsValidPointer(AvatarManager)) continue;
    auto UmaAvatarSimple = Read<uintptr_t>(AvatarManager + Offsets::Avatar_UmaSimple);
    if (!IsValidPointer(UmaAvatarSimple)) continue;

    if (!Read<bool>(UmaAvatarSimple + Offsets::Avatar_IsVisible2)) continue;

    auto UmaData = Read<uintptr_t>(UmaAvatarSimple + Offsets::Avatar_UmaData);
    if (!IsValidPointer(UmaData)) continue;

    if (Read<bool>(UmaData + Offsets::Avatar_IsDead)) continue;

    bool isKnocked = IsPlayerKnocked(enemy);
    if (Ignorknock && isKnocked) continue;

    int enemyHealth = 0;
    uintptr_t hp_step1 = Read<uintptr_t>(enemy + Offsets::Player_HealthChain1);
    if (IsValidPointer(hp_step1)) {
      uintptr_t hp_step2 = Read<uintptr_t>(hp_step1 + Offsets::Player_HealthChain2);
      if (IsValidPointer(hp_step2)) {
        uintptr_t hp_step3 = Read<uintptr_t>(hp_step2 + Offsets::Player_HealthChain3);
        if (IsValidPointer(hp_step3)) {
          enemyHealth = Read<int>(hp_step3 + Offsets::Player_HealthValue);
        }
      }
    }
    if (enemyHealth <= 0) continue;

    if (Read<bool>(enemy + Offsets::Player_IsSpectating)) continue;

    if (IgnoreBot && Read<int>(enemy + Offsets::Player_ClientBotCheck) == 1) continue;

    auto HedColider = Read<uintptr_t>(enemy + Offsets::Player_HeadCollider);
    if (AimLock && IsValidPointer(HedColider)) {
      Write(enemy + Offsets::Player_AimTargetWrite, HedColider);
    }

    Vector3 EnemyPos = GetTargetPosition(enemy);
    if (EnemyPos == Vector3::Zero()) continue;
    auto screen = WorldToScreenPoint(matrix, EnemyPos);
    float screenDist = sqrt((screen.X - sw / 2) * (screen.X - sw / 2) +
                            (screen.Y - sh / 2) * (screen.Y - sh / 2));
    float distanceToEnemy = Vector3::Distance(LocalPosition, EnemyPos);
    if (is3600 && !isKnocked && screen.X > 0 && screen.Y > 0)
      Draw360Alert(draw, screen, distanceToEnemy, sw, sh, isKnocked);
    if (screenDist < AimFov && screenDist < bestDist) {
      bestDist = screenDist;
      bestTarget = enemy;
      bestPos = EnemyPos;
    }
  }

  if (bestTarget && showAimLine) {
    if (IsValidPointer(bestTarget)) {
      auto bestAvatarMgr = Read<uintptr_t>(bestTarget + Offsets::Player_AvatarManager);
      if (IsValidPointer(bestAvatarMgr)) {
        auto bestUmaSimple = Read<uintptr_t>(bestAvatarMgr + Offsets::Avatar_UmaSimple);
        if (IsValidPointer(bestUmaSimple) && Read<bool>(bestUmaSimple + Offsets::Avatar_IsVisible2)) {
          Vector3 bestScreen = WorldToScreenPoint(matrix, bestPos);
          if (bestScreen.X > 0 && bestScreen.Y > 0) {
            ImVec2 center(sw / 2.0f, sh / 2.0f);
            ImVec2 target(bestScreen.X, bestScreen.Y);
            draw->AddLine(center, target, IM_COL32(255, 0, 0, 255), 2.5f);
          }
        }
      }
    }
  }

  if (bestTarget && AimSilent && track) {
    g_LocalPlayer.store(localPlayer);
    g_CurrentTargetObj.store(bestTarget);
    aimsilent::enabled = true;
    StartAimSilent();
  } else {
    aimsilent::enabled = false;
  }

  if (bestTarget && Aimbot) {
    auto CameraPosition = GetPosition(GetPlayerMainCamera(localPlayer));
    int isFiring = Read<int>(localPlayer + Offsets::Player_FiringState);
    if (!isFiring) {
      isFiring = Read<bool>(localPlayer + Offsets::sAim1) ? 1 : 0;
    }

    if (isFiring > 0 || AimLock) {
      Quaternion targetRot = GetRotationToTheLocation(bestPos, 0.0f, CameraPosition);
      Write<Quaternion>(localPlayer + Offsets::Player_ViewRotation, targetRot);
    }
  }

  if (bestTarget && AimVisible) {
    bool isFiring = Read<bool>(localPlayer + Offsets::sAim1);
    if (!isFiring) {
      isFiring = (Read<int>(localPlayer + Offsets::Player_FiringState) > 0);
    }

    if (isFiring) {
      Vector3 targetPos = Vector3::Zero();

      if (selectedBoneIndex == 0) {
        uintptr_t headNode = Read<uintptr_t>(bestTarget + Offsets::Bones_Head);
        if (IsValidPointer(headNode)) {
          targetPos = GetNodePosition(headNode);
        }
      } else if (selectedBoneIndex == 1) {
        uintptr_t headNode = Read<uintptr_t>(bestTarget + Offsets::Bones_Head);
        uintptr_t hipNode = Read<uintptr_t>(bestTarget + Offsets::Bones_Hip);
        if (IsValidPointer(headNode) && IsValidPointer(hipNode)) {
          Vector3 headW = GetNodePosition(headNode);
          Vector3 hipW = GetNodePosition(hipNode);
          if (headW != Vector3::Zero() && hipW != Vector3::Zero()) {
            targetPos.X = headW.X + (hipW.X - headW.X) * 0.35f;
            targetPos.Y = headW.Y + (hipW.Y - headW.Y) * 0.35f;
            targetPos.Z = headW.Z + (hipW.Z - headW.Z) * 0.35f;
          }
        }
      } else {
        uintptr_t ankleNode = Read<uintptr_t>(bestTarget + Offsets::Bones_Feet);
        if (IsValidPointer(ankleNode)) {
          targetPos = GetNodePosition(ankleNode);
        }
      }

      if (targetPos != Vector3::Zero()) {
        uintptr_t aimingInfo = Read<uintptr_t>(localPlayer + Offsets::Player_AimingInfo);
        if (!IsValidPointer(aimingInfo)) {
          aimingInfo = Read<uintptr_t>(localPlayer + Offsets::sAim2);
        }

        if (IsValidPointer(aimingInfo)) {
          Vector3 rayStart = Read<Vector3>(aimingInfo + Offsets::Player_AimingInfoStartPos);
          if (rayStart.X == 0 && rayStart.Y == 0 && rayStart.Z == 0) {
            rayStart = Read<Vector3>(aimingInfo + Offsets::sAim3);
          }

          if (rayStart != Vector3::Zero()) {
            Vector3 diff;
            diff.X = targetPos.X - rayStart.X;
            diff.Y = targetPos.Y - rayStart.Y;
            diff.Z = targetPos.Z - rayStart.Z;

            float dist = sqrtf(diff.X * diff.X + diff.Y * diff.Y + diff.Z * diff.Z);
            if (dist >= 0.5f && dist <= 500.0f) {
              Vector3 dir(diff.X / dist, diff.Y / dist, diff.Z / dist);
              Write<Vector3>(aimingInfo + Offsets::Player_AimingInfoDirection, dir);
              Write<Vector3>(aimingInfo + Offsets::sAim4, dir);
              Write<bool>(aimingInfo + 0x71, false);

              if (selectedBoneIndex == 0) {
                uintptr_t headCollider = Read<uintptr_t>(bestTarget + Offsets::Player_HeadCollider);
                if (IsValidPointer(headCollider)) {
                  Write<uintptr_t>(bestTarget + Offsets::Player_AimTargetWrite, headCollider);
                }
              }
            }
          }
        }
      }
    }
  }

  if (AimFov > 0 && showAimFov) {
    float fovRadius = GetFOVRadius(sh, AimFov);
    ImVec2 center(sw / 2.0f, sh / 2.0f);
    DrawFOVCircle(draw, center, fovRadius);
  }
}

void DrawESP(float sw, float sh, uintptr_t libAddress) {
  ImDrawList *draw = ImGui::GetBackgroundDrawList();
  if (!Enable) return;
  if (!IsValidPointer(libAddress)) return;

  auto GameFacade_c = Read<uintptr_t>(libAddress + Offsets::GameFacade);
  if (!IsValidPointer(GameFacade_c)) return;

  auto P2 = Read<uintptr_t>(GameFacade_c + Offsets::Engine_BaseGamePtr);
  if (!IsValidPointer(P2)) return;

  auto BaseGame = Read<uintptr_t>(P2);
  if (!IsValidPointer(BaseGame)) return;

  auto m_Match = Read<uintptr_t>(BaseGame + Offsets::Engine_Match);
  if (!IsValidPointer(m_Match)) return;

  int MatchIsRunning = Read<int>(m_Match + Offsets::MatchIsRunning);
  if (MatchIsRunning != 1) return;

  static uintptr_t s_stableLocalPlayer = 0;
  static uintptr_t s_lastBaseGame = 0;
  static int s_localPlayerMisses = 0;

  if (BaseGame != s_lastBaseGame) {
    s_lastBaseGame = BaseGame;
    s_stableLocalPlayer = 0;
    s_localPlayerMisses = 0;
  }

  auto rawLocalPlayer = Read<uintptr_t>(m_Match + Offsets::Match_LocalPlayer);
  if (!rawLocalPlayer) {
    auto CurrentObserve = Read<uintptr_t>(m_Match + Offsets::CurrentObserve);
    if (IsValidPointer(CurrentObserve)) {
      rawLocalPlayer = Read<uintptr_t>(CurrentObserve + Offsets::ObserverPlayer);
    }
  }

  if (IsValidPointer(rawLocalPlayer)) {
    s_stableLocalPlayer = rawLocalPlayer;
    s_localPlayerMisses = 0;
  } else if (IsValidPointer(s_stableLocalPlayer)) {
    auto oldAvatar = Read<uintptr_t>(s_stableLocalPlayer + Offsets::Player_AvatarManager);
    if (IsValidPointer(oldAvatar) && s_localPlayerMisses < 15) {
      s_localPlayerMisses++;
    } else {
      s_stableLocalPlayer = 0;
      s_localPlayerMisses = 0;
    }
  } else {
    s_stableLocalPlayer = 0;
    s_localPlayerMisses = 0;
  }

  uintptr_t localPlayer = s_stableLocalPlayer;
  if (!IsValidPointer(localPlayer)) return;

  auto dictionary = Read<uintptr_t>(BaseGame + Offsets::Engine_Dictionary);
  if (!IsValidPointer(dictionary)) return;

  auto entitylist = Read<uintptr_t>(dictionary + Offsets::Engine_EntityList);
  if (!IsValidPointer(entitylist)) return;

  uintptr_t activeCamOwner = localPlayer;
  D3DMatrix matrix;
  bool cameraFound = false;

  if (IsValidPointer(localPlayer)) {
    auto FollowCamera = Read<uintptr_t>(localPlayer + Offsets::Cam_FollowCamera);
    if (IsValidPointer(FollowCamera)) {
      auto Camera = Read<uintptr_t>(FollowCamera + Offsets::Cam_CameraObject);
      if (IsValidPointer(Camera)) {
        auto IntPtrCam = Read<uintptr_t>(Camera + Offsets::Cam_RawCameraData);
        if (IsValidPointer(IntPtrCam)) {
          matrix = Read<D3DMatrix>(IntPtrCam + Offsets::Cam_ViewMatrix);
          cameraFound = true;
        }
      }
    }
  }

  if (!cameraFound && IsValidPointer(entitylist)) {
    for (int i = 0; i < 300; i++) {
      uintptr_t entPtr = entitylist + i * 8;
      if (!IsValidPointer(entPtr)) break;

      auto ent = Read<uintptr_t>(entPtr);
      if (!IsValidPointer(ent)) continue;

      auto FollowCamera = Read<uintptr_t>(ent + Offsets::Cam_FollowCamera);
      if (IsValidPointer(FollowCamera)) {
        auto Camera = Read<uintptr_t>(FollowCamera + Offsets::Cam_CameraObject);
        if (IsValidPointer(Camera)) {
          auto IntPtrCam = Read<uintptr_t>(Camera + Offsets::Cam_RawCameraData);
          if (IsValidPointer(IntPtrCam)) {
            matrix = Read<D3DMatrix>(IntPtrCam + Offsets::Cam_ViewMatrix);
            activeCamOwner = ent;
            cameraFound = true;
            break;
          }
        }
      }
    }
  }

  if (!cameraFound) return;

  Vector3 LocalPosition = Vector3::Zero();
  if (IsValidPointer(activeCamOwner)) {
    LocalPosition = GetNodePosition(GetPlayerHeadTF(activeCamOwner));
    if (LocalPosition == Vector3::Zero()) {
      LocalPosition = GetPosition(GetPlayerMainCamera(activeCamOwner));
    }
  }

  int tempPlayerCount = 0;
  int tempBotCount = 0;
  uintptr_t processedEnemies[1000];
  int processedCount = 0;

  int touchAimCollectCount = 0;

  for (int i = 0; i < 1000; i++) {
    uintptr_t enemyPtr = entitylist + i * 8;
    if (!IsValidPointer(enemyPtr)) break;

    auto enemy = Read<uintptr_t>(enemyPtr);
    if (enemy == 0 || enemy == activeCamOwner) continue;

    bool alreadyProcessed = false;
    for (int p = 0; p < processedCount; p++) {
      if (processedEnemies[p] == enemy) {
        alreadyProcessed = true;
        break;
      }
    }
    if (alreadyProcessed) continue;
    if (processedCount < 1000) processedEnemies[processedCount++] = enemy;

    auto AvatarManager = Read<uintptr_t>(enemy + Offsets::Player_AvatarManager);
    if (!IsValidPointer(AvatarManager)) continue;

    auto UmaAvatarSimple = Read<uintptr_t>(AvatarManager + Offsets::Avatar_UmaSimple);
    if (!IsValidPointer(UmaAvatarSimple)) continue;

    if (!Read<bool>(UmaAvatarSimple + Offsets::Avatar_IsVisible2)) continue;

    auto UmaData = Read<uintptr_t>(UmaAvatarSimple + Offsets::Avatar_UmaData);
    if (!IsValidPointer(UmaData)) continue;

    if (Read<bool>(UmaData + Offsets::Avatar_IsDead)) continue;
    if (Read<bool>(enemy + 0x7C)) continue;

    bool isSpectating = Read<bool>(enemy + Offsets::Player_IsSpectating);
    if (isSpectating) continue;

    bool isKnocked = IsPlayerKnocked(enemy);

    auto Health = 0;
    uintptr_t hp_step1 = Read<uintptr_t>(enemy + Offsets::Player_HealthChain1);
    if (IsValidPointer(hp_step1)) {
      uintptr_t hp_step2 = Read<uintptr_t>(hp_step1 + Offsets::Player_HealthChain2);
      if (IsValidPointer(hp_step2)) {
        uintptr_t hp_step3 = Read<uintptr_t>(hp_step2 + Offsets::Player_HealthChain3);
        if (IsValidPointer(hp_step3)) {
          Health = Read<int>(hp_step3 + Offsets::Player_HealthValue);
        }
      }
    }

    if (Health <= 0) continue;

    auto isClientBot = Read<int>(enemy + Offsets::Player_ClientBotCheck);
    if (IgnoreBot && isClientBot == 1) continue;

    char currentName[64];
    memset(currentName, 0, 64);
    bool isBot = false;

    if (isClientBot == 1) {
      isBot = true;
      strcpy(currentName, "BOT");
      tempBotCount++;
    } else {
      uintptr_t NamePtr = Read<uintptr_t>(enemy + Offsets::Player_NamePtr);
      if (IsValidPointer(NamePtr)) getUTF8(currentName, NamePtr);
      if (currentName[0] == '\0') {
        strcpy(currentName, "BOT");
        isBot = true;
        tempBotCount++;
      } else {
        char checkName[16];
        strncpy(checkName, currentName, 15);
        checkName[15] = '\0';
        bool onlyNumbers = true;
        for (int ci = 0; ci < (int)strlen(checkName) && ci < 15; ci++) {
          if (!isdigit(checkName[ci]) && checkName[ci] != '\0') {
            onlyNumbers = false;
            break;
          }
        }
        if (onlyNumbers || strstr(checkName, "Player") != nullptr || strstr(checkName, "Bot") != nullptr || checkName[0] == '\0') {
          strcpy(currentName, "BOT");
          isBot = true;
          tempBotCount++;
        } else {
          tempPlayerCount++;
        }
      }
    }

    auto EnemyPosition = GetNodePosition(GetPlayerHeadTF(enemy));
    auto EnemyPositionPe = GetNodePosition(GetPlayerPeTF(enemy));

    if (EnemyPosition == Vector3::Zero()) continue;

    float distanceToEnemy = Vector3::Distance(LocalPosition, EnemyPosition);
    auto screenPos = WorldToScreenPoint(matrix, EnemyPosition);
    auto LocationHeadBox = WorldToScreenPoint(matrix, EnemyPosition);
    auto LocationToeBox = WorldToScreenPoint(matrix, EnemyPositionPe);

    if (LocationHeadBox.X <= 0 || LocationToeBox.X <= 0 || LocationHeadBox.Y <= 0 || LocationToeBox.Y <= 0) continue;
    if (LocationHeadBox.X > abs_ScreenX || LocationToeBox.X > abs_ScreenX || LocationHeadBox.Y > abs_ScreenY || LocationToeBox.Y > abs_ScreenY) continue;

    float headToFeetHeight = abs(LocationToeBox.Y - LocationHeadBox.Y);
    if (headToFeetHeight < 1.0f || headToFeetHeight > abs_ScreenY * 2) continue;

    float boxHeight = headToFeetHeight * 1.15f;
    float width = boxHeight * 0.48f;
    float boxX = LocationHeadBox.X - (width / 2.0f);
    float boxY = LocationHeadBox.Y - (boxHeight * 0.12f);

    if (TouchCfg.EnableAim && touchAimCollectCount < 100) {
        if (!(TouchCfg.IgnoreKnocked && isKnocked)) {
            uintptr_t targetBone = 0;
            if (TouchCfg.TargetBone == 0)      targetBone = Read<uintptr_t>(enemy + Offsets::Bones_Head);
            else if (TouchCfg.TargetBone == 1) targetBone = Read<uintptr_t>(enemy + Offsets::Bones_Chest);
            else                               targetBone = Read<uintptr_t>(enemy + Offsets::Bones_Hip);

            if (IsValidPointer(targetBone)) {
                Vector3 worldT = GetNodePosition(targetBone);
                if (worldT != Vector3::Zero()) {
                    Vector3 screenT = WorldToScreenPoint(matrix, worldT);
                    if (screenT.X > 0 && screenT.Y > 0 && screenT.X < abs_ScreenX && screenT.Y < abs_ScreenY) {
                        float scrDistT = sqrtf((screenT.X - abs_ScreenX / 2.0f) * (screenT.X - abs_ScreenX / 2.0f) +
                                               (screenT.Y - abs_ScreenY / 2.0f) * (screenT.Y - abs_ScreenY / 2.0f));
                        float wldDistT = Vector3::Distance(LocalPosition, worldT);

                        TouchAim[touchAimCollectCount].PawnAddr       = enemy;
                        TouchAim[touchAimCollectCount].ObjAim         = worldT;
                        TouchAim[touchAimCollectCount].Velocity       = Vector3(0, 0, 0);
                        TouchAim[touchAimCollectCount].ScreenDistance = scrDistT;
                        TouchAim[touchAimCollectCount].WodDistance    = wldDistT;
                        touchAimCollectCount++;
                    }
                }
            }
        }
    }

    if (showSkeleton) {
      DrawSkeletonESP(draw, enemy, matrix, isKnocked);
    }

    if (showHealth && Health > 0 && Health <= 200) {
      DrawAdvancedHealthBar(draw, boxX - healthBarWidth - 2.0f, boxY,
                            healthBarWidth, boxHeight, Health);
    }

    if (showBox) {
      DrawMainOutlineBox(draw, boxX, boxY, width, boxHeight, Health,
                         distanceToEnemy, isKnocked);
    }

    if (showLine) {
      ImVec2 lineStartPos = GetLineStartPosition(sw);
      DrawAdvancedLine(draw, lineStartPos, ImVec2(screenPos.X, screenPos.Y),
                       distanceToEnemy, Health, isKnocked);
    }

    if (showName) {
      ImU32 nc = isKnocked ? IM_COL32(255, 0, 0, 255) : IM_COL32(255, 255, 255, 255);
      draw->AddText(NULL, (float)textSize, ImVec2(boxX, boxY - (float)textSize - 5.0f),
                    nc, currentName);
    }

    if (showDistance) {
      char distText[32];
      sprintf(distText, "%.0fm", distanceToEnemy);
      ImU32 distCol = isKnocked ? IM_COL32(255, 0, 0, 255) : IM_COL32(255, 255, 255, 255);
      draw->AddText(NULL, (float)textSize, ImVec2(boxX, boxY + boxHeight + 4.0f),
                    distCol, distText);
    }

    if (is3600 && !isKnocked && screenPos.X > 0 && screenPos.Y > 0) {
      Draw360Alert(draw, screenPos, distanceToEnemy, sw, sh, isKnocked);
    }
  }

  {
      std::lock_guard<std::mutex> lock(g_TouchAimMtx);
      TouchAimCount = touchAimCollectCount;
  }

  globalPlayerCount = tempPlayerCount;
  globalBotCount = tempBotCount;
}

#endif