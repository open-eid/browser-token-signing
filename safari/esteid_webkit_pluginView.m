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

#import "esteid_webkit_pluginView.h"
#import "dialogs.h"

#include "cert-class.h"
#include "esteid_log.h"
#include "esteid_sign.h"
#include "esteid_json.h"
#include "l10n.h"
#include "esteid_mime_types.h"
#include "esteid_error.h"

extern char EstEID_error[1024];
extern int EstEID_errorCode;

BOOL allowedSite = false;

@interface esteid_webkit_pluginView (Internal)
- (id)_initWithArguments:(NSDictionary *)arguments;
@end

@implementation esteid_webkit_pluginView

+ (NSView *)plugInViewWithArguments:(NSDictionary *)newArguments {
	LOG_LOCATION;
    return [[[self alloc] _initWithArguments:newArguments] autorelease];
}

+ (BOOL)accessInstanceVariablesDirectly { return NO; }

- (NSString *)errorMessage { 
	LOG_LOCATION;
	return [[NSString stringWithUTF8String: EstEID_error] retain]; 
}

- (NSInteger)errorCode { 
	LOG_LOCATION;
	return EstEID_errorCode; 
}

- (NSString *)version { 
	LOG_LOCATION;
	return version; 
}

- (NSString *)pluginLanguage {
	LOG_LOCATION;
	return pluginLanguage;
}

- (void)setPluginLanguage:(NSString*) language {
	LOG_LOCATION;
	if ([NSStringFromClass([language class]) isEqual:@"WebUndefined"]) {
		EstEID_log("plugin language not defined");
		pluginLanguage = @"";
	}
	else {
		pluginLanguage = language;
	}

}


- (void)webPlugInInitialize {
	EstEID_log("version: %s", ESTEID_PLUGIN_VERSION);
	version = @ESTEID_PLUGIN_VERSION;
	if (!EstEID_loadLibrary()) {
		char messageBuffer[2048];
		sprintf(messageBuffer, "%s: %s", l10n("EstEID Plugin initialization failed"), EstEID_error);
		showAlert([self window], messageBuffer);		
	}
}

- (void)webPlugInDestroy {
	LOG_LOCATION;
	EstEID_freeCerts();
}

- (id)objectForWebScript { return self; }

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)selector {
	LOG_LOCATION;
	if (selector == @selector(signBy:hash:withOptions:)) return NO;
	if (selector == @selector(getCertificate)) return NO;
	
	return YES;
}

+ (NSString *)webScriptNameForSelector:(SEL)selector {
	LOG_LOCATION;
	if (selector == @selector(signBy:hash:withOptions:)) return @"sign";
	if (selector == @selector(getCertificate)) return @"getCertificate";
    
	return nil;
}

+ (BOOL)isKeyExcludedFromWebScript:(const char *)name {
	//if (strcmp(name, "version") == 0) return NO; 
	return NO;
}

- (cert_class *)getCertificate {
	LOG_LOCATION;	
	EstEID_setLocale([[self pluginLanguage] UTF8String]);

	char selectedCertID[33];	
	int dlg_result = promptForCertificate([self window], selectedCertID);
	EstEID_log("Certificate selection dialog result = %i", dlg_result);
	EstEID_log("Selected certificate ID = %s", selectedCertID);
	
	if(dlg_result == IDCANCEL) {
		sprintf(EstEID_error, "User canceled");
		EstEID_errorCode = ESTEID_USER_CANCEL;
		EstEID_log("EstEID_error=%s", EstEID_error);
		[WebScriptObject throwException:[[[NSString alloc] initWithUTF8String: EstEID_error] retain]];
		return NULL;
	}
		
	return [[[cert_class alloc] init:EstEID_getNonRepudiationCertById(selectedCertID)] retain];
}

- (const char*)getLanguageFromOptions:(WebScriptObject *)options {
	LOG_LOCATION;
	const char* lang = NULL;
	@try {
		id languageValue = [options valueForKey:@"language"];
		const char *className = object_getClassName(languageValue);
		if (!strcmp("NSCFString", className)) {
			lang = [languageValue UTF8String];
		}
	}
	@catch (NSException *ignored) {
		lang = [pluginLanguage UTF8String];
	}
	return lang;
}

- (NSString *)signBy:(NSString *)cn hash:(NSString *)hash withOptions:(WebScriptObject *)options {
	LOG_LOCATION;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	EstEID_setLocale([self getLanguageFromOptions:options]);
	
	EstEID_PINPromptData pinPromptData = { promptForPIN, showAlert, [self window] };
	char *signature = EstEID_sign(strdup([cn UTF8String]), strdup([hash UTF8String]), pinPromptData);
	
	[pool release];
	if (signature) {
		NSString *result = [[[NSString alloc] initWithUTF8String: signature] retain];
		free(signature);
		return [result retain];
	}
	else {		
		[WebScriptObject throwException:[[[NSString alloc] initWithUTF8String: EstEID_error] retain]];
		return NULL;
	}
}

@end


@implementation esteid_webkit_pluginView (Internal)

- (id)_initWithArguments:(NSDictionary *)newArguments {
	LOG_LOCATION;
		
#ifndef DEVELOPMENT_MODE
	NSString *siteAddress = [[newArguments objectForKey:@"WebPlugInBaseURLKey"] description];
	NSRange prefixRange = [siteAddress rangeOfString:@"https://"
											 options:(NSAnchoredSearch | NSCaseInsensitiveSearch)];	
	if (prefixRange.location != NSNotFound) {
		allowedSite = true;
		EstEID_log("%s is allowed to access plugin", [siteAddress UTF8String]); 
	}
	else {
		allowedSite = false;
		EstEID_log("%s is not allowed to access plugin", [siteAddress UTF8String]);
	}
#else	
	allowedSite = true;
	EstEID_log("*** Built in development mode, all sites are allowed to access plugin ***"); 	
#endif
	
	NSDictionary *pluginAttributes = [newArguments objectForKey:@"WebPlugInAttributesKey"];	
	NSString *mimeType = [[pluginAttributes objectForKey:@"type"] description];
	EstEID_log("MIME Type: %s", [mimeType UTF8String]);

    if (!(self = [super initWithFrame:NSZeroRect]))
        return nil;
    return self;
}

@end
