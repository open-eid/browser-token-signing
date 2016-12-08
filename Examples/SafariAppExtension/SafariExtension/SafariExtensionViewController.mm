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

#import "SafariExtensionViewController.h"
#import <AppKit/AppKit.h>

#if 1
#import "BinaryUtils.h"
#import "PKCS11CardManager.h"
#import "PKCS11Path.h"
#endif

@interface SafariExtensionViewController () <NSTableViewDataSource,NSTableViewDelegate>
@property (weak) IBOutlet NSTableView *certificateSelection;
@property (weak) IBOutlet NSButton *cancel;
@property (weak) IBOutlet NSButton *select;
@property (weak) IBOutlet NSSecureTextField *pin;
@property (strong) NSMutableArray *certificates;

@end

@implementation SafariExtensionViewController

- (void)viewDidLoad {
    [super viewDidLoad];
#if 1
    self.certificateSelection.dataSource = self;
    self.certificateSelection.delegate = self;
    [[self.certificateSelection tableColumnWithIdentifier:@"CN"].headerCell setStringValue:@"certificate"];
    [[self.certificateSelection tableColumnWithIdentifier:@"type"].headerCell setStringValue:@"type"];
    [[self.certificateSelection tableColumnWithIdentifier:@"validTo"].headerCell setStringValue:@"valid to"];
    [self.certificateSelection setDoubleAction:@selector(okClicked:)];

    try {
        self.certificates = [[NSMutableArray alloc] init];
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
                [self.certificates addObject: @{
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

    if (self.certificateSelection.numberOfRows > 0) {
        [self.certificateSelection selectRowIndexes:[NSIndexSet indexSetWithIndex:0] byExtendingSelection:FALSE];
        self.select.enabled = YES;
    }
#endif
}

+ (SafariExtensionViewController *)sharedController {
    static SafariExtensionViewController *sharedController = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedController = [[SafariExtensionViewController alloc] init];
    });
    return sharedController;
}

- (IBAction)okClicked:(id)sender {
    if (self.pin.stringValue.length > 0) {
        std::string pkcs11ModulePath(PKCS11Path::getPkcs11ModulePath());
        std::unique_ptr<PKCS11CardManager> selected;
        try {
            for (auto &token : PKCS11CardManager::instance(pkcs11ModulePath)->getAvailableTokens()) {
                selected.reset(PKCS11CardManager::instance(pkcs11ModulePath)->getManagerForReader(token));
                if (BinaryUtils::hex2bin([self.cert[@"cert"] UTF8String]) == selected->getSignCert()) {
                    break;
                }
                selected.reset();
            }
        }
        catch(const std::runtime_error &e) {
            return NSLog(@"PKCS11 exception %s", e.what());
        }

        if (!selected) {
            return NSLog(@"invalid_argument");
        }

        std::vector<unsigned char> hash = BinaryUtils::hex2bin([self.digest[@"digest"] UTF8String]);
        switch (hash.size()) {
            case BINARY_SHA1_LENGTH:
            case BINARY_SHA224_LENGTH:
            case BINARY_SHA256_LENGTH:
            case BINARY_SHA384_LENGTH:
            case BINARY_SHA512_LENGTH: break;
            default: break;
        }
        
        self.signature = @{
            @"cert": self.cert,
            @"signature": @(BinaryUtils::bin2hex(selected->sign(hash, self.pin.stringValue.UTF8String)).c_str()),
            @"algo": self.digest[@"algo"]
        };
    }
    else if (self.certificateSelection.selectedRow != -1) {
        self.cert = self.certificates[self.certificateSelection.selectedRow];
    }
    //[self.view.window close];
}

#pragma mark - NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView*)tableView {
    return self.certificates.count;
}

- (id)tableView:(NSTableView*)tableView objectValueForTableColumn:(NSTableColumn*)tableColumn row:(NSInteger)row {
    if (self.certificates.count == 0) {
        return [NSString string];
    }
    if ([[tableColumn identifier] isEqualToString:@"CN"]) {
        return self.certificates[row][@"CN"];
    }
    if ([[tableColumn identifier] isEqualToString:@"type"]) {
        return self.certificates[row][@"type"];
    }
    if ([[tableColumn identifier] isEqualToString:@"validTo"]) {
        return self.certificates[row][@"validTo"];
    }
    return [NSString string];
}

#pragma mark - NSTableViewDelegate

- (BOOL)tableView:(NSTableView*)tableView shouldSelectRow:(NSInteger)row {
    return self.select.enabled = YES;
}

@end
