#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class LeaveProgrammingMode: public Avr8GenericCommandFrame<std::array<unsigned char, 2>>
    {
    public:
        LeaveProgrammingMode() {
            /*
             * The leave programming mode command consists of 2 bytes:
             * 1. Command ID (0x16)
             * 2. Version (0x00)
             */
            this->payload = {
                0x16,
                0x00
            };
        }
    };
}
