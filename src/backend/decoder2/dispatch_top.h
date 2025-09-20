#pragma once

#include "dispatch.h"

// Top level decode table for instructions. Depending on `byte` the handler
// returned might decode a general form instruction or a special form
// instruction.
D_Handler GetDispatcherTop(uint8_t byte);
