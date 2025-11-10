#include "dllmain.hpp"
#include <format>
#include <mc/src-client/common/client/game/ClientInstance.hpp>
#include <mc/src/common/world/actor/player/Player.hpp>
#include <mc/src-client/common/client/player/LocalPlayer.hpp>
#include <mc/src/common/world/item/Item.hpp>
#include <mc/src/common/world/item/FoodItemComponentLegacy.hpp>
#include <mc/src/common/world/item/ItemStack.hpp>
#include <mc/src/common/world/level/Level.hpp>
#include <mc/src/common/world/level/storage/GameRules.hpp>
#include <mc/src-client/common/client/renderer/screen/MinecraftUIRenderContext.hpp>
#include <mc/src-client/common/client/gui/gui/UIControl.hpp>
#include <mc/src-deps/core/string/StringHash.hpp>
#include <mc/src-client/common/client/gui/ScreenView.hpp>
#include <mc/src-client/common/client/gui/gui/VisualTree.hpp>
#include "impl/AttributesComponent.hpp"

bool hasLoadedTextures = false;
bool doNaturalRegeneration = true;
static HashedString flushString(0xA99285D21E94FC80, "ui_flush");
FadeAnimation fadeanimation;
glm::tvec2<float> hudHungerPos;
glm::tvec2<float> hudHeartPos;
glm::tvec2<float> iconSize(9.f, 9.f);
glm::tvec2<float> uvPos(0.f, 0.f);
glm::tvec2<float> uvSize(1.f, 1.f);
SafetyHookInline _Item_appendFormattedHovertext;
SafetyHookInline _HudHungerRenderer_render;
SafetyHookInline _HudHeartRenderer_render;

std::string buildBarString(int v, const std::string full, const std::string half) {
    std::string out;
    for (int i = 0; i < (int)v; i+=2) out += full; // 
    if (fmod(v, 2) != 0)
        out += half; // 
    return out;
}

float calculateRegeneratedHealth(float hunger, float saturation, float exhaustion, const std::vector<FoodItemComponentLegacy::Effect>& effects) {
    // To do: calc regen heart from effects
    float healed = 0.f;
    if (doNaturalRegeneration || hunger >= 18.0f) healed = floorf((saturation + (hunger - 18.0f) + (exhaustion / 4.0f)) / 1.5f);
    for (auto& effect : effects) {
        if (effect.name == "regeneration") {
            healed += floorf((float)effect.duration.mValue / std::max(50 >> effect.amplifier, 1));
            break;
        }
    }
    return healed;
}

void drawIconBar(MinecraftUIRenderContext& ctx, mce::TexturePtr& iconTexture, mce::TexturePtr& halfIconTexture, float val, glm::tvec2<float> pos, BarAlign align, float alpha = 255.f, mce::Color flushColor = mce::Color::WHITE) {
    int barLength = ceil(val);
    for (int i = 0; i < barLength; i++) {
        glm::tvec2<float> iconPos(pos.x + (int)align * (iconSize.x - 1) * (i + (align == BarAlign::LEFT ? 0 : 1)), pos.y);
        ctx.drawImage((val - (int)val >= 0.5 && i == barLength - 1) ? halfIconTexture : iconTexture, iconPos, iconSize, uvPos, uvSize, 0);
        ctx.flushImages(flushColor, alpha, flushString);
    }
}

void onLevelConstructed(OnLevelConstructedEvent& ev) {
    doNaturalRegeneration = ev.mLevel.getGameRules().mGameRules[(int)GameRulesIndex::DoNaturalRegeneration].mEnabled;
}

void HudHungerRenderer_render(MinecraftUICustomRenderer& self, MinecraftUIRenderContext& renderContext, ClientInstance& client, UIControl& owner, int pass, RectangleArea& aabb) {
    _HudHungerRenderer_render.call<void, MinecraftUICustomRenderer&, MinecraftUIRenderContext&, ClientInstance&, UIControl&, int, RectangleArea&>(self, renderContext, client, owner, pass, aabb);
    hudHungerPos = owner.mCachedPosition;
}

void HudHeartRenderer_render(MinecraftUICustomRenderer& self, MinecraftUIRenderContext& renderContext, ClientInstance& client, UIControl& owner, int pass, RectangleArea& aabb) {
    _HudHeartRenderer_render.call<void, MinecraftUICustomRenderer&, MinecraftUIRenderContext&, ClientInstance&, UIControl&, int, RectangleArea&>(self, renderContext, client, owner, pass, aabb);
    hudHeartPos = owner.mCachedPosition;
}

void beforeRenderUI(BeforeRenderUIEvent& ev) {
    MinecraftUIRenderContext& renderContext = ev.ctx;
    LocalPlayer* lp = renderContext.mClient->getLocalPlayer();
    if (lp == nullptr || lp->isCreative() || !hasLoadedTextures || ev.screen.visualTree->mRootControlName->mName != "hud_screen") return;
    BaseAttributeMap& attributes = lp->tryGetComponent<AttributesComponent>()->mAttributes;
    float exhaustion = attributes.getInstance(4).mCurrentValue;
    drawIconBar(renderContext, modTextures::exhaustionFull, modTextures::exhaustionHalf, exhaustion / 2, hudHungerPos, BarAlign::RIGHT);
}

