/*********************************************************************************
 *  TotalCross Virtual Machine, version 1                                        *
 *  Copyright (C) 2007-2012 SuperWaba Ltda.                                      *
 *  All Rights Reserved                                                          *
 *********************************************************************************/


#define Object NSObject*
#include "mainview.h"
#include "gfx_ex.h"

#import <QuartzCore/CALayer.h>
#define LKLayer CALayer

void privateScreenChange(int32 w, int32 h);
bool isFullScreen();
bool allowMainThread();

static bool allowOrientationChanges = false;
static NSLock *deviceCtxLock;
int statusbar_height;
int keyboardH,realAppH;


void lockDeviceCtx(const char *info)
{
   DEBUG1("lock DeviceCtx: '%s'\n", info);
   if (!deviceCtxLock)
      deviceCtxLock = [[NSRecursiveLock alloc] init];
   [ deviceCtxLock lock ];
   DEBUG0("DeviceCtx locked\n");
}

void unlockDeviceCtx()
{
   [ deviceCtxLock unlock ];
   DEBUG0("DeviceCtx unlocked\n");
}

void _debug(const char *format, ...)
{
   char buffer[1024];
   va_list va;
   va_start(va, format);
   vsprintf(buffer, format, va);
   va_end(va);

   bool dont_close = false;
   FILE *lout = fopen( [NSThread isMainThread] ? "/tmp/MAIN.out" : "/tmp/TC.out", "a+");
   if (!lout)
   {
      lout = stdout;
      dont_close = true;
   }
   fprintf(lout, [NSThread isMainThread] ? "MAIN [%08x]: " : "tc   [%08x]: ", (unsigned int)[NSThread currentThread]);
   fprintf(lout, buffer);
   if (buffer[strlen(buffer)-1] != '\n')
     fprintf(lout, "\n");

   if (!dont_close)
      fclose(lout);
}

@implementation SSize

- (id)set:(CGSize)size
{
   ssize = size;
   return self;
}

- (CGSize)get
{
   return ssize;
}

@end

//--------------------------------------------------------------------------------------------------------

@implementation MainView

- (double)durationForTransition:(int)type
{
   return 1.0f;
}

