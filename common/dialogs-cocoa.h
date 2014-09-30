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

#ifndef __PIN_PROMPT_COCOA_H__
#define __PIN_PROMPT_COCOA_H__

#import <Cocoa/Cocoa.h>

char *cocoa_promptForPIN(NSWindow *window, const char *name, const char *message, unsigned minPin2Length, int usePinPad);

void cocoa_showAlert(NSWindow *window, const char *message);

int cocoa_promptForCertificate(void* nativeWindowHandle, char* certId);

#endif