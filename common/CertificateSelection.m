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

#import "CertificateSelection.h"

#import "esteid_log.h"
#import "esteid_map.h"
#import "esteid_certinfo.h"
#import "l10n.h"

@implementation CertificateSelection

@synthesize certificateSelectionPanel, certificateSelection, okButton, cancelButton, warningLabel;

- (void)dealloc
{
	[certificateSelection release];
	[certificateSelectionPanel release];
	[certificates release];
	[okButton release];
	[cancelButton release];
	[super dealloc];
}

- (void)awakeFromNib
{
	LOG_LOCATION;
	[certificateSelection setTarget:self];
	[certificateSelection setDoubleAction:@selector(okClicked:)];
	[certificateSelection reloadData];

	NSTableColumn* cn = [certificateSelection tableColumnWithIdentifier:@"CN"];
	NSTableHeaderCell* header = [cn headerCell];
	[header setStringValue: [NSString stringWithUTF8String: l10n("Certificate")]];
	cn = [certificateSelection tableColumnWithIdentifier:@"type"];
	header = [cn headerCell];
	[header setStringValue: [NSString stringWithUTF8String: l10n("Type")]];
	cn = [certificateSelection tableColumnWithIdentifier:@"validTo"];
	header = [cn headerCell];
	[header setStringValue: [NSString stringWithUTF8String: l10n("Valid to")]];
	
	[certificateSelectionPanel setTitle:[NSString stringWithUTF8String:l10n("Select certificate")]];
	
	[okButton setTitle:[NSString stringWithUTF8String:l10n("Select")]];
	[cancelButton setTitle:[NSString stringWithUTF8String:l10n("Cancel")]];
  
	[warningLabel setTitleWithMnemonic:[NSString stringWithUTF8String: l10n("By selecting a certificate I accept that my name and personal ID code will be sent to service provider.")]];
  
	if ([certificateSelection numberOfRows] > 0) {
		EstEID_log("Selecting first row in certificates list");
		[certificateSelection selectRow:0 byExtendingSelection:FALSE];
		[okButton setEnabled: YES];
	}
}

- (id)init 
{
	
	if ((self = [super init])) {
		EstEID_Certs *certs = EstEID_loadCerts();
		EstEID_log("%i certificates found", certs->count);
		certificates = [NSMutableArray new];
		
		for(int i = 0; i < certs->count; i++) {
			NSMutableDictionary* certificateData = [NSMutableDictionary new];
			EstEID_Map cert = certs->certs[i];
			if (!EstEID_mapGet(cert, "usageNonRepudiation")) continue;
			
			[certificateData setObject:[NSString  stringWithCString:EstEID_mapGet(cert, "commonName") encoding:NSUTF8StringEncoding] forKey:@"CN"];
			NSString* validTo = [NSString  stringWithCString:EstEID_mapGet(cert, "validTo") encoding:NSUTF8StringEncoding];
			NSArray* list = [validTo componentsSeparatedByString:@" "];
			[certificateData setObject:[list objectAtIndex:0] forKey:@"validTo"];
			[certificateData setObject:[NSString stringWithCString:EstEID_mapGet(cert, "certHash") encoding:NSUTF8StringEncoding] forKey:@"id"];
			const char *org = EstEID_mapGet(cert, "organizationName");
			if (org)
				[certificateData setObject:[NSString stringWithCString:org encoding:NSUTF8StringEncoding] forKey:@"type"];
			[certificates addObject:certificateData];
			[certificateData release];
		}
	}
	
	return self;
}

- (int)numberOfRowsInTableView:(NSTableView*)pTableView
{
	return [certificates count];
}

- (id)tableView:(NSTableView*)pTableView objectValueForTableColumn:(NSTableColumn*)pTableColumn row:(int)pRowIndex
{
	NSString* data = [[NSString alloc] autorelease];
	NSDictionary* current = [NSDictionary dictionaryWithDictionary:[certificates objectAtIndex:pRowIndex]];
	if ([[pTableColumn identifier] isEqualToString:@"CN"]) {
		data = [current objectForKey:@"CN"];
	}
	if ([[pTableColumn identifier] isEqualToString:@"type"]) {
		data = [current objectForKey:@"type"];
	}
	if ([[pTableColumn identifier] isEqualToString:@"validTo"]) {
		data = [current objectForKey:@"validTo"];
	}	
	return data;
}

- (BOOL)tableView:(NSTableView*)pTableView shouldSelectRow:(int)row
{
	EstEID_log("Selected row %i, enabled?: %i, button: %p", row, [okButton isEnabled], okButton);
	[okButton setEnabled: YES];
	EstEID_log("enabled?: %i", [okButton isEnabled]);
	return YES;
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

- (IBAction)enableOkButton:(id)pId
{
	EstEID_log("button: %p", okButton);
	[okButton setEnabled:([certificateSelection selectedRow] != -1)];
}

- (NSString *)showForWindow:(NSWindow *)window
{
	LOG_LOCATION;
	[NSBundle loadNibNamed: @"CertificateSelection" owner: self];
    [NSApp beginSheet: certificateSelectionPanel modalForWindow: window modalDelegate: nil didEndSelector: nil contextInfo: nil];
	[NSApp runModalForWindow: certificateSelectionPanel];
	int selectedCertificateIndex = [certificateSelection selectedRow];
	[NSApp endSheet: certificateSelectionPanel];
	[certificateSelectionPanel orderOut: self];	
	
	BOOL isCancelledOrNotSelected = (cancelled || (selectedCertificateIndex == -1));
	EstEID_log("Certificate selection returns with index %i, cert id is %s",
			   selectedCertificateIndex,
			   isCancelledOrNotSelected ? "N/A" : [[[certificates objectAtIndex:selectedCertificateIndex] objectForKey:@"id"] UTF8String]);
	
	return (isCancelledOrNotSelected) ? @"" : [[certificates objectAtIndex:selectedCertificateIndex] objectForKey:@"id"];
}

@end