- (id)initWithFrame:(CGRect)rect
{
   child_view = nil;
   child_added = true;
   current_orientation = UIDeviceOrientationPortrait; // initial orientation

   _events = nil;
   _lock = [[NSLock alloc] init];

   DEBUG4("initWithFrame: %dx%d,%dx%d\n",
         (int)rect.origin.x, (int)rect.origin.y, (int)rect.size.width, (int)rect.size.height);   
   
   self = [ super initWithFrame: rect ];
   [ self geometryChanged ];
   realAppH = rect.size.height;
   
   //flsobral@tc126: register didRotate to receive orientation change notifications.
   [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
   
   [[NSNotificationCenter defaultCenter] addObserver:self
      selector:@selector (keyboardDidShow:)
      name: UIKeyboardDidShowNotification object:nil];
 
   [[NSNotificationCenter defaultCenter] addObserver:self 
      selector:@selector (keyboardDidHide:)
      name: UIKeyboardDidHideNotification object:nil];
         
   [[NSNotificationCenter defaultCenter] addObserver:self
      selector:@selector(didRotate:)
      name:UIDeviceOrientationDidChangeNotification object:nil];

   [ self screenChange: true ]; //needed to make the keyboard appear without first rotating the screen on iOS 5, don't know why.

   return self;
}

- (void)geometryChanged
{
   [ self lock: "mainview:geometryChanged" ];
   DEBUG0("MainView geometryChanged\n");

   /* start orientation */
   DEBUG1("init rotate: %d\n", current_orientation);

   old_view = child_view;

   CGRect rect = [ self frame ];
   DEBUG4("NEWCHILD: %dx%d,%dx%d\n", (int)rect.origin.x, (int)rect.origin.y, (int)rect.size.width, (int)rect.size.height);

   float max_dim = rect.size.width > rect.size.height ? rect.size.width  : rect.size.height;
   float min_dim = rect.size.width > rect.size.height ? rect.size.height : rect.size.width;
   DEBUG2("max_dim=%f min_dim=%f\n", max_dim, min_dim);
   
   struct CGAffineTransform transEnd;

   if (current_orientation == UIDeviceOrientationLandscapeLeft || current_orientation == UIDeviceOrientationLandscapeRight)
   {
      float diff = max_dim - min_dim;
      rect = CGRectMake(-diff/2, diff/2, max_dim, min_dim);
      child_view = [ [ ChildView alloc ] initWithFrame: rect orientation:current_orientation ];

      transEnd = (current_orientation == UIDeviceOrientationLandscapeLeft)
               ? CGAffineTransformMake(0,  1, -1, 0, 0, 0)
               : CGAffineTransformMake(0, -1,  1, 0, 0, 0);

     [ child_view setTransform:transEnd];
   }
   else
   {
      rect = CGRectMake(0, 0, min_dim, max_dim);
      child_view = [ [ ChildView alloc ] initWithFrame: rect orientation:current_orientation ];

      transEnd = (current_orientation == UIDeviceOrientationPortraitUpsideDown)
               ? CGAffineTransformMake(-1,  0,  0, -1, 0, 0)
               : CGAffineTransformMake( 1,  0,  0,  1, 0, 0);

     [ child_view setTransform:transEnd];
   }

   DEVICE_CTX->_childview = child_view;

   [ self addSubview: child_view ];

   if (old_view != nil)
      [ self sendSubviewToBack: old_view ];
   [ self bringSubviewToFront: child_view ];

   if (old_view != nil)
      [ old_view removeFromSuperview ];

   [ self unlock ];
}

- (bool)isKbdShown
{
   return (kbd_view != nil);
}

- (int)orientation
{
   return current_orientation;
}

- (void)destroySIP
{
   DEBUG1("********************************************************** destroySIP: kbd_view=%x\n", kbd_view);
   [ self lock: "mainview:destroySIP" ];
   [ self bringSubviewToFront: child_view ];

   if (kbd_view != null)
   {
      DEBUG0("really release kbd_view\n");
      [ kbd_view removeFromSuperview ];
      kbd_view = nil;
   }
   [ self unlock ];   
}

- (void)showSIP:(SipArguments*)args
{
   int options = [ args values].options;

   DEBUG1("********************************************************** showSIP option=%d\n", options);

   if (options == SIP_HIDE)
     [ self destroySIP ];
   else
   {
      [ self lock: "mainview:showSIP" ];
      if (kbd_view != nil)
      {
         kbd_view.hidden = YES;
         [ kbd_view removeFromSuperview ];
      }

      CGRect rect = [ self frame ];
      kbd_view = [ [ KeyboardView alloc ] initWithFrame: CGRectMake(0, 0, rect.size.width, rect.size.height) params: args ];
      if (kbd_view != null)
      {
        [ self addSubview: kbd_view ];
        [ self bringSubviewToFront: kbd_view ];
      }
      [ self unlock ];
   }
   DEBUG0("showSIP DONE\n");
}

static bool verbose_lock;

- (void)lock:(const char *)info;
{
   verbose_lock = (info != NULL);
   if (verbose_lock)
      DEBUG1("claim MainView lock for '%s'\n", info);
   [_lock lock];
   if (verbose_lock)
      DEBUG0("got MainView lock\n");
}

- (void)unlock
{
   [_lock unlock];
   if (verbose_lock)
      DEBUG0("MainView lock released\n");
}

- (bool)isEventAvailable;
{
   [self lock: NULL ];
   unsigned int num = [_events count];
   [self unlock];
   return num > 0;
}

- (NSArray*)getEvents
{
   [self lock: NULL ];
   NSArray* events = _events;
   _events = nil;
   [self unlock];

   return events;
}

- (void)addEvent:(NSDictionary*)event
{
   [self lock: "addEvent"];

   if(_events == nil)
      _events = [[NSMutableArray alloc] init];

   [_events addObject: event];

   [self unlock];
}

- (void)setFullscreen:(bool)mode
{
   full_screen = mode;
   [ self screenChange: true ];
}

- (bool)isFullscreen
{
   return full_screen;
}

- (void)screenChange: (bool)force
{
   if ( [ self isKbdShown ]) return;
   DEBUG1("main screenChange: force=%d\n", force);
   int orientation = [[UIDevice currentDevice] orientation];

   if (child_view != nil && !force)
   {
      if (orientation == UIDeviceOrientationUnknown || orientation == UIDeviceOrientationFaceUp || orientation == UIDeviceOrientationFaceDown)
         return; // keep previous

      if (orientation == current_orientation)
         return; // don't change
   }

   int width, height;
   CGRect rect = [ self frame ];
   if (orientation == UIDeviceOrientationLandscapeLeft || orientation == UIDeviceOrientationLandscapeRight)
   {
      height = rect.size.width;
      width = rect.size.height;
   }
   else
   {
      width = rect.size.width;
      height = rect.size.height;
   }
   realAppH = height;
   current_orientation = orientation;

   lockDeviceCtx("screenChange");
   if (DEVICE_CTX && DEVICE_CTX->_childview)
   {
      [ DEVICE_CTX->_childview screenChange: width height:height ];
   }
   unlockDeviceCtx();

}

- (void)scheduleScreenChange: (CGSize)size
{
   if (allowMainThread())
   {
      // must be an object, cannot be a struct
      SSize *s = [[ SSize alloc ] set: size ];
      [ self performSelectorOnMainThread:@selector(doScreenChange:) withObject:s waitUntilDone: YES ];
   }
}

- (void)doScreenChange: (SSize*)size
{
   privateScreenChange([size get].width, [size get].height);
}

- (void)didRotate:(NSNotification *)notification
{
   [self screenChange: NO];
}


-(void) keyboardDidShow: (NSNotification *)notif
{
   if (keyboardH != 0) 
      return;

   // Get the size of the keyboard.
   NSDictionary* info = [notif userInfo];
   NSValue* aValue = [info objectForKey:UIKeyboardBoundsUserInfoKey];
   CGSize keyboardSize = [aValue CGRectValue].size;
   keyboardH = keyboardSize.height;
}

-(void) keyboardDidHide: (NSNotification *)notif
{
   keyboardH = 0;
   [ self addEvent:
      [[NSDictionary alloc] initWithObjectsAndKeys:
       @"sipClosed", @"type",
       [NSNumber numberWithInt:0], @"x",
       [NSNumber numberWithInt:0], @"y",
       nil
      ]
   ];
}

//--------------------------------------------------------------------------------------------------------

@end

void orientationChanged() // called by the UI
{
   if (allowOrientationChanges && DEVICE_CTX && DEVICE_CTX->_mainview)
   {
      DEBUG0("orientationChanged() call screenChange\n");
      [DEVICE_CTX->_mainview screenChange: false];
   }
}

void privateFullscreen(bool on)
{
   if (DEVICE_CTX && DEVICE_CTX->_mainview)
      [ DEVICE_CTX->_mainview setFullscreen: on ];
}

void privateScreenChange(int32 w, int32 h)
{
   DEBUG0("privateScreenChange\n");
   if (![NSThread isMainThread])
   {
      [DEVICE_CTX->_mainview scheduleScreenChange: CGSizeMake(w, h)];
      return;
   }

   float bar_orientation = 0.0f;

   lockDeviceCtx("privateScreenChange");

   float bar_size = statusbar_height;
   int current_orientation = [DEVICE_CTX->_mainview orientation];
   DEBUG2("orientation: %d bar_size=%f\n", current_orientation, bar_size);

   MainView *main_view = DEVICE_CTX->_mainview;
   bool fullscreen = (main_view != nil) ? [ main_view isFullscreen ] : false;
   if (fullscreen)
   {
      bar_size = 0.0f; //hide the status bar
   }
   else if (current_orientation == UIDeviceOrientationLandscapeLeft)
      bar_orientation = 90;
   else if (current_orientation == UIDeviceOrientationLandscapeRight)
      bar_orientation = -90;
   
   [[UIApplication sharedApplication] setStatusBarHidden: (bar_size > 0) ? false:true ];
   [[UIApplication sharedApplication] setStatusBarOrientation: current_orientation animated: true];

   CGRect rect = [[UIScreen mainScreen] applicationFrame];
   DEBUG4("SCREEN: %dx%d,%dx%d\n",
            (int)rect.origin.x, (int)rect.origin.y, (int)rect.size.width, (int)rect.size.height);

   if (!fullscreen)
   {
      if (current_orientation == UIDeviceOrientationLandscapeLeft)
      {
         rect.origin.x -= statusbar_height;
         rect.origin.y = 0;
      }
      else if (current_orientation == UIDeviceOrientationLandscapeRight)
         rect.origin.y = 0;
   }

   DEBUG4("WINDOW: %dx%d,%dx%d\n",
            (int)rect.origin.x, (int)rect.origin.y, (int)rect.size.width, (int)rect.size.height);

   UIWindow *window = DEVICE_CTX->_window;
   if (window == nil)
      DEVICE_CTX->_window = window = [ [ UIWindow alloc ] initWithFrame: rect ];
   else
      [ window setFrame: rect ];

   CGRect viewRect = CGRectMake(0, 0, rect.size.width, rect.size.height);
   DEBUG4("MAINVIEW: %dx%d,%dx%d\n",
            (int)0, (int)0, (int)rect.size.width, (int)rect.size.height);

   if (main_view == nil)
   {
      DEVICE_CTX->_mainview = main_view = [ [ MainView alloc ] initWithFrame: viewRect];
      DEBUG0("new MainView\n");
      [ window addSubview: main_view ];
      [ window makeKeyAndVisible ];
   }
   else
   {
      [ main_view geometryChanged ];
   }

   unlockDeviceCtx();
}

bool graphicsStartup(ScreenSurface screen, int16 appTczAttr)
{
   lockDeviceCtx("graphicsStartup");

   deviceCtx = screen->extension = (TScreenSurfaceEx*)malloc(sizeof(TScreenSurfaceEx));
   memset(screen->extension, 0, sizeof(TScreenSurfaceEx));

   CGRect statusBarRect = [[UIApplication sharedApplication] statusBarFrame];
   statusbar_height = statusBarRect.size.height;
   
   /************************ START privateScreenChange *********************/   
   DEBUG0(">> STARTUP\n");
   if (![NSThread isMainThread])
   {
      [DEVICE_CTX->_mainview scheduleScreenChange: CGSizeMake(0,0)];
      return false;
   }

   float bar_orientation = 0.0f;

   lockDeviceCtx("startup-privateScreenChange");

   float bar_size = statusbar_height;
   int current_orientation = [DEVICE_CTX->_mainview orientation];
   DEBUG2("orientation: %d bar_size=%f\n", current_orientation, bar_size);

   MainView *main_view = DEVICE_CTX->_mainview;
   bool fullscreen = (main_view != nil) ? [ main_view isFullscreen ] : false;
   if (fullscreen)
   {
      bar_size = 0.0f; //hide the status bar
   }
   else if (current_orientation == UIDeviceOrientationLandscapeLeft)
      bar_orientation = 90;
   else if (current_orientation == UIDeviceOrientationLandscapeRight)
      bar_orientation = -90;
   
   [[UIApplication sharedApplication] setStatusBarHidden: (bar_size > 0) ? false:true ];
   [[UIApplication sharedApplication] setStatusBarOrientation: current_orientation animated: true];
   
   CGRect rect = [[UIScreen mainScreen] bounds];
   
   if (!fullscreen)
   {
      switch (current_orientation)
      {
         case UIDeviceOrientationUnknown:
         case UIDeviceOrientationPortrait:
         case UIDeviceOrientationFaceUp:
         case UIDeviceOrientationFaceDown:
         {
            rect.origin.y += statusbar_height;
            rect.size.height -= statusbar_height;
         } break;         
      }
   }
   
   UIWindow *window = DEVICE_CTX->_window;
   if (window == nil)
      DEVICE_CTX->_window = window = [ [ UIWindow alloc ] initWithFrame: rect ];
   else
      [ window setFrame: rect ];
   
   CGRect viewRect = CGRectMake(0, 0, rect.size.width, rect.size.height);
   DEBUG4(">> MAINVIEW: %dx%d,%dx%d\n",
            (int)0, (int)0, (int)rect.size.width, (int)rect.size.height);

   if (main_view == nil)
   {
      DEVICE_CTX->_mainview = main_view = [ [ MainView alloc ] initWithFrame: viewRect];
      DEBUG0(">> new MainView\n");
      [ window addSubview: main_view ];
      [ window makeKeyAndVisible ];
   }
   else
   {
      [ main_view geometryChanged ];
   }

   unlockDeviceCtx();
   /************************ END privateScreenChange *********************/
   
   DEBUG0("graphicsStartup done\n");

   [ DEVICE_CTX->_childview updateScreen: screen ];

   screen->pixels = (void*)1;

   unlockDeviceCtx();

   return true;
}

bool graphicsCreateScreenSurface(ScreenSurface screen)
{
   lockDeviceCtx("graphicsCreateScreenSurface");
   screen->extension = deviceCtx;
   unlockDeviceCtx();
   return true;
}

void graphicsUpdateScreen(ScreenSurface screen, int32 transitionEffect)
{
   lockDeviceCtx("graphicsUpdateScreen");
   ChildView* vw = (ChildView*)SCREEN_EX(screen)->_childview;
   if (allowMainThread())
      [vw invalidateScreen: screen];
   allowOrientationChanges = true;
   unlockDeviceCtx();       
}

void graphicsDestroy(ScreenSurface screen, bool isScreenChange)
{
   lockDeviceCtx("graphicsDestroy");
   if (isScreenChange)
   {
     screen->extension = NULL;
   }
   else
   {
      if (screen->extension)
        free(screen->extension);
     deviceCtx = screen->extension = NULL;
   }
   unlockDeviceCtx();
}

bool graphicsLock(ScreenSurface screen, bool on)
{
   DEBUG2("graphicsLock begin screen=%x %d\n", screen,(int)on);
   if (on)
      lockDeviceCtx("graphicsLock");
   else
      unlockDeviceCtx();
   return true;
}