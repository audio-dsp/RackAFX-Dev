/***********************************************************************

	The software and files contained herein are furnished under a 
	license agreement. The software: (1) is for internal use only; 
	(2) may not be redistributed; (3) may not be reverse assembled; 
	and (4) may be used or copied only in accordance with the terms 
	of the license agreement.  

	Copyright (c) 2004 Avid Technology, Inc. All rights reserved.

************************************************************************/


#ifndef __acfextras_h__
#define __acfextras_h__

#include "acfbasetypes.h"
#include "acfresult.h"

/*! 
	\file acfextras.h
	\brief acfUID comparison operators. Utility and error checking Macros.
*/

inline bool operator == (const acfUID& lhs, const acfUID& rhs)
{
    return ((lhs.Data1 == rhs.Data1) &&
            (lhs.Data2 == rhs.Data2) &&
            (lhs.Data3 == rhs.Data3) &&
            (* (acfSInt32 *) lhs.Data4 == *(acfSInt32 *)rhs.Data4) &&
            (* (acfSInt32 *) (lhs.Data4+4) == *( (acfSInt32 *) (rhs.Data4+4)) ));
}

inline bool operator != (const acfUID& lhs,
                         const acfUID& rhs)
{
    return !(lhs == rhs);
}


// Macro to enforce relationship between the interface id and
// its associated interface type arguments in QueryInterface.
// In the following example:
// IXInterfaceType * pInterface = 0;
// pUnk->QueryInterface(IID_IXInterfaceType, (void **)&pInterface);
//
// The QueryInterface call can be rewritten to be compiler
// safe as:
// pUnk->QueryInterface(IID_PPV_ARG(IXInterfaceType, &pInterface));
// Macro from "Essential COM", by Don Box.
#define IID_PPV_ARG(Type, Expr) IID_##Type, \
	reinterpret_cast<void **>(static_cast<Type **>(Expr))

// Namespace version of IID_PPV_ARG.
#define NS_IID_PPV_ARG(NameSpace, Type, Expr) \
	NameSpace::IID_##Type, \
	reinterpret_cast<void **>(static_cast<NameSpace::Type **>(Expr))

// NOTE: We may not want the less than operator to be inline...
inline bool operator < (const acfUID& lhs, const acfUID& rhs)
{
    if (lhs.Data1 < rhs.Data1)
        return true;
    else if (lhs.Data1 == rhs.Data1) 
    {
        if (lhs.Data2 < rhs.Data2)
            return true;
        else if (lhs.Data2 == rhs.Data2)
        {
            if (lhs.Data3 < rhs.Data3)
                return true;
            else if (lhs.Data3 == rhs.Data3)
            {
                if (*(acfUInt32 *) lhs.Data4 < *(acfUInt32 *)rhs.Data4)
                    return true;
                else if (*(acfUInt32 *) lhs.Data4 == *(acfUInt32 *)rhs.Data4)
                {
                    if (*(acfUInt32 *) (lhs.Data4+4) < *((acfUInt32 *)(rhs.Data4+4)))
                        return true;
                }
            }
        }
    }

    return false;
}


// Preliminary exception handlers for ACF methods.

/*!
    \b BEGIN_ACF_METHOD
    \brief Opens a try block for ACF result codes
	\remarks BEGIN_ACF_METHOD must be followed by a  
	corresponding END_ACF_METHOD. BEGIN_ACF_METHOD is the try
	end of the exception block. These MACROS define a
	try and catch block for the handling ACFRESULT exceptions.
 */
#define BEGIN_ACF_METHOD\
    ACFRESULT avx2methodresult = ACF_OK;\
    try\
    {
/*!
    \b END_ACF_METHOD
    \brief closes a try block for ACF result codes
	\remarks END_ACF_METHOD must be preceeded by a  
	corresponding BEGIN_ACF_METHOD. END_ACF_METHOD is the catch
	end of the exception block.These MACROS define a
	try and catch block for the handling ACFRESULT exceptions.
 */
#define END_ACF_METHOD\
    }\
    catch (ACFRESULT & acfresultexception)\
    {\
        avx2methodresult = acfresultexception;\
    }\
    catch (...)\
    {\
        avx2methodresult = ACF_E_UNEXPECTED;\
    }\
    return avx2methodresult;
/*!
	\b acfcheck( result )
	\brief acfcheck is a convenience function used for validating the
	success of returning ACFRESULT codes.
	\remarks if the return code is a comprised of a
	ACFFAILED code this function will throw an ACFRESULT exception.
	For this reason, use of the function should be wrapped in a 
	try -- catch block that can handle the exception, such as the 
	BEGIN_ACF_METHOD and END_ACF_METHOD macros. 

*/
inline void acfcheck(ACFRESULT result)
{
    if (ACFFAILED(result))
		throw ACFRESULT(result);
}

#endif // __acfextras_h__
