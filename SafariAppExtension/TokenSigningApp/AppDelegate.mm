/*
 * Safari Token Signing
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

#import "CertificateSelection.h"
#import "PINDialog.h"
#import "Labels.h"
#import "../TokenSigningExtension/TokenSigning.h"

#import <SafariServices/SafariServices.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    LSSharedFileListRef list = LSSharedFileListCreate(kCFAllocatorDefault, kLSSharedFileListSessionLoginItems, NULL);
    if (list) {
        CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
        NSDictionary *props = @{(__bridge NSString*)kLSSharedFileListLoginItemHidden: @(YES)};
        LSSharedFileListItemRef item = LSSharedFileListInsertItemURL(
            list, kLSSharedFileListItemLast, NULL, NULL, url, (__bridge CFDictionaryRef)props, NULL);
        if (item) {
            CFRelease(item);
        }
        CFRelease(url);
        CFRelease(list);
    }
#pragma clang diagnostic pop

    [SFSafariExtensionManager getStateOfSafariExtensionWithIdentifier:TokenSigningExtension completionHandler:^(SFSafariExtensionState *state, NSError *error) {
        NSLog(@"Extension state %@, error %@", @(state ? state.enabled : 0), error);
        if (!state.enabled) {
            [SFSafariApplication showPreferencesForExtensionWithIdentifier:TokenSigningExtension completionHandler:nil];
        }
    }];

    [NSDistributedNotificationCenter.defaultCenter addObserver:self selector:@selector(notificationEvent:) name:TokenSigning object:nil];
}

-(void)notificationEvent:(NSNotification *)notification {
    NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:TokenSigningShared];
    NSMutableDictionary *resp = [[defaults dictionaryForKey:notification.object] mutableCopy];
    [defaults removeObjectForKey:notification.object];
    [defaults synchronize];
    NSLog(@"request %@", resp);
    resp[@"src"] = @"background.js";
    resp[@"extension"] = [NSString stringWithFormat:@"%@.%@", NSBundle.mainBundle.infoDictionary[@"CFBundleShortVersionString"], NSBundle.mainBundle.infoDictionary[@"CFBundleVersion"]];
    resp[@"result"] = @"ok";

    if (resp[@"lang"]) {
        Labels::l10n.setLanguage([resp[@"lang"] UTF8String]);
    }

    if ([resp[@"type"] isEqualToString:@"VERSION"]) {
        resp[@"version"] = resp[@"extension"];
    }
    else if ([resp[@"origin"] compare:@"https" options:NSCaseInsensitiveSearch range:NSMakeRange(0, 5)]) {
        resp[@"result"] = @"not_allowed";
    }
    else if ([resp[@"type"] isEqualToString:@"CERT"]) {
        NSDictionary *cert = [CertificateSelection show:![@"AUTH" isEqualToString:resp[@"filter"]]];
        resp[@"cert"] = (NSString*)cert[@"cert"];
    }
    else if ([resp[@"type"] isEqualToString:@"SIGN"]) {
        NSDictionary *sign = [PINPanel show:resp cert:resp[@"cert"]];
        resp[@"signature"] = sign[@"signature"];
    }
    else {
        resp[@"result"] = @"invalid_argument";
    }
    NSLog(@"response %@", resp);
    [SFSafariApplication dispatchMessageWithName:TokenSigningMessage toExtensionWithIdentifier:TokenSigningExtension userInfo:resp completionHandler:nil];
}

@end
