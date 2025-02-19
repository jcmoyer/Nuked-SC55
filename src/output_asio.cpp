#include "output_asio.h"

#include "asiosys.h"
#include "asio.h"
#include "asiodrivers.h"

struct GlobalAsioState
{
    AsioDrivers drivers;
};

// there isn't a way around using globals here, the ASIO API doesn't accept
// arbitrary userdata in its callbacks
GlobalAsioState g_asio_state;

bool Out_ASIO_QueryOutputs(AudioOutputList& list)
{
    const size_t MAX_NAMES    = 32;
    const size_t MAX_NAME_LEN = 32;

    // TODO: wat. do we seriously need to allocate all this?
    char* names[MAX_NAMES];
    for (size_t i = 0; i < MAX_NAMES; ++i)
    {
        names[i] = (char*)malloc(MAX_NAME_LEN);
    }

    long names_count = g_asio_state.drivers.getDriverNames(names, MAX_NAMES);
    for (long i = 0; i < names_count; ++i)
    {
        list.push_back({.name = names[i], .kind = AudioOutputKind::ASIO});
    }

    for (size_t i = 0; i < MAX_NAMES; ++i)
    {
        free(names[i]);
    }

    return true;
}
