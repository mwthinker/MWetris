#ifndef MWETRIS_NETWORK_PROTOCOL_H
#define MWETRIS_NETWORK_PROTOCOL_H

#include "protobufmessage.h"

#include "game/playerslot.h"

#include <message.pb.h>

namespace mwetris::network {

	void toTpSlot(const game::PlayerSlot& playerSlot, tetris_protocol::Slot& tpSlot);

}

#endif
