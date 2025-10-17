#include "dllmain.hpp"
#include <mc/src-client/common/client/game/ClientInstance.hpp>
#include <mc/src/common/world/actor/player/Player.hpp>
#include <mc/src-client/common/client/player/LocalPlayer.hpp>
#include <mc/src/common/world/item/Item.hpp>
#include <mc/src/common/world/item/ItemStack.hpp>
#include <mc/src/common/world/level/Level.hpp>
#include <mc/src/common/world/level/storage/GameRules.hpp>
#include <mc/src-client/common/client/renderer/screen/MinecraftUIRenderContext.hpp>
#include <mc/src-client/common/client/gui/gui/UIControl.hpp>
#include <mc/src-deps/core/string/StringHash.hpp>
#include "impl/AttributesComponent.hpp"
#include "impl/FoodItemComponent.hpp"

bool hasLoadedTextures = false;
bool doNaturalRegeneration = true;
static HashedString flushString(0xA99285D21E94FC80, "ui_flush");
FadeAnimation fadeanimation;
glm::tvec2<float> iconSize(9.f, 9.f);
glm::tvec2<float> uvPos(0.f, 0.f);
glm::tvec2<float> uvSize(1.f, 1.f);
float heartPos;
SafetyHookInline _Item_appendFormattedHovertext;
SafetyHookInline _HudHungerRenderer_render;
SafetyHookInline _HudHeartRenderer_render;

std::string buildHungerString(float v) {
    std::string out;
    out.reserve(3 * ceil(v));
    for (int i = 0; i < (int)v; i++) out += "";
    if (floor(v) != v)
        out += "";
    return out;
}

float calculateRegeneratedHealth(float hunger, float exhaustion) {
    if (!doNaturalRegeneration || hunger < 18.f) return 0.f;
    float healed = 0.f;
    while (hunger >= 18.f) {
        healed += 1;
        exhaustion += 6;
        if (exhaustion >= 4) {
            hunger -= 1;
            exhaustion -= 1;
        }
    }
    return healed;
}

void drawIconBar(MinecraftUIRenderContext& ctx, mce::TexturePtr& iconTexture, mce::TexturePtr& halfIconTexture, float val, float x, float y, BarAlign align, float alpha = 255.f, mce::Color flushColor = mce::Color::WHITE) {
    int barLength = ceil(val);
    for (int i = 0; i < barLength; i++) {
        glm::tvec2<float> iconPos(x + (int)align * (iconSize.x - 1) * (i + 1), y);
        ctx.drawImage((val - (int)val >= 0.5 && i == barLength - 1) ? halfIconTexture : iconTexture, &iconPos, &iconSize, &uvPos, &uvSize, 0);
        ctx.flushImages(flushColor, alpha, flushString);
    }
}

void onStartJoinGame(OnStartJoinGameEvent& ev) {
    ILevel* level = ev.client.getLocalPlayer()->getLevel();
    doNaturalRegeneration = level->getGameRules().mGameRules[(int)GameRulesIndex::DoNaturalRegeneration].mEnabled;
}

