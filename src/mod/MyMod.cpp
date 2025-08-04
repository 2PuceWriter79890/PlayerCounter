#include "mod/MyMod.h"

#include <ll/api/event/Listener.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/player/PlayerJoinEvent.h>
#include <ll/api/io/Logger.h>
#include <mc/world/actor/player/Player.h>

std::shared_ptr<ll::io::Logger> logger = std::make_shared<ll::io::Logger>("SkinPackDetector");

class SkinCheckListener : public ll::event::Listener<ll::event::player::PlayerJoinEvent> {
public:
    explicit SkinCheckListener() = default;
    ~SkinCheckListener() override = default;

    void handle(ll::event::player::PlayerJoinEvent& ev) override {
        auto& player = ev.self();
        
        try {
            bool isOfficial = false;
            
            if (player.getCommandPermissionLevel() > 1) {
                isOfficial = true;
            }
            
            if (player.getPlayerGameType() == GameType::Creative) {
                isOfficial = true;
            }
            
            std::string deviceId = player.getDeviceId();
            if (!deviceId.empty() && deviceId.find("Xbox") != std::string::npos) {
                isOfficial = true;
            }
            
            if (isOfficial) {
                logger->info("Player {} is using official content", player.getRealName());
                player.sendMessage("Official content detected");
            } else {
                logger->info("Player {} is using custom content", player.getRealName());
                player.sendMessage("Custom content detected");
            }
        } catch (...) {
            logger->error("Error checking player {}", player.getRealName());
        }
    }
};

std::shared_ptr<SkinCheckListener> listener;

extern "C" {
    _declspec(dllexport) void ll_main() {
        logger->info("Loading SkinPackDetector");
        auto& bus = ll::event::EventBus::getInstance();
        listener = std::make_shared<SkinCheckListener>();
        bus.addListener(listener);
    }

    _declspec(dllexport) void ll_exit() {
        logger->info("Unloading SkinPackDetector");
        if (listener) {
            auto& bus = ll::event::EventBus::getInstance();
            bus.removeListener(listener);
        }
    }
}
