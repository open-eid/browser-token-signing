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

#import "cert-class.h"
#import "esteid_certinfo.h"
#import "esteid_log.h"
#import "esteid_error.h"

extern bool allowedSite;
extern char EstEID_error[1024];
extern int EstEID_errorCode;


@implementation cert_class

- (id)init:(EstEID_Map) _certInfo {
	EstEID_log("");
	certInfo = EstEID_mapClone(_certInfo);
	if (!allowedSite) {
		sprintf(EstEID_error, "Site is not allowed");
		EstEID_errorCode = ESTEID_SITE_NOT_ALLOWED;		
		EstEID_log("this site is not allowed to access plugin"); 
	}
	
	return self;
}

- (void)finalizeForWebScript {
	EstEID_log("");
	EstEID_mapFree(certInfo);
}

- (id)valueForKey:(NSString *)key {
	EstEID_log([key UTF8String]);
	const char *result = EstEID_mapGet(certInfo, EstEID_getCertPropertyName([key UTF8String]));
	return result ? [[NSString stringWithUTF8String: result] retain] : nil;
}

-(NSArray *)attributeKeys {
	EstEID_log("");
	NSArray *objects;
	if (allowedSite) {
		objects = [NSArray arrayWithObjects:@"id", @"cert", @"pluginLanguage", @"CN", @"issuerCN", @"keyUsage", @"validFrom", @"validTo", @"certSerialNumber", @"certificateAsPEM", @"certificateAsHex", @"errorCode", nil];
	}
	else {
		objects = [NSArray arrayWithObjects:@"errorCode", nil];
	}
	return objects;
}

+ (BOOL)isKeyExcludedFromWebScript:(const char *)name {
	return NO;
}

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)aSelector {
	return YES;
}

@end
