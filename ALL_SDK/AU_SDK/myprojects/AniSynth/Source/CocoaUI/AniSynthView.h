//
//  DXSynthView.h
//
//  Created by Will Pirkle
//  Copyright (c) 2014 Will Pirkle All rights reserved.
/*
 The Software is provided by Will Pirkle on an "AS IS" basis.
 Will Pirkle MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL Will Pirkle BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#import <Cocoa/Cocoa.h>
#import <AudioUnit/AudioUnit.h>
#import <AudioToolbox/AudioToolbox.h>

#import "WPRotaryKnob.h"
#import "WPOptionMenuGroup.h"
#import "WPTrackPadVS.h"

/************************************************************************************************************/
/* NOTE: It is important to rename ALL ui classes when using the XCode Audio Unit with Cocoa View template	*/
/*		 Cocoa has a flat namespace, and if you use the default filenames, it is possible that you will		*/
/*		 get a namespace collision with classes from the cocoa view of a previously loaded audio unit.		*/
/*		 We recommend that you use a unique prefix that includes the manufacturer name and unit name on		*/
/*		 all objective-C source files. You may use an underscore in your name, but please refrain from		*/
/*		 starting your class name with an undescore as these names are reserved for Apple.					*/
/************************************************************************************************************/


@interface AniSynthView : NSView
{
    // --- rotary knob groups
    //
    // --- rotary knob groups
    //
    // --- column 1
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_0;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_1;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_2;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_3;
    
    // --- column 2
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_4;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_5;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_6;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_7;
    
    // --- column 3
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_8;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_9;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_10;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_11; // unused
    
    // --- column 4
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_12;// unused
    IBOutlet WPOptionMenuGroupAS* wpOMG_0;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_13;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_14;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_15; // unused
    
    // --- column 5
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_16;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_17;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_18;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_19;
    
    // --- column 6
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_20;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_21;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_22; // unused
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_23; // unused
    
    // --- voice row
    IBOutlet WPOptionMenuGroupAS* wpOMG_1;
    IBOutlet WPOptionMenuGroupAS* wpOMG_2;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_24; // unused
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_25; // unused
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_26; // unused
    
    // --- globals row
    IBOutlet WPOptionMenuGroupAS* wpOMG_3;
    IBOutlet WPOptionMenuGroupAS* wpOMG_4;
    IBOutlet WPOptionMenuGroupAS* wpOMG_5;
    IBOutlet WPOptionMenuGroupAS* wpOMG_6;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_27; // unused
    
    // --- DigiSynth additions
    IBOutlet WPOptionMenuGroupAS* wpOMG_7;

    // --- VectorSynth additions
    IBOutlet WPOptionMenuGroupAS* wpOMG_8; // rotor waveform
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_28;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_29;
    IBOutlet WPRotaryKnobAS* wpRotaryKnob_30;
    
    // --- the custom NSControl for vector joystick
    IBOutlet WPTrackPadAS* vectorJoystick;

    // --- array for controls
    NSMutableArray* controlArray;
    NSMutableArray* knobImages;
    
    // --- AU members
    AudioUnit 				buddyAU; // the AU we connect to
	AUEventListenerRef		AUEventListener;
	
    // --- a background color
	NSColor*                backgroundColor;	// the background color (pattern) of the view
    NSImage* backImage;
}

- (id)getControlWithIndex:(int)index;

#pragma mark ____ PUBLIC FUNCTIONS ____
- (void)setAU:(AudioUnit)inAU;

#pragma mark ____ INTERFACE ACTIONS ____

- (IBAction)WPRotaryKnobChanged:(id)sender;
- (IBAction)WPOptionMenuItemChanged:(id)sender;
- (IBAction)WPTrackPadChanged:(id)sender;

#pragma mark ____ PRIVATE FUNCTIONS
- (void)synchronizeUIWithParameterValues;
- (void)addListeners;
- (void)removeListeners;

#pragma mark ____ LISTENER CALLBACK DISPATCHEE ____
- (void)eventListener:(void *) inObject event:(const AudioUnitEvent *)inEvent value:(Float32)inValue;
@end



