#pragma once

#include "ResponsePacket.hpp"

namespace Bloom::DebugServers::Gdb::ResponsePackets
{
    /**
     * OK response packet expected by the GDB client, in response to certain commands.
     */
    class OkResponsePacket: public ResponsePacket
    {
    public:
        Ok() = default;

        [[nodiscard]] std::vector<unsigned char> getData() const override {
            return {'O', 'K'};
        }
    };
}