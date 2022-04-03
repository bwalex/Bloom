#include "SupportedFeaturesQuery.hpp"

#include <QtCore/QString>

#include "src/DebugServer/Gdb/Feature.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/SupportedFeaturesResponse.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/DebugServer/Gdb/Exceptions/ClientNotSupported.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using ResponsePackets::SupportedFeaturesResponse;
    using ResponsePackets::ErrorResponsePacket;

    using Bloom::Exceptions::Exception;
    using Gdb::Exceptions::ClientNotSupported;

    void SupportedFeaturesQuery::init() {
        /*
         * For qSupported packets, supported and unsupported GDB features are reported in the packet
         * data, where each GDB feature is separated by a semicolon.
         */

        // The "qSupported:" prefix occupies 11 bytes
        if (this->data.size() > 11) {
            const auto packetData = QString::fromLocal8Bit(
                reinterpret_cast<const char*>(this->data.data() + 11),
                static_cast<int>(this->data.size() - 11)
            );

            const auto featureList = packetData.split(";");
            static const auto gdbFeatureMapping = getGdbFeatureToNameMapping();

            for (auto featureName : featureList) {
                // We only care about supported features. Supported features will precede a '+' character.
                if (featureName[featureName.size() - 1] == '+') {
                    featureName.remove('+');

                    const auto feature = gdbFeatureMapping.valueAt(featureName.toStdString());
                    if (feature.has_value()) {
                        this->supportedFeatures.insert(feature.value());
                    }
                }
            }
        }
    }

    void SupportedFeaturesQuery::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        Logger::debug("Handling QuerySupport packet");

        if (!this->isFeatureSupported(Feature::HARDWARE_BREAKPOINTS)
            && !this->isFeatureSupported(Feature::SOFTWARE_BREAKPOINTS)
        ) {
            // All GDB clients are expected to support breakpoints!
            throw ClientNotSupported("GDB client does not support HW or SW breakpoints");
        }

        // Respond with a SupportedFeaturesResponse packet, listing all supported GDB features by Bloom
        debugSession.connection.writePacket(SupportedFeaturesResponse({
            {Feature::SOFTWARE_BREAKPOINTS, std::nullopt},
            {Feature::PACKET_SIZE, std::to_string(debugSession.connection.getMaxPacketSize())},
        }));
    }
}
