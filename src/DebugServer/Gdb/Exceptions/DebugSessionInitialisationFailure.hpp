#pragma once

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::Exceptions
{
    /**
     * The GDB server may fail to prepare for a debug session, if an internal error occurs. One circumstance where
     * this can happen is when the TargetController is not able to service the debug session for whatever reason.
     *
     * See GdbRspDebugServer::serve() for handling code.
     */
    class DebugSessionInitialisationFailure: public Bloom::Exceptions::Exception
    {
    public:
        explicit DebugSessionInitialisationFailure(const std::string& message)
            : Bloom::Exceptions::Exception(message)
        {
            this->message = message;
        }

        explicit DebugSessionInitialisationFailure(const char* message)
            : Bloom::Exceptions::Exception(message)
        {
            this->message = std::string(message);
        }

        explicit DebugSessionInitialisationFailure() = default;
    };
}
