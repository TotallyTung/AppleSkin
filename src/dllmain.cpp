#include "dllmain.hpp"
#include <mc/src-client/common/client/game/ClientInstance.hpp>
#include <mc/src/common/world/actor/player/Player.hpp>
#include <mc/src-client/common/client/player/LocalPlayer.hpp>
#include <mc/src/common/world/item/Item.hpp>
#include <mc/src/common/world/item/ItemStack.hpp>
#include "impl/AttributesComponent.hpp"
#include "impl/FoodItemComponent.hpp"

std::string buildHungerString(float v) {
    std::string out;
    out.reserve(3 * ceil(v));
    for (int i = 0; i < (int)v; i++) out += "";
    if (floor(v) != v)
        out += "";
    return out;
}

SafetyHookInline _Item_appendFormattedHovertext;

void beforeRenderUI(BeforeRenderUIEvent& ev) {
    ClientInstance* client = ev.ctx.mClient;
    LocalPlayer* lp = client->getLocalPlayer();
    if (lp == nullptr) return;
    auto& attributes = lp->tryGetComponent<AttributesComponent>()->mAttributes;
    auto& hunger = attributes.getAttribute(2);
    Log::Info("{}", hunger.mCurrentValue);
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
    auto& hooks = Amethyst::GetHookManager();

    VHOOK(Item, appendFormattedHovertext, this);
    bus.AddListener<BeforeRenderUIEvent>(&beforeRenderUI);
}
