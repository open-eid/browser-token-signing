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

#include <stdlib.h>
#include "Cocoa/Cocoa.h"
#include "esteid_log.h"
#include "l10n.h"

char *EstEID_getUserLocale() {
	char *result = NULL;
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	NSString *homePath = [NSString stringWithUTF8String: getenv("HOME")];
    NSString *file = [homePath stringByAppendingString: @"/Library/Preferences/com.estonian-id-card.plist"];
	NSMutableDictionary* plist = [[NSMutableDictionary alloc] initWithContentsOfFile:file];
	NSString *language = [plist objectForKey:@"Main.Language"];
	if (language) {
		EstEID_log("locale from plist file: %s", [language UTF8String]);
		result = strdup([language UTF8String]);
	}
	if (!result) result = strdup("en");
	[pool release];
	return result;
}

