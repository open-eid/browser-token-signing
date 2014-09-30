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

#include "dialogs-cocoa.h"
#include "dialogs.h"
#include "PINPanel.h"
#include "CertificateSelection.h"
#include "esteid_log.h"

char *cocoa_promptForPIN(NSWindow *window, const char *name, const char *message, unsigned minPin2Length, int usePinPad) {
	EstEID_log("parameters: window=%p, name=%s, message=%s, minPin2Length=%u, usePinPad=%i", window, name, message, minPin2Length, usePinPad);
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];		
	PINPanel *pinPanel = [[PINPanel alloc] init];	
	char *pin = strdup([[pinPanel showForWindow: window withName: name withMessage: message withMinAcceptablePin2Length: minPin2Length usePinPad: usePinPad] UTF8String]);
	LOG_LOCATION;
	[pool release];	
	LOG_LOCATION;
	return pin;
}

void cocoa_showAlert(NSWindow *window, const char *message) {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSAlert *alert = [[[NSAlert alloc] init] autorelease];
	[alert addButtonWithTitle:@"OK"];
	[alert setMessageText:[NSString stringWithUTF8String: message]];
	[alert setAlertStyle:NSCriticalAlertStyle];
    [alert beginSheetModalForWindow: window modalDelegate: nil didEndSelector: nil contextInfo: nil];
	[pool release];
}

int cocoa_promptForCertificate(void* nativeWindowHandle, char* certId) {
	LOG_LOCATION;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	CertificateSelection *certificateSelection = [[CertificateSelection alloc] init];
	NSString* selectedCertificateId = [certificateSelection showForWindow: nativeWindowHandle];
	EstEID_log("selectedCertificate length = %u", [selectedCertificateId length]);
	strcpy(certId, [selectedCertificateId UTF8String]);
	int returnValue = IDOK;
	
	if (([selectedCertificateId length]) == 0) {
		returnValue = IDCANCEL;
	}
	[pool release];
	return returnValue;
}

void closePinPadModalSheet() {
	LOG_LOCATION;
	[NSApp abortModal];
}
