/*
  File: FilterVersion.h
  Copyright (C) 2012 Apple Inc. All Rights Reserved.
 
  Modified by Will Pirkle for use with RackAFX Software 2015
  www.willpirkle.com
*/

#ifndef __FilterVersion_h__
#define __FilterVersion_h__


#ifdef DEBUG
	#define kFilterVersion 0xFFFFFFFF
#else
	#define kFilterVersion 0x00010000	
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define Filter_COMP_SUBTYPE		'FLIT'
#define Filter_COMP_MANF		'WILL'
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#endif

