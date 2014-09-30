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

#import "PINPanel.h"

#include "esteid_log.h"
#include "l10n.h"

@implementation PINPanel
@synthesize okButton, cancelButton, countdownTimer, progressBar;

- (void)dealloc {
	LOG_LOCATION;
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[super dealloc];
}

- (void) awakeFromNib {
	LOG_LOCATION;
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(pinFieldDidChange:) name:NSControlTextDidChangeNotification object:nil];
}

- (IBAction)okClicked:(id)pId 
{
	cancelled = FALSE;
	[NSApp stopModal];
}

- (IBAction)cancelClicked:(id)pId 
{
	cancelled = TRUE;
	[NSApp abortModal];
}

- (void)pinFieldDidChange:(NSNotification*)aNotification;
{
	[okButton setEnabled:([[[aNotification object] stringValue] length] >= minAcceptablePin2Length)];
}

- (void)doCountdown
{
	if (countdownTimer) {
		return;
	}
	remainingTicks = 30;
	[self updateProgressBar];
	[progressBar startAnimation: self];
	countdownTimer = [NSTimer scheduledTimerWithTimeInterval: 1.0 target: self selector: @selector(handleTimerTick) userInfo: nil repeats: YES];
}

- (void)handleTimerTick
{
	remainingTicks--;
	[self updateProgressBar];	
	if (remainingTicks <= 0) {
		[countdownTimer invalidate];
		[progressBar stopAnimation: self];
		countdownTimer = nil;
		EstEID_log("PIN pad countdown timer reached 0"); 
	}
}

- (void)updateProgressBar
{
	[progressBar setDoubleValue:(double)remainingTicks];
}

- (NSString *)showForWindow:(NSWindow *)window withName:(const char *)name withMessage:(const char *)message withMinAcceptablePin2Length:(unsigned)minPin2Length usePinPad:(int)usePinPad 
{
	EstEID_log("parameters: withName=%s, withMessage=%s, withMinAcceptablePin2Length=%u, usePinPad=%i", name, message, minPin2Length, usePinPad);		
	
	if(usePinPad) {
		[NSBundle loadNibNamed: @"PINPadMessageBox" owner: self];
		[pinFieldLabel setTitleWithMnemonic:[NSString stringWithUTF8String: l10n("For signing enter PIN2 from PIN pad")]];
		[messageField setTitleWithMnemonic: (message && message[0]) ? [NSString stringWithUTF8String: message] : @""];
		
		[self doCountdown];
		[[NSRunLoop currentRunLoop] addTimer:countdownTimer forMode:NSModalPanelRunLoopMode];
		
	}else{
		minAcceptablePin2Length = minPin2Length;
		EstEID_log("minAcceptablePin2Length set to %u", minAcceptablePin2Length);
		if (message && message[0]) {
			[NSBundle loadNibNamed: @"PINDialogWithMessage" owner: self];
			[messageField setTitleWithMnemonic: [NSString stringWithUTF8String: message]];
		}
		else {
			[NSBundle loadNibNamed: @"PINDialog" owner: self];
		}

		[pinFieldLabel setTitleWithMnemonic:[NSString stringWithUTF8String: l10n("For signing enter PIN2:")]];
		[okButton setTitle:[NSString stringWithUTF8String:l10n("Sign")]];
		[cancelButton setTitle:[NSString stringWithUTF8String:l10n("Cancel")]];
	}
	[nameLabel setTitleWithMnemonic: [NSString stringWithUTF8String: name]];
	
	[NSApp beginSheet: pinPanel modalForWindow: window modalDelegate: nil didEndSelector: nil contextInfo: nil];
	EstEID_log("starting modal sheet (%s)", usePinPad ? "pinpad": "card reader");
	[pinPanel setTitle:[NSString stringWithUTF8String:l10n("Sign")]];
	[NSApp runModalForWindow: pinPanel];
	EstEID_log("modal sheet ended");
	[NSApp endSheet: pinPanel];
	[pinPanel orderOut: self];	

	if (countdownTimer) {
		[countdownTimer invalidate];
		countdownTimer = nil;
	}

	if(usePinPad) {
		LOG_LOCATION;
		return @"";
	}
	LOG_LOCATION;
	return cancelled ? @"" : [pinField stringValue];
}

@end
