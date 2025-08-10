#include "PlayerCounter.h"
#include <ll/api/mod/RegisterHelper.h>

namespace {
player_counter::PlayerCounterMod instance;
} // namespace

LL_REGISTER_MOD(player_counter::PlayerCounterMod, instance);
