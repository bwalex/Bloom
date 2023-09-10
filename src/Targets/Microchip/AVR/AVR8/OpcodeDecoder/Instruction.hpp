#pragma once

#include <string>
#include <cstdint>
#include <optional>

#include "src/Targets/TargetMemory.hpp"

namespace Targets::Microchip::Avr::Avr8Bit::OpcodeDecoder
{
    struct Instruction
    {
        enum class Mnemonic: std::uint8_t
        {
            ADC, ADD, ADIW, AND, ANDI, ASR, BCLR, BLD, BRBC, BRBS, BRCC, BRCS, BREAK, BREQ, BRGE, BRHC, BRHS, BRID,
            BRIE, BRLO, BRLT, BRMI, BRNE, BRPL, BRSH, BRTC, BRTS, BRVC, BRVS, BSET, BST, CALL, CBI, CBR, CLC, CLH,
            CLI, CLN, CLR, CLS, CLT, CLV, CLZ, COM, CP, CPC, CPI, CPSE, DEC, DES, EICALL, EIJMP, ELPM, EOR, FMUL,
            FMULS, FMULSU, ICALL, IJMP, IN, INC, JMP, LAC, LAS, LAT, LD, LDD, LDI, LDS, LPM, LSL, LSR, MOV, MOVW,
            MUL, MULS, MULSU, NEG, NOP, OR, ORI, OUT, POP, PUSH, RCALL, RET, RETI, RJMP, ROL, ROR, SBC, SBCI, SBI,
            SBIC, SBIS, SBIW, SBR, SBRC, SBRS, SEC, SEH, SEI, SEN, SER, SES, SET, SEV, SEZ, SLEEP, SPM, ST, STD, STS,
            SUB, SUBI, SWAP, TST, WDR, XCH,
        };

        const std::string& name;
        std::uint32_t opcode;
        std::uint8_t byteSize;
        Mnemonic mnemonic;
        bool canChangeProgramFlow;

        std::optional<std::uint32_t> data;

        std::optional<std::uint8_t> sourceRegister;
        std::optional<std::uint8_t> destinationRegister;

        std::optional<Targets::TargetMemoryAddress> programWordAddress;
        std::optional<std::int16_t> programWordAddressOffset;
        bool canSkipNextInstruction;

        std::optional<std::uint8_t> registerBitPosition;
        std::optional<std::uint8_t> statusRegisterBitPosition;

        std::optional<Targets::TargetMemoryAddress> ioSpaceAddress;
        std::optional<Targets::TargetMemoryAddress> dataSpaceAddress;

        std::optional<std::uint8_t> displacement;

        Instruction(
            const std::string& name,
            std::uint32_t opcode,
            std::uint8_t byteSize,
            Mnemonic mnemonic,
            bool canChangeProgramFlow,
            std::optional<std::uint32_t> data = std::nullopt,
            std::optional<std::uint8_t> sourceRegister = std::nullopt,
            std::optional<std::uint8_t> destinationRegister = std::nullopt,
            std::optional<Targets::TargetMemoryAddress> programWordAddress = std::nullopt,
            std::optional<std::int16_t> programWordAddressOffset = std::nullopt,
            bool canSkipNextInstruction = false,
            std::optional<std::uint8_t> registerBitPosition = std::nullopt,
            std::optional<std::uint8_t> statusRegisterBitPosition = std::nullopt,
            std::optional<Targets::TargetMemoryAddress> ioSpaceAddress = std::nullopt,
            std::optional<Targets::TargetMemoryAddress> dataSpaceAddress = std::nullopt,
            std::optional<std::uint8_t> displacement = std::nullopt
        )
            : name(name)
            , opcode(opcode)
            , byteSize(byteSize)
            , mnemonic(mnemonic)
            , canChangeProgramFlow(canChangeProgramFlow)
            , data(data)
            , sourceRegister(sourceRegister)
            , destinationRegister(destinationRegister)
            , programWordAddress(programWordAddress)
            , programWordAddressOffset(programWordAddressOffset)
            , canSkipNextInstruction(canSkipNextInstruction)
            , registerBitPosition(registerBitPosition)
            , statusRegisterBitPosition(statusRegisterBitPosition)
            , ioSpaceAddress(ioSpaceAddress)
            , dataSpaceAddress(dataSpaceAddress)
            , displacement(displacement)
        {}
    };
}