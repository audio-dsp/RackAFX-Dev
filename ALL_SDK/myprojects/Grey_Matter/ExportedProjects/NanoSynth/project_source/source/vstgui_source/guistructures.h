//
//  guistructures.h
//  multifx_au
//
//  Created by Will Pirkle on 12/2/17.
//
//

#ifndef guistructures_h
#define guistructures_h

#ifdef AUPLUGIN
#import <CoreFoundation/CoreFoundation.h>
#import <AudioUnit/AudioUnit.h>
#import <AudioToolbox/AudioToolbox.h>
#endif

typedef struct
{
    void* pWindow;
    float width;
    float height;
#ifdef AUPLUGIN
    AudioUnit au;
#endif
} VIEW_STRUCT;

#endif /* guistructures_h */
