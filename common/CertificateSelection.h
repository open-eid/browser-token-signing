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

@interface CertificateSelection : NSObject {
  IBOutlet NSPanel* certificateSelectionPanel;
	IBOutlet NSTableView* certificateSelection;
	IBOutlet NSButton* okButton;
	IBOutlet NSButton* cancelButton;
  IBOutlet NSTextField* warningLabel;
	BOOL cancelled;	
	NSMutableArray* certificates;
}

@property (assign) NSPanel* certificateSelectionPanel;
@property (assign) NSTableView * certificateSelection;
@property (assign) NSButton* okButton;
@property (assign) NSButton* cancelButton;
@property (assign) NSTextField* warningLabel;

- (IBAction)okClicked:(id)pId; 

- (IBAction)cancelClicked:(id)pId;

- (IBAction)enableOkButton:(id)pId; 

- (NSString *)showForWindow:(NSWindow *)window;

- (BOOL)tableView:(NSTableView*)pTableView shouldSelectRow:(int)row;

@end
