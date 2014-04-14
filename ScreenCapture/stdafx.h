#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

//resource
#include "resource.h"

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <atlstr.h>

#include <vector>
#include <stack>

#include <locale.h>  
#include <ShlObj.h>
#include <commdlg.h>


#include "assert.h"

//project header file
#include "RectEx.hpp"
#include "PreDefine.h"

#include <gdiplus.h>
using namespace Gdiplus;

#pragma comment(lib, "gdiplus.lib")

#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")


