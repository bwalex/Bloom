#pragma once

#include <optional>

#include "Command.hpp"
#include "src/TargetController/Responses/TargetMemoryRead.hpp"

#include "src/Targets/TargetRegister.hpp"

namespace Bloom::TargetController::Commands
{
    class ReadTargetMemory: public Command
    {
    public:
        using SuccessResponseType = Responses::TargetMemoryRead;

        static constexpr CommandType type = CommandType::READ_TARGET_MEMORY;
        static inline const std::string name = "ReadTargetMemory";

        Targets::TargetMemoryType memoryType;
        std::uint32_t startAddress;
        std::uint32_t bytes;
        std::set<Targets::TargetMemoryAddressRange> excludedAddressRanges;

        explicit ReadTargetMemory(
            Targets::TargetMemoryType memoryType,
            std::uint32_t startAddress,
            std::uint32_t bytes,
            const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges
        )
            : memoryType(memoryType)
            , startAddress(startAddress)
            , bytes(bytes)
            , excludedAddressRanges(excludedAddressRanges)
        {};

        [[nodiscard]] CommandType getType() const override {
            return ReadTargetMemory::type;
        }
    };
}