#pragma once

#include <cstdint>

bool Touch_Init(int w, int h, uint32_t orientation_, bool readOnly);
void UpdateScreenData(int w, int h, uint32_t orientation_);

void Touch_Close();
void Touch_Down(float x, float y);
void Touch_Move(float x, float y);
void Touch_Up();

bool IsFingerDown(int slot);
bool GetFingerScreenPos(int slot, float& outX, float& outY);
void Touch_Down_Slot(int slot, float xt, float yt);
void Touch_Move_Slot(int slot, float x, float y);
void Touch_Up_Slot(int slot);
bool Touch_IsInitialized();
int Touch_GetFdNum();
float Touch_GetScaleX();
float Touch_GetScaleY();
uint32_t Touch_GetOrientation();
float Touch_GetScreenWidth();
float Touch_GetScreenHeight();