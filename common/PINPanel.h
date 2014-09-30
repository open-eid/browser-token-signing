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

#import <Cocoa/Cocoa.h>

@interface PINPanel : NSObject {

    IBOutlet NSPanel* pinPanel;
    IBOutlet NSSecureTextField* pinField;
	IBOutlet NSTextField* messageField;
	IBOutlet NSTextField* pinFieldLabel;
	IBOutlet NSTextField* nameLabel;
	IBOutlet NSButton* okButton;
	IBOutlet NSButton* cancelButton;
	IBOutlet NSProgressIndicator* progressBar;
	BOOL cancelled;

	@private
	NSTimer* countdownTimer;
	NSUInteger remainingTicks;	
	unsigned minAcceptablePin2Length;
}

@property (assign) NSButton* okButton;
@property (assign) NSButton* cancelButton;
@property (assign) NSProgressIndicator* progressBar;
@property (nonatomic, retain) NSTimer* countdownTimer;

- (IBAction)okClicked:(id)pId;

- (IBAction)cancelClicked:(id)pId;

- (void)pinFieldDidChange:(NSNotification*)aNotification;

- (NSString *)showForWindow:(NSWindow *)window withName:(const char *)name withMessage:(const char *)message withMinAcceptablePin2Length:(unsigned)minPin2Length usePinPad:(int)usePinPad;

- (void)doCountdown;

- (void)handleTimerTick;

- (void)updateProgressBar;

@end
