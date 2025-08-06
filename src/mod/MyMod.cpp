#include "mod/MyMod.h"

#include <ll/api/io/Logger.h>
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerJoinEvent.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/player/SkinInfoData.h"
#include "mc/world/actor/player/SerializedSkin.h"

namespace SkinControl {

std::unordered_map<std::string, SerializedSkin> originalSkins;
std::unordered_map<std::string, bool> skinVisibilityStates;

SerializedSkin createSteveSkin() {
    SerializedSkin skin;
    skin.setSkinId("steve");
    return skin;
}

SerializedSkin createAlexSkin() {
    SerializedSkin skin;
    skin.setSkinId("alex");
    return skin;
}

void setDefaultSkin(Player& player) {
    bool isSlimModel = false;
    auto& skinInfo = player.getSkinInfoData();
    if (skinInfo) {
        skinInfo->updateSkin(
            isSlimModel ? createAlexSkin() : createSteveSkin(),
            nullptr,
            nullptr
        );
    }
}

void restoreOriginalSkin(Player& player) {
    auto it = originalSkins.find(player.getRealName());
    if (it != originalSkins.end()) {
        if (auto* skinInfo = player.getSkinInfoData()) {
            skinInfo->updateSkin(it->second, nullptr, nullptr);
        }
        originalSkins.erase(it);
    }
}

void toggleSkinVisibility(Player& player, bool visible) {
    std::string name = player.getRealName();
    
    if (!visible) {
        if (originalSkins.find(name) == originalSkins.end()) {
            originalSkins[name] = player.getSkin();
        }
        setDefaultSkin(player);
    } else {
        restoreOriginalSkin(player);
    }
    
    skinVisibilityStates[name] = visible;
    player.sendMessage(visible ? "§a已恢复皮肤显示" : "§a已隐藏其他玩家皮肤");
}

void registerCommands() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "skin",
        "控制皮肤可见性",
        CommandPermissionLevel::Any
    );
    
    cmd.overload().text("mode").execute([](CommandOrigin const& origin, CommandOutput& output, std::string const& mode) {
        if (auto player = origin.getPlayer()) {
            if (mode == "d") {
                toggleSkinVisibility(*player, false);
                output.success("已隐藏自定义皮肤");
            } else if (mode == "e") {
                toggleSkinVisibility(*player, true);
                output.success("已显示自定义皮肤");
            } else {
                output.error("无效参数，使用 /skin d 或 /skin e");
            }
        }
    });
}

void handlePlayerJoin(Player& player) {
    for (auto& [name, visible] : skinVisibilityStates) {
        if (!visible) {
            if (auto target = ll::service::getLevel()->getPlayer(name)) {
                setDefaultSkin(*target);
            }
        }
    }
}

void PluginInit() {
    auto& logger = ll::Logger::getInstance("SkinControl");
    logger.info("皮肤控制插件已加载");

    registerCommands();

    ll::event::EventBus::getInstance().subscribe<ll::event::PlayerJoinEvent>([](auto& ev) {
        handlePlayerJoin(ev.mPlayer);
    });
}
} // namespace SkinControl
