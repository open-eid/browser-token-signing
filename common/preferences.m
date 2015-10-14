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

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#include "preferences.h"
#include "esteid_log.h"
#include "dialogs.h"
#include <dlfcn.h>

void saveDefaultPreferences() {
    LOG_LOCATION
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    if([defaults objectForKey:@"esteid.pkcs11.paths"] == nil) {
        EstEID_log("Setting pkcs11 module search paths");
        NSMutableDictionary *dic = [[NSMutableDictionary  alloc] init];
        [dic setObject:@"/Library/EstonianIDCard/lib/esteid-pkcs11.so" forKey:@"EST"];
        [dic setObject:@"/Library/OpenSC/lib/opensc-pkcs11.so" forKey:@"FIN"];
        //[dic setObject:@"/Library/OpenSC/lib/opensc-pkcs11_.so" forKey:@"LTU"];
        [dic setObject:@"/Library/latvia-eid/lib/otlv-pkcs11.so" forKey:@"LAT"];
        
        [defaults setObject:dic forKey:@"esteid.pkcs11.paths"];
    }
    //[defaults removeObjectForKey:@"esteid.pkcs11.path.default"];
    [defaults synchronize];
}

void saveDefaultPKCS11ModulePath(const char *path) {
    EstEID_log("Saving default pkcs11 module path as %s", path);
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    [defaults setObject:[NSString stringWithUTF8String:path] forKey:@"esteid.pkcs11.path.default"];
    [defaults synchronize];
}

const char *loadDefaultPKCS11ModulePath() {
    LOG_LOCATION
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    if([defaults stringForKey:@"esteid.pkcs11.path.default"] != nil) {
        return [[defaults stringForKey:@"esteid.pkcs11.path.default"] UTF8String];
    } else {
        NSDictionary *pkcs11PathDictionary = [defaults dictionaryForKey:@"esteid.pkcs11.paths"];
        return [[pkcs11PathDictionary objectForKey:@"EST"] UTF8String];
    }
}

bool isDefaultPKCS11ModulePathSet() {
    bool defaultPathSet = [[NSUserDefaults standardUserDefaults] stringForKey:@"esteid.pkcs11.path.default"] != nil;
    EstEID_log("default pkcs11 module path set: %s", defaultPathSet ? "true" : "false");
    return defaultPathSet;
}

EstEID_Map getAvailablePKCS11Modules() {
    LOG_LOCATION
    EstEID_Map availableModules = NULL;
    NSDictionary *pkcs11PathDictionary = [[NSUserDefaults standardUserDefaults] dictionaryForKey:@"esteid.pkcs11.paths"];
    for (NSString * key in [pkcs11PathDictionary allKeys]) {
        if (dlopen_preflight([[pkcs11PathDictionary objectForKey:key] UTF8String])) {
            EstEID_log("PKCS11 module for: %s -> %s available", [key UTF8String], [[pkcs11PathDictionary objectForKey:key] UTF8String]);
            availableModules = EstEID_mapPut(availableModules, [key UTF8String], [[pkcs11PathDictionary objectForKey:key] UTF8String]);
        } else {
            EstEID_log("PKCS11 module for: %s -> %s not available", [key UTF8String], [[pkcs11PathDictionary objectForKey:key] UTF8String]);

        }
    }
    return availableModules;

}


