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

#include "dialogs.h"
#include "dialogs-cocoa.h"

#ifdef __APPLE__
#ifndef XP_MACOSX
#define XP_MACOSX
#endif
#endif
#include "npapi.h"

NSWindow* getNSWindow(void *nativeWindowHandle) {
	NSWindow* browserWindow = NULL;
	if (nativeWindowHandle) {
		NP_CGContext* npContext = (NP_CGContext*)nativeWindowHandle;
		browserWindow = [[NSWindow alloc] initWithWindowRef:npContext->window];
	}
	return browserWindow;
}

void showAlert(void *nativeWindowHandle, const char *message) {
	NSWindow *browserWindow = getNSWindow(nativeWindowHandle);
	cocoa_showAlert(browserWindow, message);
	[browserWindow release];
}

char* promptForPIN(void *nativeWindowHandle, const char *name, const char *message, unsigned minPin2Length, int usePinPad) {
	NSWindow *browserWindow = getNSWindow(nativeWindowHandle);
	char *pin = cocoa_promptForPIN(browserWindow, name, message, minPin2Length, usePinPad);
	[browserWindow release];
	return pin;
}

int promptForCertificate(void* nativeWindowHandle, char* certId) {
	NSWindow *browserWindow = getNSWindow(nativeWindowHandle);
	int result = cocoa_promptForCertificate(browserWindow, certId);
	[browserWindow release];
	return result;
}
