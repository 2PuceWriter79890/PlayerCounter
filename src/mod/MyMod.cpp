#include "mod/MyMod.h"

#include <ll/api/event/EventBus.h>
#include <ll/api/event/player/PlayerJoinEvent.h>
#include <ll/api/io/Logger.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/actor/player/SerializedSkin.h>

std::shared_ptr<ll::io::Logger> logger = std::make_shared<ll::io::Logger>("SkinPackDetector");

void onPlayerJoin(ll::event::player::PlayerJoinEvent& ev) {
    auto& player = ev.self();
    
    try {
        if (player.mSkin) {
            const auto& skin = *player.mSkin;
            std::string skinId = skin.skinId;
            
            bool isOfficial = 
                skinId.find("SkinPack_") != std::string::npos ||
                skinId.find("Persona_") != std::string::npos ||
                skinId.find("Geometry_") != std::string::npos;

            if (isOfficial) {
                logger->info("Player {} is using skin pack (ID: {})", 
                           player.getRealName(), skinId);
                player.sendMessage("Official skin pack detected");
            } else {
                logger->info("Player {} is using custom skin (ID: {})", 
                           player.getRealName(), skinId);
                player.sendMessage("Custom skin detected");
            }
        }
    } catch (...) {
        logger->error("Error checking skin for {}", player.getRealName());
    }
}

ll::event::ListenerPtr listener;

extern "C" {
    _declspec(dllexport) void ll_main() {
        logger->info("Loading SkinPackDetector");
        auto& bus = ll::event::EventBus::getInstance();
        listener = bus.emplaceListener<ll::event::player::PlayerJoinEvent>(onPlayerJoin);
    }

    _declspec(dllexport) void ll_exit() {
        logger->info("Unloading SkinPackDetector");
        if (listener) {
            auto& bus = ll::event::EventBus::getInstance();
            bus.removeListener(listener);
        }
    }
}
