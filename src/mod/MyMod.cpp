#include "mod/MyMod.h"

#include <ll/api/event/EventBus.h>
#include <ll/api/event/player/PlayerJoinEvent.h>
#include <ll/api/io/Logger.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/actor/player/SerializedSkin.h>

std::shared_ptr<ll::io::Logger> logger = std::make_shared<ll::io::Logger>("SkinPackDetector");

class SkinCheckListener : public ll::event::Listener<ll::event::PlayerJoinEvent> {
public:
    explicit SkinCheckListener() = default;
    ~SkinCheckListener() override = default;

    void handle(ll::event::PlayerJoinEvent& ev) override {
        auto& player = ev.self();
        
        try {
            if (player.mSkin) {
                const SerializedSkin& skin = *player.mSkin;
                bool isOfficial = false;

                if (skin.getAnimationFrames(::persona::AnimatedTextureType::Blinking) > 0.0f ||
                    skin.getAnimationFrames(::persona::AnimatedTextureType::Waving) > 0.0f) {
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
            bus.removeListener(listener->getId());
        }
    }
}
