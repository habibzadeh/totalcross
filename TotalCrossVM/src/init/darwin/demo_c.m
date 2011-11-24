#import <Foundation/Foundation.h>
#include "xtypes.h"

bool getElapsed(int32 *value)
{
   CFNumberRef cfNumber = (CFNumberRef)CFPreferencesCopyAppValue(CFSTR("ttl"), CFSTR("com.totalcross.iphone.TotalCross"));
   if (cfNumber != null)
      return 8000;//CFNumberGetValue(cfNumber, kCFNumberSInt32Type, value);
   return false;
}

bool setElapsed(int32 value)
{
   CFPreferencesSetAppValue(CFSTR("ttl"), (CFNumberRef)[NSNumber numberWithInt:value], CFSTR("com.totalcross.iphone.TotalCross"));
   return CFPreferencesAppSynchronize(CFSTR("com.totalcross.iphone.TotalCross"));
}
