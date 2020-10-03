#pragma once

#include <stdint.h>

#ifdef __cplusplus

#include "Protocol.hpp"

namespace Communication {

using Callback = void(*)(const Protocol::PacketInfo&);

void SetCallback(Callback cb);
void Input(const uint8_t *buf, uint16_t len);
bool Send(const Protocol::PacketInfo &packet);
bool SendWithoutPayload(Protocol::PacketType type);

}

extern "C" {
#endif
void communication_usb_input(const uint8_t *buf, uint16_t len);
#ifdef __cplusplus
}
#endif
