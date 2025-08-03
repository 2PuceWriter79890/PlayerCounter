#include "mod/MyMod.h"

#include <ll/api/event/EventBus.h>
#include <ll/api/io/Logger.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/actor/player/SerializedSkin.h>

std::shared_ptr<ll::io::Logger> logger = std::make_shared<ll::io::Logger>("SkinPackDetector");

class SkinCheckListener : public ll::event::Listener<ll::event::PlayerJoinEvent> {
public:
    explicit SkinCheckListener() = default;

    void handle(ll::event::PlayerJoinEvent& ev) override {
        auto& player = ev.self();
        
        try {
            if (player.mSkin) {
                const SerializedSkin& skin = *player.mSkin;
                
                bool isOfficial = false;
                if (skin.isAnimated()) {
                    isOfficial = true;
                }
                
                std::string skinId = skin.skinId;
                if (skinId.find("SkinPack_") == 0 || 
                    skinId.find("Persona_") == 0) {
                    isOfficial = true;
                }
                
                std::string resourcePatch = skin.skinResourcePatch;
                if (!resourcePatch.empty() && 
                    (resourcePatch.find("skin_packs") != std::string::npos ||
                     resourcePatch.find("persona") != std::string::npos)) {
                    isOfficial = true;
                }
                
                if (isOfficial) {
                    logger->info("Player {} is using skin pack", player.getRealName());
                    player.sendMessage("Official skin pack detected");
                } else {
                    logger->info("Player {} is using custom skin", player.getRealName());
                    player.sendMessage("Custom skin detected");
                }
            }
        } catch (...) {
            logger->error("Error checking skin for {}", player.getRealName());
        }
    }
};

std::shared_ptr<SkinCheckListener> listener;

extern "C" {
    _declspec(dllexport) void ll_main() {
        logger->info("Loading SkinPackDetector");
        auto& bus = ll::event::EventBus::getInstance();
        listener = bus.emplaceListener<ll::event::PlayerJoinEvent, SkinCheckListener>();
    }

    _declspec(dllexport) void ll_exit() {
        logger->info("Unloading SkinPackDetector");
        if (listener) {
            auto& bus = ll::event::EventBus::getInstance();
            bus.removeListener(listener);
        }
    }
}
