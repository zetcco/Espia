#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <stdio.h>

#include "espia_debug.h"
#include "espia_error.h"

void whoami(PSTR buffer, PDWORD size);