#ifndef INPUT_HOOK_H
#define INPUT_HOOK_H

#include <android/input.h>
#include <android/keycodes.h>
#include <android/looper.h>
#include <jni.h>
#include <dlfcn.h>
#include <iostream>
#include "imgui.h"

// Note: ImGui initialized hai isko pehle se hi ensure karein touch coordinates capture se pehle
extern IMGUI_IMPL_API LRESULT ImGui_ImplAndroid_HandleInputEvent(AInputEvent* event, float window_width, float window_height);

namespace AndroidTouchHook {

    // Global states
    bool g_TouchInitialized = false;
    float g_ScreenWidth = 1920.0f;
    float g_ScreenHeight = 1080.0f;

    // ==================== SOLUTION 1: NATIVE INPUT QUEUE HOOKING (No Sandbox block) ====================
    // Android 16 safety mechanism: events ko direct in-process loop me catch karta hai
    
    typedef int32_t (*t_AInputQueue_getEvent)(AInputQueue* queue, AInputEvent** outEvent);
    typedef int32_t (*t_AInputQueue_preDispatchEvent)(AInputQueue* queue, AInputEvent* event);
    typedef void (*t_AInputQueue_finishEvent)(AInputQueue* queue, AInputEvent* event, int handled);

    t_AInputQueue_getEvent orig_AInputQueue_getEvent = nullptr;
    t_AInputQueue_preDispatchEvent orig_AInputQueue_preDispatchEvent = nullptr;
    t_AInputQueue_finishEvent orig_AInputQueue_finishEvent = nullptr;

    // Hook logic jo custom touches ko direct ImGui IO buffer me register karwayega
    int32_t hooked_AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent) {
        int32_t result = orig_AInputQueue_getEvent(queue, outEvent);
        if (result >= 0 && outEvent && *outEvent) {
            AInputEvent* event = *outEvent;
            int32_t type = AInputEvent_getType(event);
            
            if (type == AINPUT_EVENT_TYPE_MOTION) {
                ImGuiIO& io = ImGui::GetIO();
                int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
                float x = AMotionEvent_getX(event, 0);
                float y = AMotionEvent_getY(event, 0);

                // Mapping coordinated to ImGui
                io.AddMousePosEvent(x, y);

                if (action == AMOTION_EVENT_ACTION_DOWN) {
                    io.AddMouseButtonEvent(0, true);
                } else if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_CANCEL) {
                    io.AddMouseButtonEvent(0, false);
                }
                
                // Blocks game execution background input if menu is actively shown
                bool menuFocused = true; // Overlay visibility boolean toggle ke mutabik badlein
                if (menuFocused) {
                    orig_AInputQueue_finishEvent(queue, event, 1);
                    return -1; // Concluded & Consume touch frame
                }
            }
        }
        return result;
    }

    // JNI_OnLoad ya initial library runtime me isko execute karein
    bool InstallNativeInputHook() {
        void* libAndroid = dlopen("libandroid.so", RTLD_NOLOAD);
        if (!libAndroid) {
            libAndroid = dlopen("libandroid.so", RTLD_NOW);
        }

        if (libAndroid) {
            orig_AInputQueue_getEvent = (t_AInputQueue_getEvent)dlsym(libAndroid, "AInputQueue_getEvent");
            orig_AInputQueue_preDispatchEvent = (t_AInputQueue_preDispatchEvent)dlsym(libAndroid, "AInputQueue_preDispatchEvent");
            orig_AInputQueue_finishEvent = (t_AInputQueue_finishEvent)dlsym(libAndroid, "AInputQueue_finishEvent");

            if (orig_AInputQueue_getEvent && orig_AInputQueue_finishEvent) {
                // Inline Hook setup like MSHookFunction ya DobbyHook inject karein
                // Example: DobbyHook((void*)orig_AInputQueue_getEvent, (void*)hooked_AInputQueue_getEvent, (void**)&orig_AInputQueue_getEvent);
                g_TouchInitialized = true;
                return true;
            }
        }
        return false;
    }

    // ==================== SOLUTION 2: SELINUX & DAEMON PERMISSION PATCH ====================
    // Root Daemon overlays run karne ke liye in operations ko bypass background me deploy karein
    void ApplySelinuxRules() {
        // Enforcing system permission overrides to unlock device event accessibility
        system("su -c setenforce 0"); 
        system("su -c chmod 666 /dev/input/event*");
        
        // Magisk policy inject updates for Android 16 to bypass sandbox boundaries
        system("su -c magiskpolicy --live \"allow system_server system_server_tmpfs file { read write open }\"");
        system("su -c magiskpolicy --live \"allow untrusted_app input_device chr_file { read write open ioctl }\"");
        system("su -c magiskpolicy --live \"allow priv_app input_device chr_file { read write open ioctl }\"");
    }

    // Dynamic resolution coordinates setter to fix offset/shift click glitches
    void SetScreenResolution(float width, float height) {
        g_ScreenWidth = width;
        g_ScreenHeight = height;
    }
}

#endif // INPUT_HOOK_H