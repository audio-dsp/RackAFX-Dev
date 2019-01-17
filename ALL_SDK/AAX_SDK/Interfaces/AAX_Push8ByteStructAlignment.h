/*================================================================================================*/
/*
 *
 *	Copyright 2014-2015 by Avid Technology, Inc.
 *	All rights reserved.
 *	
 *	CONFIDENTIAL: This document contains confidential information.  Do not 
 *	read or examine this document unless you are an Avid Technology employee
 *	or have signed a non-disclosure agreement with Avid Technology which protects
 *	the	confidentiality of this document.  DO NOT DISCLOSE ANY INFORMATION 
 *	CONTAINED IN THIS DOCUMENT TO ANY THIRD-PARTY WITHOUT THE PRIOR WRITTEN	CONSENT 
 *	OF Avid Technology, INC.
 *
 */

/**  
 *	\file   AAX_Push8ByteStructAlignment.h
 *	
 *	\brief Set the struct alignment to 8-byte
 *
 *	\details
 *	When setting the alignment for a struct in order to match a particular environment (e.g.
 *	host/plug-in binary compatibility) the following macros are recommended:
 *	\li \ref AAX_ALIGN_FILE_HOST
 *	\li \ref AAX_ALIGN_FILE_ALG
 *	\li \ref AAX_ALIGN_FILE_RESET
 *
 *	\section AAX_Push8ByteStructAlignment_usagenotes Usage notes
 *
 *		\li Always follow an inclusion of this file with a matching
 *			inclusion of AAX_PopStructAlignment.h
 *	
 *		\li Do not place other file \c \#include after this file.  For example:
 *	\code
 *	// HeaderFile1.h
 *		#include AAX_Push8ByteStructAlignment.h
 *		#include HeaderFile2.h	// this file now has 8-byte alignment also!!
 *		// HeaderFile1.h definitions...
 *		#include AAX_PopStructAlignment.h
 *	// end HeaderFile1.h
 *	\endcode
 *				This will cause problems if HeaderFile2.h is included elsewhere without the 8-byte 
 *				alignment which will manifest as hard to find run-time bugs. The proper usage is:
 *	\code
 *	// HeaderFile1.h
 *		#include HeaderFile2.h
 *		#include AAX_Push8ByteStructAlignment.h
 *			// HeaderFile1.h definitions...
 *		#include AAX_PopStructAlignment.h
 *	// end HeaderFile1.h
 *	\endcode
 *		
 *	\sa \ref AAX_Push2ByteStructAlignment.h
 *	\sa \ref AAX_Push4ByteStructAlignment.h
 *	\sa \ref AAX_PopStructAlignment.h
 *			
 *	\internal
 *		NOTE: we don't use include guards for this file because it *is* possible to 
 *		include this file multiple times in the same file.
 *	\endinternal
 *
 *
 */ 
/*================================================================================================*/


#ifdef _TMS320C6X
	// TI is OK - 8 byte alignment is the only allowed alignment
#elif defined (_MSC_VER)
	#pragma warning( disable : 4103 ) // used #pragma pack to change alignment
	#pragma pack(push, 8)
#elif defined (__GNUC__)
// Uncomment this warning suppression if you really want to apply packing to a virtual data
// structure, but note that there is no guarantee of cross-platform compatibility for such
// a structure. For more information, see the AAX_ALIGN_FILE_ALG macro documentation
//	#ifdef __clang__
//		#pragma clang diagnostic ignored "-Wno-incompatible-ms-struct"
//	#endif
	#pragma ms_struct on
//	#ifdef __clang__
//		#pragma clang diagnostic pop
//	#endif
	#pragma pack(push, 8)
#elif defined (__MWERKS__)
	#pragma options align=mac68k
#elif defined (__IAR_SYSTEMS_ICC)
  // Do nothing for IAR Systems Embedded Workbench (Sync I/O F/W)
#else
	#error "You need to supply a pragma here to set structure alignment to 8 bytes"
#endif

// Nesting of struct alignment headers is not allowed
#ifdef __AAX_CUSTOM_STRUCT_ALIGN_IS_SET__
	#error "Nested AAX struct alignment directives"
#else
	#define __AAX_CUSTOM_STRUCT_ALIGN_IS_SET__
#endif
