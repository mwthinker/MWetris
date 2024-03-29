#ifndef MWETRIS_NETWORK_PROTOCOL_H
#define MWETRIS_NETWORK_PROTOCOL_H

#include "protobufmessage.h"

#include "game/playerslot.h"

#include <client_to_server.pb.h>

namespace mwetris::network {

	void toTpSlot(const game::PlayerSlot& playerSlot, tp_c2s::Slot& tpSlot);

}

#endif
