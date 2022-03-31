#include "StepExecution.hpp"

#include "src/DebugServers/GdbRsp/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using ResponsePackets::ErrorResponsePacket;

    using Exceptions::Exception;

    void StepExecution::init() {
        if (this->data.size() > 1) {
            this->fromProgramCounter = static_cast<std::uint32_t>(
                std::stoi(std::string(this->data.begin(), this->data.end()), nullptr, 16)
            );
        }
    }

    void StepExecution::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        Logger::debug("Handling StepExecution packet");

        try {
            targetControllerConsole.stepTargetExecution(this->fromProgramCounter);
            debugSession.waitingForBreak = true;

        } catch (const Exception& exception) {
            Logger::error("Failed to step execution on target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
