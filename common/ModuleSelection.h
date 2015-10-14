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

@interface ModuleSelection : NSObject {
    NSPanel *driverSelectionPanel;
    NSTextField *informationLabel;
    NSTableView *moduleSelection;
    
    NSMutableArray* modules;
}
@property (assign) IBOutlet NSTableView *moduleSelection;
@property (assign) IBOutlet NSTextField *informationLabel;


@property (assign) IBOutlet NSPanel *driverSelectionPanel;
- (IBAction)OkClicked:(id)pId;
- (IBAction)BrowseClicked:(id)sender;

- (NSString *)showForWindow:(NSWindow *)window;

- (BOOL)tableView:(NSTableView*)pTableView shouldSelectRow:(int)row;

@end
