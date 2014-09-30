/*
 * Estonian ID card plugin for web browsers
 *
 * Copyright (C) 2010-2011 Codeborne <info@codeborne.com>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdio.h>
#include "esteid_dialog_common.h"
#include "l10n.h"

#define TITLE_BUFFER_SIZE 512
char* createDialogTitle(const char* nameAndID) {
  static char title[TITLE_BUFFER_SIZE];
#ifndef _WIN32
  snprintf(title, TITLE_BUFFER_SIZE, "%s - %s", l10n("Signing"), nameAndID);
#else
  sprintf_s(title, TITLE_BUFFER_SIZE, "%s - %s", l10n("Signing"), nameAndID);
#endif
  return title;
}

