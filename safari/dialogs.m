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

#include "CoreFoundation/CoreFoundation.h"
#include "Cocoa/Cocoa.h"

#include <string.h>
#include "PINPanel.h"

#include "dialogs.h"
#include "dialogs-cocoa.h"

void showAlert(void *nativeWindowHandle, const char *message) {
	cocoa_showAlert((NSWindow *)nativeWindowHandle, message);
}

char *promptForPIN(void *nativeWindowHandle, const char *name, const char *message, unsigned minPin2Length, int usePinPad) {
	return cocoa_promptForPIN((NSWindow *)nativeWindowHandle, name, message, minPin2Length, usePinPad);
}

int promptForCertificate(void* nativeWindowHandle, char* certId) {
	return cocoa_promptForCertificate(nativeWindowHandle, certId);
}
