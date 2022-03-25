#include "DebugSession.hpp"

#include "src/Logger/Logger.hpp"

namespace Bloom::DebugServers::Gdb
{
    using Bloom::Targets::TargetDescriptor;

    DebugSession::DebugSession(const Connection& connection, const TargetDescriptor& targetDescriptor)
        : connection(connection)
        , targetDescriptor(targetDescriptor)
    {}

    void DebugSession::terminate() {
        this->connection.close();
    }
}