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

#import "SafariExtensionHandler.h"
#import "SafariExtensionViewController.h"

#if 0
#import "BinaryUtils.h"
#import "PKCS11CardManager.h"
#import "PKCS11Path.h"
#endif

@interface SafariExtensionHandler ()
@end

@implementation SafariExtensionHandler

- (void)messageReceivedWithName:(NSString *)messageName fromPage:(SFSafariPage *)page userInfo:(NSDictionary *)userInfo {
    // This method will be called when a content script provided by your extension calls safari.extension.dispatchMessage("message").
    [page getPagePropertiesWithCompletionHandler:^(SFSafariPageProperties *properties) {
        NSLog(@"The extension received a message (%@) from a script injected into (%@) with userInfo (%@)", messageName, properties.url, userInfo);

        if ([messageName isEqualToString:@"cert"]) {
#if 0
            NSMutableArray *certificates = [[NSMutableArray alloc] init];
            try {
                NSDateFormatter *df = [[NSDateFormatter alloc] init];
                df.dateFormat = @"dd.MM.YYYY";
                NSDateFormatter *asn1 = [[NSDateFormatter alloc] init];
                asn1.dateFormat = @"yyyyMMddHHmmss'Z'";
                asn1.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
                std::string pkcs11ModulePath(PKCS11Path::getPkcs11ModulePath());
                for (auto &token : PKCS11CardManager::instance(pkcs11ModulePath)->getAvailableTokens()) {
                    PKCS11CardManager *local = PKCS11CardManager::instance(pkcs11ModulePath)->getManagerForReader(token);
                    NSDate *date = [asn1 dateFromString:@(local->getValidTo().c_str())];
                    if ([date compare:NSDate.date] > 0) {
                        NSLog(@"token has valid signing certificate, adding it to selection");
                        [certificates addObject: @{
                                                        @"cert": @(BinaryUtils::bin2hex(local->getSignCert()).c_str()),
                                                        @"validTo": [df stringFromDate:date],
                                                        @"CN": @(local->getCN().c_str()),
                                                        @"type": @(local->getType().c_str()),
                                                        }];
                    }
                    delete local;
                }
            } catch (const std::runtime_error &e) {
                NSLog(@"Exception: %s", e.what());
            }
            [page dispatchMessageToScriptWithName:@"cert" userInfo:certificates[0]];
#else
            [SFSafariApplication getActiveWindowWithCompletionHandler:^(SFSafariWindow *activeWindow) {
                [activeWindow getToolbarItemWithCompletionHandler:^(SFSafariToolbarItem *toolbarItem) {
                    [toolbarItem setEnabled:YES withBadgeText:@"Cert"];
                }];
            }];
#endif
        }
        else if ([messageName isEqualToString:@"digest"]) {
            SafariExtensionViewController.sharedController.digest = userInfo;
            [SFSafariApplication getActiveWindowWithCompletionHandler:^(SFSafariWindow *activeWindow) {
                [activeWindow getToolbarItemWithCompletionHandler:^(SFSafariToolbarItem *toolbarItem) {
                    [toolbarItem setEnabled:YES withBadgeText:@"Sign"];
                }];
            }];
        }
    }];
}

- (void)toolbarItemClickedInWindow:(SFSafariWindow *)window {
    // This method will be called when your toolbar item is clicked.
    NSLog(@"The extension's toolbar item was clicked");
}

- (void)validateToolbarItemInWindow:(SFSafariWindow *)window validationHandler:(void (^)(BOOL enabled, NSString *badgeText))validationHandler {
    // This method will be called whenever some state changes in the passed in window. You should use this as a chance to enable or disable your toolbar item and set badge text.
    NSLog(@"validationHandler");
    //validationHandler(self.title != nil, self.title);
    NSDictionary *signature = SafariExtensionViewController.sharedController.signature;
    NSDictionary *cert = SafariExtensionViewController.sharedController.cert;
    if (signature != nil) {
        [SFSafariApplication getActiveWindowWithCompletionHandler:^(SFSafariWindow *activeWindow) {
            [activeWindow getActiveTabWithCompletionHandler:^(SFSafariTab *activeTab) {
                [activeTab getActivePageWithCompletionHandler:^(SFSafariPage *activePage) {
                    [activePage dispatchMessageToScriptWithName:@"signature" userInfo:signature];
                }];
            }];
        }];
    }
    else if (cert != nil) {
        [SFSafariApplication getActiveWindowWithCompletionHandler:^(SFSafariWindow *activeWindow) {
            [activeWindow getActiveTabWithCompletionHandler:^(SFSafariTab *activeTab) {
                [activeTab getActivePageWithCompletionHandler:^(SFSafariPage *activePage) {
                    [activePage dispatchMessageToScriptWithName:@"cert" userInfo:cert];
                }];
            }];
        }];
    }
}

- (SFSafariExtensionViewController *)popoverViewController {
    return [SafariExtensionViewController sharedController];
}

- (void)popoverDidCloseInWindow:(SFSafariWindow *)window {
    [SFSafariApplication setToolbarItemsNeedUpdate];
}

@end
