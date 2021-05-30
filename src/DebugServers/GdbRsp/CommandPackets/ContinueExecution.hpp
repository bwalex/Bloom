#pragma once

#include <cstdint>
#include <optional>

#include "CommandPacket.hpp"

namespace Bloom::DebugServers::Gdb::CommandPackets
{
    /**
     * The ContinueExecution class implements a structure for "c" packets. These packets instruct the server
     * to continue execution on the target.
     *
     * See @link https://sourceware.org/gdb/onlinedocs/gdb/Packets.html#Packets for more on this.
     */
    class ContinueExecution: public CommandPacket
    {
    private:
        void init();

    public:
        /**
         * The "c" packet can contain an address which defines the point from which the execution should be resumed on
         * the target.
         *
         * Although the packet *can* contain this address, it is not required, hence the optional.
         */
        std::optional<std::uint32_t> fromProgramCounter;

        ContinueExecution(std::vector<unsigned char> rawPacket): CommandPacket(rawPacket) {
            init();
        }

        virtual void dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) override;
    };
}
