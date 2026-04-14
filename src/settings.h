#ifndef SETTINGS_H
#define SETTINGS_H

#define FMT_UNICODE 0
#define ZYDIS_STATIC_BUILD
#define FILE_NAME "main.exe"

// Debug
// #define DEBUG_SECTION
#define SHOW_INSTRUCTION
#define HARDCORE_DEOBFUSCATION

#include <Zydis/Zydis.h>

#include "utils/logger.h"
#include "utils/operand.h"

#include <mimalloc/mimalloc.h>
#include <tsl/robin_map.h>
#include <tsl/robin_set.h>

#include <Windows.h>

#include <bitset>
#include <list>
#include <queue>
#include <stack>
#include <string>
#include <vector>

#endif
