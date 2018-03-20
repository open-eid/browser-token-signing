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

#import <SafariServices/SafariServices.h>

static SFSafariPage *_page = nil;

@interface SafariExtensionHandler : SFSafariExtensionHandler

@end

@implementation SafariExtensionHandler

- (void)messageReceivedWithName:(NSString *)messageName fromPage:(SFSafariPage *)page userInfo:(NSDictionary *)userInfo {
    // This method will be called when a content script provided by your extension calls safari.extension.dispatchMessage("message").
    [page getPagePropertiesWithCompletionHandler:^(SFSafariPageProperties *properties) {
        NSLog(@"The extension received a message (%@) from a script injected into (%@) with userInfo (%@)", messageName, properties.url, userInfo);
        if (![messageName isEqualToString:@"message"]) {
            return;
        }
        _page = page;
        NSUserDefaults *defaults = [[NSUserDefaults alloc] initWithSuiteName:@"ee.ria.SafariApp.shared"];
        [defaults setObject:userInfo forKey:userInfo[@"nonce"]];
        [defaults synchronize];
        [NSDistributedNotificationCenter.defaultCenter postNotificationName:@"TokenSigning" object:userInfo[@"nonce"] userInfo:nil deliverImmediately:YES];
    }];
}

- (void)messageReceivedFromContainingAppWithName:(NSString *)messageName userInfo:(NSDictionary<NSString *,id> *)userInfo {
    NSLog(@"The extension received a message (%@) from a application with userInfo (%@)", messageName, userInfo);
    [_page dispatchMessageToScriptWithName:@"message" userInfo:userInfo];
}

@end
