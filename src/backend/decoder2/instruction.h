#pragma once

#include <cstdint>

struct mcu_t;

namespace decoder2
{

// Contains all of the parameters for a fully decoded instruction.
struct InstructionParams
{
    union {
        uint16_t ea_data; // used by addressing modes that include generic data in the EA field
        int16_t  ea_disp; // used by addressing modes that include a displacement in the EA field
        uint16_t br_true; // only used for short form branch instructions
    };

    union {
        uint8_t op_reg;  // used by instructions that encode a general purpose register as part of the opcode
        uint8_t op_c;    // used by instructions that encode a control register as part of the opcode
        uint8_t op_page; // used by instructions that encode an immediate page in addition to immediate address
    };

    uint8_t ea_reg; // used by addressing modes that refer to a register

    union {
        uint16_t op_data;  // used by instructions that have immediate data
        uint16_t br_false; // only used for short form branch instructions
    };
};

using InstructionHandler = void (*)(mcu_t&, const InstructionParams&);

struct DecodedInstruction
{
    InstructionHandler handler;
    InstructionParams  params;
};

} // namespace decoder2
