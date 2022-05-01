#pragma once

#include <cstdint>

namespace Bloom::TargetController::Responses
{
    enum class ResponseType: std::uint8_t
    {
        GENERIC,
        ERROR,
        TARGET_REGISTERS_READ,
        TARGET_MEMORY_READ,
        TARGET_STATE,
        TARGET_PIN_STATES,
        TARGET_STACK_POINTER,
        TARGET_DESCRIPTOR,
    };
}
