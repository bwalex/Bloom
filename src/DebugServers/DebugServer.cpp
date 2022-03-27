#include "DebugServer.hpp"

#include <variant>

// Debug server implementations
#include "GdbRsp/AvrGdb/AvrGdbRsp.hpp"

#include "src/Exceptions/InvalidConfig.hpp"
#include "src/Logger/Logger.hpp"

namespace Bloom::DebugServers
{
    using namespace Bloom::Events;

    DebugServer::DebugServer(const DebugServerConfig& debugServerConfig)
        : debugServerConfig(debugServerConfig)
    {}

    void DebugServer::run() {
        try {
            this->startup();

            Logger::info("DebugServer ready");

            while (this->getThreadState() == ThreadState::READY) {
                this->server->run();
                this->eventListener->dispatchCurrentEvents();
            }
        } catch (const std::exception& exception) {
            Logger::error("DebugServer fatal error: " + std::string(exception.what()));
        }

        this->shutdown();
    }

    std::map<std::string, std::function<std::unique_ptr<ServerInterface>()>> DebugServer::getAvailableServersByName() {
        return std::map<std::string, std::function<std::unique_ptr<ServerInterface>()>> {
            {
                "avr-gdb-rsp",
                [this] () -> std::unique_ptr<ServerInterface> {
                    return std::make_unique<DebugServers::Gdb::AvrGdb::AvrGdbRsp>(
                        this->debugServerConfig,
                        *(this->eventListener.get())
                    );
                }
            },
        };
    }
    void DebugServer::startup() {
        this->setName("DS");
        Logger::info("Starting DebugServer");

        EventManager::registerListener(this->eventListener);
        this->eventListener->setInterruptEventNotifier(&this->interruptEventNotifier);

        // Register event handlers
        this->eventListener->registerCallbackForEventType<Events::ShutdownDebugServer>(
            std::bind(&DebugServer::onShutdownDebugServerEvent, this, std::placeholders::_1)
        );

        static const auto availableServersByName = this->getAvailableServersByName();
        if (!availableServersByName.contains(this->debugServerConfig.name)) {
            throw Exceptions::InvalidConfig(
                "DebugServer \"" + this->debugServerConfig.name + "\" not found."
            );
        }

        this->server = availableServersByName.at(this->debugServerConfig.name)();
        Logger::info("Selected DebugServer: " + this->server->getName());

        this->server->init();
        this->setThreadStateAndEmitEvent(ThreadState::READY);
    }

    void DebugServer::shutdown() {
        if (this->getThreadState() == ThreadState::STOPPED
            || this->getThreadState() == ThreadState::SHUTDOWN_INITIATED
        ) {
            return;
        }

        this->setThreadState(ThreadState::SHUTDOWN_INITIATED);
        Logger::info("Shutting down DebugServer");
        this->server->close();
        this->setThreadStateAndEmitEvent(ThreadState::STOPPED);
        EventManager::deregisterListener(this->eventListener->getId());
    }

    void DebugServer::onShutdownDebugServerEvent(const Events::ShutdownDebugServer& event) {
        this->shutdown();
    }
}
