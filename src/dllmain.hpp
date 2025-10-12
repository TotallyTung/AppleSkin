#pragma once
#include <Windows.h>
#include <amethyst/runtime/mod/Mod.hpp>
#include <amethyst/runtime/ModContext.hpp>
#include <amethyst/runtime/HookManager.hpp>
#include <amethyst/runtime/events/RenderingEvents.hpp>

#define ModFunction extern "C" __declspec(dllexport)

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}

void beforeRenderUI(BeforeRenderUIEvent& ev);

struct FoodInfo {
    std::string itemName;
    int foodValue;
};