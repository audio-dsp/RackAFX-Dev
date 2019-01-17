
/*
 File: RackAFX2AUFilter.r

 Originally provided in Apple Sample AU Code 2012 as FilterDemo.r
 Modified by Will Pirkle for use with RackAFX Software 2015
 www.willpirkle.com

 */

#include <AudioUnit/AudioUnit.r>
#include "FilterVersion.h"

// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define kAudioUnitResID_Filter				1000

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Filter~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define RES_ID			kAudioUnitResID_Filter
#define COMP_TYPE		kAudioUnitType_Effect
#define COMP_SUBTYPE	'mFX1'
#define COMP_MANUF		'WILL'	

#define VERSION			kFilterVersion
#define NAME			"PurpleWave: MULTFX_AU" // NOTE this needs to be in the format "<COMP_MANUF>: <Custom Name>"
#define DESCRIPTION		"Filter AU"
#define ENTRY_POINT		"RackAFX2AUFilterEntry"

#include "AUResources.r"

