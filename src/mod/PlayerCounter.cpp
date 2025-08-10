#include "mod/PlayerCounter.h"

#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/platform/UUID.h>
#include <mc/world/actor/player/Player.h>

#include "gmlib/mc/world/actor/OfflinePlayer.h"
#include "gmlib/mc/world/actor/Player.h"

#include <thread>
#include <atomic>

namespace player_counter {

PlayerCounterMod::PlayerCounterMod()
: mSelf(*ll::mod::NativeMod::current()),
  mLogger(mSelf.getLogger()) {}

bool PlayerCounterMod::load() {
    mLogger.info("PlayerCounter 正在加载...");
    return true;
}

bool PlayerCounterMod::enable() {
    mLogger.info("PlayerCounter 正在启用...");
    registerCommand();
    return true;
}

bool PlayerCounterMod::disable() {
    mLogger.info("PlayerCounter 正在卸载...");
    return true;
}

void PlayerCounterMod::registerCommand() {
    using namespace ll::command;

    auto& command =
        CommandRegistrar::getInstance()
            .getOrCreateCommand(
                "playercount",
                "获取服务器存档中的总玩家数量",
                CommandPermissionLevel::Admin
            );

    struct NoParams {};

    command.overload<NoParams>().execute(
        [this](CommandOrigin const& origin, CommandOutput& output, NoParams const& /*params*/) {
            ::Actor* actor = origin.getEntity();
            ::Player* player = dynamic_cast<::Player*>(actor);

            mce::UUID playerUuid = player ? player->getUuid() : mce::UUID::EMPTY();

            output.success("§a正在后台统计玩家数量，请稍候...");

            std::thread([this, playerUuid]() {
                std::atomic<int> count = 0;
                gmlib::OfflinePlayer::forEachOfflinePlayer([&]([[maybe_unused]] gmlib::OfflinePlayer&& player) {
                    count++;
                    return true;
                });

                if (playerUuid == mce::UUID::EMPTY()) {
                    mLogger.info("服务器总计拥有 {} 名玩家的存档数据", count.load());
                    return;
                }
                
                auto targetPlayer = gmlib::OfflinePlayer::fromUuid(playerUuid);
                if (targetPlayer) {
                    if (auto onlinePlayerRef = targetPlayer->getPlayer()) {
                        ::Player& playerRef = onlinePlayerRef.get();
                        
                        std::string message = "§b[PlayerCounter] §e服务器总计拥有 §f" + std::to_string(count) + " §e名玩家的存档数据";
                        playerRef.sendMessage(message);
                        
                        return;
                    }
                }

                mLogger.info("玩家 {} 的统计任务完成，结果: {}", playerUuid.asString(), count.load());

            }).detach();
        }
    );
}

} // namespace player_counter
