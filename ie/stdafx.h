/*
* Chrome Token Signing Native Host
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#define ISOLATION_AWARE_ENABLED 1

#include "targetver.h"
#include "resource.h"
#include <afxrich.h>
#include <atlctl.h>
#include <comutil.h>
 
#include <Windows.h>
#include <cryptuiapi.h>
#include <Knownfolders.h>
#include <ncrypt.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <WinCrypt.h>

#include <algorithm>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <cstring>
#include <cstdarg>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <time.h>
