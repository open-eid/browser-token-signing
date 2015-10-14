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

#import "ModuleSelection.h"
#import "esteid_log.h"
#import "preferences.h"

@implementation ModuleSelection
@synthesize moduleSelection;
@synthesize informationLabel;
@synthesize driverSelectionPanel;

- (void)dealloc
{
    [moduleSelection release];
    [driverSelectionPanel release];
    [modules release];
    [super dealloc];
}

- (void)awakeFromNib
{
    LOG_LOCATION
    [moduleSelection setTarget:self];
    [moduleSelection setDoubleAction:@selector(okClicked:)];
    [moduleSelection reloadData];
    LOG_LOCATION
    NSTableColumn* cn = [moduleSelection tableColumnWithIdentifier:@"country"];
    NSTableHeaderCell* header = [cn headerCell];
    LOG_LOCATION
    [header setStringValue: @"Country"];
    cn = [moduleSelection tableColumnWithIdentifier:@"location"];
    LOG_LOCATION
    header = [cn headerCell];
    [header setStringValue: @"Location"];
    LOG_LOCATION
    
    [driverSelectionPanel setTitle:@"Select Driver"];
    
    LOG_LOCATION
    [informationLabel setTitleWithMnemonic: @"We found more than one driver implementation, please select which one you would like to use"];
    LOG_LOCATION
    if ([moduleSelection numberOfRows] > 0) {
        EstEID_log("Selecting first row in module list");
        [moduleSelection selectRow:0 byExtendingSelection:FALSE];
    }
    LOG_LOCATION
}

- (id)init
{
    
    if ((self = [super init])) {
        LOG_LOCATION
        EstEID_Map availableModules = getAvailablePKCS11Modules();
        int modulesCount = EstEID_mapSize(availableModules);
        EstEID_log("%i modules found", modulesCount);
        modules = [NSMutableArray new];
        
        for(int i = 0; i < modulesCount; i++) {
            EstEID_log(availableModules->key);
            
            
            NSMutableDictionary* moduleData = [NSMutableDictionary new];
            
            [moduleData setObject:[NSString stringWithUTF8String:availableModules->key] forKey:@"country"];
            
            const char *location = EstEID_mapGet(availableModules, availableModules->key);
            [moduleData setObject:[NSString stringWithCString:location encoding:NSUTF8StringEncoding] forKey:@"location"];
            
            [modules addObject:moduleData];
            [moduleData release];
            availableModules = availableModules->next;
            
        }
    }
    
    return self;
}

- (IBAction)OkClicked:(id)pId {
    [NSApp stopModal];
}

- (IBAction)BrowseClicked:(id)sender {
}

- (int)numberOfRowsInTableView:(NSTableView*)pTableView
{
    return [modules count];
}

- (id)tableView:(NSTableView*)pTableView objectValueForTableColumn:(NSTableColumn*)pTableColumn row:(int)pRowIndex
{
    NSString* data = [[NSString alloc] autorelease];
    NSDictionary* current = [NSDictionary dictionaryWithDictionary:[modules objectAtIndex:pRowIndex]];
    if ([[pTableColumn identifier] isEqualToString:@"country"]) {
        data = [current objectForKey:@"country"];
    }
    if ([[pTableColumn identifier] isEqualToString:@"location"]) {
        data = [current objectForKey:@"location"];
    }
    return data;
}

- (BOOL)tableView:(NSTableView*)pTableView shouldSelectRow:(int)row
{
    EstEID_log("Selected row %i", row);
    return YES;
}

- (NSString *)showForWindow:(NSWindow *)window
{
    LOG_LOCATION;
    [informationLabel setTitleWithMnemonic: @"We found more than one driver implementation, please select which one you would like to use"];
    [NSBundle loadNibNamed: @"ModuleSelection" owner: self];
    
    [NSApp beginSheet: driverSelectionPanel modalForWindow: window modalDelegate: nil didEndSelector: nil contextInfo: nil];
    [NSApp runModalForWindow: driverSelectionPanel];
    int selectedModuleIndex = [moduleSelection selectedRow];
    [NSApp endSheet: driverSelectionPanel];
    [driverSelectionPanel orderOut: self];
    
    
    return [[modules objectAtIndex:selectedModuleIndex] objectForKey:@"location"];}


@end
