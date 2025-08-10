#pragma once

#include <ll/api/mod/NativeMod.h>
#include <ll/api/io/Logger.h>

namespace player_counter {

class PlayerCounterMod {
public:
    PlayerCounterMod()
    : mSelf(*ll::mod::NativeMod::current()),
      mLogger(mSelf.getLogger()) {}

    bool load();
    bool enable();
    bool disable();

private:
    void registerCommand();

    ll::mod::NativeMod& mSelf;
    ll::io::Logger&     mLogger;
};

} // namespace player_counter