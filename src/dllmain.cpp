#include "dllmain.hpp"
#include <mc/src-client/common/client/game/ClientInstance.hpp>
#include <mc/src/common/world/actor/player/Player.hpp>
#include <mc/src-client/common/client/player/LocalPlayer.hpp>
#include <mc/src/common/world/item/Item.hpp>
#include <mc/src/common/world/item/ItemStack.hpp>
#include <mc/src-client/common/client/gui/controls/renderers/HoverRenderer.hpp>
#include <mc/src/common/world/item/ItemStack.hpp>
#include <mc/src-client/common/client/gui/ScreenView.hpp>
#include <mc/src-deps/core/string/StringHash.hpp>
#include <mc/src-deps/core/resource/ResourceHelper.hpp>
#include <mc/src-client/common/client/renderer/TexturePtr.hpp>
#include <mc/src-client/common/client/renderer/BaseActorRenderContext.hpp>
#include <mc/src-client/common/client/renderer/actor/ItemRenderer.hpp>
#include <mc/src-client/common/client/renderer/NinesliceInfo.hpp>
#include <amethyst/ui/NinesliceHelper.hpp>
#include "AttributesComponent.hpp"
#include "FoodItemComponent.hpp"

std::string repeatStr(const std::string& c, int n) {
    std::string out;
    out.reserve(c.size() * n);
    for (int i = 0; i < n; i++) {
        out += c;
    }
    return out;
}

FoodInfo foodinfo;
static HashedString flushString(0xA99285D21E94FC80, "ui_flush");
bool hasLoadedTexture = false;
float iconSize = 10.f;
glm::tvec2<float> textureSize(iconSize, iconSize);
glm::tvec2<float> uvPos(188.0f / 256.0f, 184.0f / 256.0f);
glm::tvec2<float> uvSize(22.0f / 256.0f, 22.0f / 256.0f);
mce::TexturePtr foodTexture;
mce::TexturePtr mBackgroundTexture;
Amethyst::NinesliceHelper backgroundNineslice(16, 16, 4, 4);
SafetyHookInline _HoverRenderer__renderHoverBox;
SafetyHookInline _Item_appendFormattedHovertext;

void beforeRenderUI(BeforeRenderUIEvent& ev) {
    ClientInstance* client = ev.ctx.mClient;
    LocalPlayer* lp = client->getLocalPlayer();
    if (lp == nullptr) return;
    auto comp = lp->tryGetComponent<AttributesComponent>();
    auto& hunger = comp->getAttribute(2);
    Log::Info("{}", hunger.mCurrentValue);
}

void Item_appendFormattedHovertext(const Item* self, const ItemStackBase& stack, Level& level, std::string& hovertext, bool showCategory) {
    _Item_appendFormattedHovertext.thiscall<void, const Item*, const ItemStackBase&, Level&, std::string&, bool>(self, stack, level, hovertext, showCategory);

    Item* item = stack.mItem;
    if (item->isFood()) {
        foodinfo.itemName = item->buildDescriptionName(stack);
        foodinfo.foodValue = (int)(item->getFood()->mSaturationModifier * 5);
        hovertext += "\n" + repeatStr("", ceil(item->getFood()->mSaturationModifier * 5));
    }
}

void HoverRenderer__renderHoverBox(HoverRenderer* self, MinecraftUIRenderContext* ctx, IClientInstance* client, RectangleArea* aabb, float someFloat) {
    if (!hasLoadedTexture) {
        foodTexture = ctx->getTexture("textures/ui/hunger_full", true);
        mBackgroundTexture = ctx->getTexture("textures/ui/purpleBorder", true);
        hasLoadedTexture = true;
    }
    if (self->mFilteredContent.find(foodinfo.itemName) != std::string::npos) {
        float panelX = self->mCursorPosition.x + self->mOffset.x;
        float panelY = self->mCursorPosition.y + self->mOffset.y;
        RectangleArea background = { panelX - 4, panelX + iconSize * foodinfo.foodValue + 4, panelY - 4, panelY + iconSize + 4 };
        
        backgroundNineslice.Draw(background, &mBackgroundTexture, ctx);
        ctx->flushImages(mce::Color::WHITE, 1.f, flushString);
        for (int i = 0; i < foodinfo.foodValue; i++) {
            glm::tvec2<float> texturePos(panelX + iconSize * i, panelY);
            ctx->drawImage(foodTexture, &texturePos, &textureSize, &uvPos, &uvSize, 0);
        }
        ctx->flushImages(mce::Color::WHITE, 1.f, flushString);
    }
    _HoverRenderer__renderHoverBox.thiscall(self, ctx, client, aabb, someFloat);
};

// Ran when the mod is loaded into the game by AmethystRuntime
ModFunction void Initialize(AmethystContext& ctx, const Amethyst::Mod& mod) 
{
    // Initialize Amethyst mod backend
    Amethyst::InitializeAmethystMod(ctx, mod);
    auto& bus = Amethyst::GetEventBus();
    auto& hooks = Amethyst::GetHookManager();

    VHOOK(Item, appendFormattedHovertext, this);
    //HOOK(HoverRenderer, _renderHoverBox);
    bus.AddListener<BeforeRenderUIEvent>(&beforeRenderUI);
}
