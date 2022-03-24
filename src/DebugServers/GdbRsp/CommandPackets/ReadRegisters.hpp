#pragma once

#include <optional>

#include "CommandPacket.hpp"

#include "src/DebugServers/GdbRsp/RegisterDescriptor.hpp"

namespace Bloom::DebugServers::Gdb::CommandPackets
{
    /**
     * The ReadRegisters class implements a structure for "g" and "p" command packets. In response to these
     * packets, the server is expected to send register values for all registers (for "g" packets) or for a single
     * register (for "p" packets).
     */
    class ReadRegisters: public CommandPacket
    {
    public:
        /**
         * "p" packets include a register number to indicate which register is requested for reading. When this is set,
         * the server is expected to respond with only the value of the requested register.
         *
         * If the register number is not supplied (as is the case with "g" packets), the server is expected to respond
         * with values for all registers.
         */
        std::optional<GdbRegisterNumberType> registerNumber;

        explicit ReadRegisters(const std::vector<unsigned char>& rawPacket): CommandPacket(rawPacket) {
            init();
        };

        void handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) override;

    private:
        void init();
    };
}
