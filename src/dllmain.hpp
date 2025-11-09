#pragma once
#include <Windows.h>
#include <amethyst/runtime/mod/Mod.hpp>
#include <amethyst/runtime/ModContext.hpp>
#include <amethyst/runtime/HookManager.hpp>
#include <amethyst/runtime/events/GameEvents.hpp>
#include <amethyst/runtime/events/RenderingEvents.hpp>
#include <mc/src-client/common/client/renderer/TexturePtr.hpp>

#define ModFunction extern "C" __declspec(dllexport)

class MinecraftUICustomRenderer;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}

void onLevelConstructed(OnLevelConstructedEvent&);
void afterRenderUI(AfterRenderUIEvent&);
void beforeRenderUI(BeforeRenderUIEvent&);

namespace modTextures {
    mce::TexturePtr hungerFull;
    mce::TexturePtr hungerHalf;
    mce::TexturePtr saturationFull;
    mce::TexturePtr saturationHalf;
    mce::TexturePtr heartFull;
    mce::TexturePtr heartHalf;
    mce::TexturePtr exhaustionFull;
    mce::TexturePtr exhaustionHalf;
};

enum BarAlign {
    LEFT = 1,
    RIGHT = -1
};

struct FadeAnimation {
    float alpha;
    float speed;
    float state;
    FadeAnimation() {
        alpha = 0.f;
        speed = 0.01f;
        state = 1;
    }
    void update() {
        if (alpha == 1)
            state = -1;
        else if (alpha == 0)
            state = 1;
        alpha = std::clamp(alpha + speed * state, 0.f, 1.f);
    }
};