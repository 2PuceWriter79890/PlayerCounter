#include "mod/MyMod.h"

#include <ll/api/event/EventBus.h>
#include <ll/api/io/Logger.h>
#include <mc/network/SubClientConnectionRequest.h>
#include <mc/world/actor/player/Player.h>

std::shared_ptr<ll::io::Logger> logger = std::make_shared<ll::io::Logger>("SkinPackDetector");

bool isFromSkinPack(const std::string& skinId, const std::string& resourcePatch, bool isPersona) {
    if (skinId.find("SkinPack_") == 0 || 
        skinId.find("Persona_") == 0 ||
        skinId.find("Geometry_") == 0) {
        return true;
    }
    
    if (isPersona) {
        return true;
    }
    
    if (!resourcePatch.empty() && 
        (resourcePatch.find("skin_packs") != std::string::npos ||
         resourcePatch.find("persona") != std::string::npos)) {
        return true;
    }
    
    return false;
}

void onPlayerJoin(ll::event::PlayerJoinEvent& ev) {
    auto& player = ev.self();
    
    try {
        if (auto connReq = player.getConnectionRequest()) {
            std::string skinId = connReq->getSkinId();
            std::string resourcePatch = connReq->getSkinResourcePatch();
            bool isPersona = connReq->isPersonaSkin();
            
            if (isFromSkinPack(skinId, resourcePatch, isPersona)) {
                logger->info("Player {} is using skin pack character (ID: {})", 
                           player.getRealName(), skinId);
                player.sendMessage("You are using an official skin pack");
            } else {
                logger->info("Player {} is using custom skin (ID: {})", 
                           player.getRealName(), skinId);
                player.sendMessage("You are using a custom skin");
            }
        } else {
            logger->warn("Player {} has no connection request data", player.getRealName());
        }
    } catch (...) {
        logger->error("Error checking skin for player {}", player.getRealName());
    }
}

extern "C" {
    _declspec(dllexport) void ll_main() {
        logger->info("Skin Pack Detector loaded");
        ll::event::EventBus::getInstance().subscribe<ll::event::PlayerJoinEvent>(onPlayerJoin);
    }

    _declspec(dllexport) void ll_exit() {
        ll::event::EventBus::getInstance().unsubscribeAll();
        logger->info("Skin Pack Detector unloaded");
    }
}