void HudHungerRenderer_render(MinecraftUICustomRenderer& self, MinecraftUIRenderContext& renderContext, ClientInstance& client, UIControl& owner, int pass, RectangleArea& aabb) {
    _HudHungerRenderer_render.call<void, MinecraftUICustomRenderer&, MinecraftUIRenderContext&, ClientInstance&, UIControl&, int, RectangleArea&>(self, renderContext, client, owner, pass, aabb);
    if (!hasLoadedTextures) {
        modTextures::hungerFull = renderContext.getTexture("textures/ui/hunger_full", true);
        modTextures::hungerHalf = renderContext.getTexture("textures/ui/hunger_half", true);
        modTextures::saturationFull = renderContext.getTexture("textures/ui/saturation_full", true);
        modTextures::saturationHalf = renderContext.getTexture("textures/ui/saturation_half", true);
        modTextures::heartFull = renderContext.getTexture("textures/ui/heart_new", true);
        modTextures::heartHalf = renderContext.getTexture("textures/ui/heart_half", true);
        hasLoadedTextures = true;
    }
    LocalPlayer* lp = client.getLocalPlayer();
    BaseAttributeMap& attributes = lp->tryGetComponent<AttributesComponent>()->mAttributes;
    float hunger = attributes.getInstance(2).mCurrentValue;
    float saturation = attributes.getInstance(3).mCurrentValue;
    float exhaustion = attributes.getInstance(4).mCurrentValue;
    float health = attributes.getInstance(7).mCurrentValue;
    drawIconBar(renderContext, modTextures::saturationFull, modTextures::saturationHalf, saturation / 2, aabb._x1, aabb._y0, BarAlign::RIGHT);
    const Item* item = lp->playerInventory->getSelectedItem().getItem();
    if (item != nullptr && item->isFood()) {
        auto food = item->getFood();
        float foodVal = food->mSaturationModifier * 5;
        drawIconBar(
            renderContext,
            modTextures::hungerFull,
            modTextures::hungerHalf,
            std::min(foodVal + hunger / 2, 10.f),
            aabb._x1,
            aabb._y0,
            BarAlign::RIGHT,
            fadeanimation.alpha
        );
        drawIconBar(
            renderContext,
            modTextures::saturationFull,
            modTextures::saturationHalf,
            std::min(foodVal + saturation / 2, 10.f),
            aabb._x0,
            aabb._y0,
            BarAlign::RIGHT,
            fadeanimation.alpha
        );
        float healed = calculateRegeneratedHealth(hunger, exhaustion);
        if(healed > 0.f)
            drawIconBar(
                renderContext,
                modTextures::heartFull,
                modTextures::heartHalf,
                std::min((healed + health) / 2, 10.f),
                heartPos,
                aabb._y0,
                BarAlign::LEFT,
                fadeanimation.alpha
            );
    }
    fadeanimation.update();
}

void HudHeartRenderer_render(MinecraftUICustomRenderer& self, MinecraftUIRenderContext& renderContext, ClientInstance& client, UIControl& owner, int pass, RectangleArea& aabb) {
    _HudHeartRenderer_render.call<void, MinecraftUICustomRenderer&, MinecraftUIRenderContext&, ClientInstance&, UIControl&, int, RectangleArea&>(self, renderContext, client, owner, pass, aabb);
    heartPos = aabb._x1;
}

void Item_appendFormattedHovertext(const Item* self, const ItemStackBase& stack, Level& level, std::string& hovertext, bool showCategory) {
    _Item_appendFormattedHovertext.thiscall<void, const Item*, const ItemStackBase&, Level&, std::string&, bool>(self, stack, level, hovertext, showCategory);

    Item* item = stack.mItem;
    if (item->isFood())
        hovertext += "\n" + buildHungerString(item->getFood()->mSaturationModifier * 5);
}

// Ran when the mod is loaded into the game by AmethystRuntime
ModFunction void Initialize(AmethystContext& ctx, const Amethyst::Mod& mod) 
{
    // Initialize Amethyst mod backend
    Amethyst::InitializeAmethystMod(ctx, mod);
    auto& bus = Amethyst::GetEventBus();
    //bus.AddListener<OnStartJoinGameEvent>(&onStartJoinGame);

    auto& hooks = Amethyst::GetHookManager();
    hooks.CreateHookAbsolute(
        _HudHungerRenderer_render,
        SigScan("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 0F 29 74 24 ? 49 8B F1"),
        (void*)HudHungerRenderer_render
    );
    hooks.CreateHookAbsolute(
        _HudHeartRenderer_render,
        SigScan("48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 ? 0F 29 78 ? 44 0F 29 40 ? 44 0F 29 88 ? ? ? ? 44 0F 29 90 ? ? ? ? 44 0F 29 98 ? ? ? ? 44 0F 29 A0 ? ? ? ? 44 0F 29 A8 ? ? ? ? 48 8B 05"),
        (void*)HudHeartRenderer_render
    );
    VHOOK(Item, appendFormattedHovertext, this);
}
