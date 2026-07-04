#include "ff.h"

DWORD get_fattime(void)
{
    return ((DWORD)(2026 - 1980) << 25)
         | ((DWORD)7 << 21)
         | ((DWORD)2 << 16);
}