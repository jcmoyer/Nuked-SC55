#include "path_util.h"
#include <cstdio>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

namespace common
{

std::filesystem::path GetProcessPath()
{
#if defined(_WIN32)
    wchar_t path[MAX_PATH];
    DWORD actual_size = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (actual_size == 0)
    {
        // TODO: handle error
        fprintf(stderr, "fatal: P_GetProcessPath failed\n");
        exit(1);
    }
#elif defined(__APPLE__)
    char path[1024];
    uint32_t actual_size = 1024;
    if (_NSGetExecutablePath(path, &actual_size) != 0)
    {
        // TODO: handle error
        fprintf(stderr, "fatal: P_GetProcessPath failed\n");
        exit(1);
    }
#else
    char path[PATH_MAX];
    ssize_t actual_size = readlink("/proc/self/exe", path, PATH_MAX);
    if (actual_size == -1)
    {
        // TODO: handle error
        fprintf(stderr, "fatal: P_GetProcessPath failed\n");
        exit(1);
    }
#endif
    return std::filesystem::path(path, path + (size_t)actual_size);
}

}