void afterRenderUI(AfterRenderUIEvent& ev) {
    
    MinecraftUIRenderContext& renderContext = ev.ctx;
    LocalPlayer* lp = renderContext.mClient->getLocalPlayer();
    if (lp == nullptr || lp->isCreative() || ev.screen.visualTree->mRootControlName->mName != "hud_screen") return;
    
    if (!hasLoadedTextures) {
        modTextures::hungerFull = renderContext.getTexture("textures/ui/hunger_full", true);
        modTextures::hungerHalf = renderContext.getTexture("textures/ui/hunger_half", true);
        modTextures::saturationFull = renderContext.getTexture("textures/ui/saturation_full", true);
        modTextures::saturationHalf = renderContext.getTexture("textures/ui/saturation_half", true);
        modTextures::heartFull = renderContext.getTexture("textures/ui/heart", true);
        modTextures::heartHalf = renderContext.getTexture("textures/ui/heart_half", true);
        modTextures::exhaustionFull = renderContext.getTexture("textures/ui/exhaustion_full", true);
        modTextures::exhaustionHalf = renderContext.getTexture("textures/ui/exhaustion_half", true);
        hasLoadedTextures = true;
    }
    BaseAttributeMap& attributes = lp->tryGetComponent<AttributesComponent>()->mAttributes;
    float hunger = attributes.getInstance(2).mCurrentValue;
    float saturation = attributes.getInstance(3).mCurrentValue;
    float exhaustion = attributes.getInstance(4).mCurrentValue;
    float health = attributes.getInstance(7).mCurrentValue;
    drawIconBar(renderContext, modTextures::saturationFull, modTextures::saturationHalf, saturation / 2, hudHungerPos, BarAlign::RIGHT);
    const Item* item = lp->playerInventory->getSelectedItem().getItem();
    if (item != nullptr && item->isFood()) {
        IFoodItemComponent* food = item->getFood();
        FoodItemComponentLegacy* legacyFoodComponent = item->mFoodComponentLegacy.get();
        int nutrition = food->getNutrition();
        float predictedHunger = std::min(hunger + nutrition, 20.f);
        float predictedSaturation = std::min(saturation + food->getSaturationModifier() * nutrition * 2, predictedHunger);
        drawIconBar(
            renderContext,
            modTextures::hungerFull,
            modTextures::hungerHalf,
            predictedHunger / 2,
            hudHungerPos,
            BarAlign::RIGHT,
            fadeanimation.alpha
        );
        drawIconBar(
            renderContext,
            modTextures::saturationFull,
            modTextures::saturationHalf,
            predictedSaturation / 2,
            hudHungerPos,
            BarAlign::RIGHT,
            fadeanimation.alpha
        );

        float healed = calculateRegeneratedHealth(
            predictedHunger,
            predictedSaturation,
            exhaustion,
            legacyFoodComponent != nullptr ? legacyFoodComponent->mEffects : std::vector<FoodItemComponentLegacy::Effect>()
        );
        if (healed > 0.f) {
            drawIconBar(
                renderContext,
                modTextures::heartFull,
                modTextures::heartHalf,
                std::min((healed + health) / 2, 10.f),
                hudHeartPos,
                BarAlign::LEFT,
                fadeanimation.alpha
            );
        }
    }
    fadeanimation.update();
}

void Item_appendFormattedHovertext(const Item* self, const ItemStackBase& stack, Level& level, std::string& hovertext, bool showCategory) {
    _Item_appendFormattedHovertext.thiscall<void, const Item*, const ItemStackBase&, Level&, std::string&, bool>(self, stack, level, hovertext, showCategory);

    Item* item = stack.mItem;
    IFoodItemComponent* food = item->getFood();
    if (item->isFood() && food != nullptr) {
        int nutrition = food->getNutrition();
        int saturation = food->getSaturationModifier() * nutrition * 2;
        if (nutrition > 0)
            hovertext += std::format("\n{} ({})", buildBarString(nutrition, "", ""), nutrition);

        if(saturation > 0)
            hovertext += std::format("\n{} ({})", buildBarString(saturation, "", ""), saturation);
    }
}

ModFunction void Initialize(AmethystContext& ctx, const Amethyst::Mod& mod) {
    Amethyst::InitializeAmethystMod(ctx, mod);
    auto& bus = Amethyst::GetEventBus();
    bus.AddListener<AfterRenderUIEvent>(&afterRenderUI);
    bus.AddListener<BeforeRenderUIEvent>(&beforeRenderUI);
    bus.AddListener<OnLevelConstructedEvent>(&onLevelConstructed);

    auto& hooks = Amethyst::GetHookManager();
    hooks.CreateHookAbsolute(
        _HudHungerRenderer_render,
        SigScan("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 0F 29 74 24 ? 49 8B F1"),
        (void*)HudHungerRenderer_render
    );
    hooks.CreateHookAbsolute(
        _HudHeartRenderer_render,
        SigScan("40 55 53 56 57 41 54 41 56 41 57 48 8B EC 48 81 EC ? ? ? ? 0F 29 74 24 ? 49 8B F9"),
        (void*)HudHeartRenderer_render
    );
    VHOOK(Item, appendFormattedHovertext, this);
}
