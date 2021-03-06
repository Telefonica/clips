//
//  CLIPSFactController.h
//  CLIPS
//
//  Created by Gary Riley on 3/10/06.
//

#import <Cocoa/Cocoa.h>

@class CLIPSEnvironment;
@class ModuleArrayController;

@interface CLIPSFactController : NSWindowController 
  {
   IBOutlet NSTableView *moduleList;
   IBOutlet NSTableView *factList;
   IBOutlet NSTableView *slotValues;
   IBOutlet NSSplitView *splitView;
   IBOutlet NSPopUpButton *environmentList;
   IBOutlet NSButton *displayDefaultedValuesButton;
   IBOutlet NSProgressIndicator *executionIndicator;
   IBOutlet NSArrayController *moduleListController;
   IBOutlet ModuleArrayController *factListController;
   IBOutlet NSArrayController *slotValuesController;
   IBOutlet NSSearchField *searchField;
   NSPredicate *slotFilter;
   CLIPSEnvironment *environment;
   int fontSize;
   int rowHeight;
   NSString *lastModule;
   NSInteger lastModuleRow;
   NSInteger lastFact;
   NSInteger lastFactRow;
  }

/*%%%%%%%%%%%%%%%%*/
/* Action Methods */
/*%%%%%%%%%%%%%%%%*/
  
- (IBAction)                     displayDefaultedValues: (id) sender;
- (IBAction)                     search: (id) sender;

/*%%%%%%%%%%%%%%%%%%%%%%*/
/* Notification Methods */
/*%%%%%%%%%%%%%%%%%%%%%%*/

- (void)                         targetEnvironmentDeallocated: (NSNotification *) note;

/*%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/* Key-Value Coding Methods */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%*/

- (void)                         setEnvironment: (CLIPSEnvironment *) theEnvironment;
- (CLIPSEnvironment *)           environment;

- (void)                         setSlotFilter: (NSPredicate *) theSlotFilter;
- (NSPredicate *)                slotFilter;

@end
