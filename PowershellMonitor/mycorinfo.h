// ==++==
//
//   
//    Copyright (c) 2006 Microsoft Corporation.  All rights reserved.
//   
//    The use and distribution terms for this software are contained in the file
//    named license.txt, which can be found in the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by the
//    terms of this license.
//   
//    You must not remove this notice, or any other, from this software.
//   
//
// ==--==
/*****************************************************************************\
*                                                                             *
* CorInfo.h -    EE / Code generator interface                                *
*                                                                             *
*               Version 1.0                                                   *
*******************************************************************************
*                                                                             *
*                                                                              
                                                                              *
*                                                                             *
*******************************************************************************
*
* This file exposes CLR runtime functionality. It can be used by compilers,
* both Just-in-time and ahead-of-time, to generate native code which
* executes in the runtime environment.
*******************************************************************************


*******************************************************************************
The semantic contract between the EE and the JIT should be documented here
It is incomplete, but as time goes on, that hopefully will change

-------------------------------------------------------------------------------
Class Construction

First of all class contruction comes in two flavors precise and 'beforeFieldInit'.
In C# you get the former if you declare an explicit class constructor method and
the later if you declaratively initialize static fields.  Precise class construction
guarentees that the .cctor is run precisely before the first access to any method
or field of the class.  'beforeFieldInit' semantics guarentees only that the 
.cctor will be run some time before the first static field access (note that
calling methods (static or insance) or accessing instance fields does not cause
.cctors to be run).  

Next you need to know that there are two kinds of code generation that can happen
in the JIT: appdomain neutral and appdomain specialized.  The difference 
between these two kinds of code is how statics are handled.  For appdomain
specific code, the address of a particular static variable is embeded in the
code.  This makes it usable only for one appdomain (since every appdomain gets
a own copy of its statics).  Appdomain neutral code calls a helper that looks
up static variables off of a thread local variable.  Thus the same code can be
used by mulitple appdomains in the same process.  

Generics also introduce a similar issue.  Code for generic classes might be
specialised for a particular set of type arguments, or it could use helpers
to access data that depends on type parameters and thus be shared across several
instantiations of the generic type.

Thus there four cases

    BeforeFieldInitCCtor - Unshared code.
        Cctors are only called when static fields are fetched.  At the time
        the method that touches the static field is JITed (or fixed up in the
        case of NGENed code), the .cctor is called.  

    BeforeFieldInitCCtor - Shared code.
        Since the same code is used for multiple classes, the act of JITing
        the code can not be used as a hook.  However, it is also the case that
        since the code is shared, it can not wire in a particular address for
        the static and thus needs to use a helper that looks up the correct 
        address based on the thread ID.   This helper does the .cctor check, 
        and thus no additional cctor logic is needed.

    PreciseCCtor - Unshared code.
        Any time a method is JITTed (or fixed up in the case of NGEN), a 
        cctor check for the class of the method being JITTed is done.  In 
        addition the JIT inserts explicit checks before any static field 
        accesses.  Instance methods and fields do NOT have hooks because
        a .ctor method must be called before the instance can be created.

    PreciseCctor - Shared code
        .cctor checks are placed in the prolog of every .ctor and static 
        method.  All methods that access static fields have an explicit
        .cctor check before use.   Again instance methods don't have hooks
        because a .ctor would have to be called first. 

Technically speaking, however the optimization of avoiding checks
on instance methods is flawed.  It requires that a .ctor alwasy
preceed a call to an instance methods.   This break down when

    1) A NULL is passed to an instance method.
    2) A .ctor does not call its superlcasses .ctor.  THis allows
       an instance to be created without necessarily calling all
       the .cctors of all the superclasses.  A virtual call can
       then be made to a instance of a superclass without necessarily
       calling the superclass's .cctor.
    3) The class is a value class (which exists without a .ctor 
       being called. 

Nevertheless, the cost of plugging these holes is considered to
high and the benefit is low.  

----------------------------------------------------------------------

Thus the JIT's cctor responsibilities require it to check with the EE on every
static field access using 'initClass', and also to check CORINFO_FLG_RUN_CCTOR     
before jitting any method to see if a .cctor check must be placed in the prolog.

CORINFO_FLG_NEEDS_INIT  Only classes with NEEDS_INIT need to check possibly
                    place .cctor checks before static field accesses.

CORINFO_FLG_INITIALIZED Even if the class needs initing, it might be true
                    that the class has already been initialized.  If 
                    this bit is true, again, nothing needs to be done.
                    If false, initClass needs to be called

initClass           For classes with CORINFO_FLG_NEEDS_INIT and not
                    CORINFO_FLG_INITIALIZED, initClass needs to be 
                    called to determine if a CCTOR check is needed 
                    (For class with beforeFieldInit semantics initclass 
                    may run the cctor eagerly and returns that not 
                    .cctor check is needed).  If a .cctor check
                    is required the CORINFO_HELP_INITCLASS method has
                    to be called with a class handle parameter 
                    (see embedGenericHandle)

CORINFO_FLG_RUN_CCTOR The jit is required to put CCTOR checks in the 
                    prolog of methods with this attribute.  Unfortunately
                    exactly what helper what helper is complicated

                        TODO describe

CORINFO_FLG_BEFOREFIELDINIT indicate the class has beforeFieldInit semantics.
                    The jit does not strictly need this information
                    however, it is valuable in optimizing static field
                    fetch helper calls.  Helper call for classes with
                    BeforeFieldInit semantics can be hoisted before other
                    side effects where classes with precise .cctor
                    semantics do not allow this optimization.


Inlining also complicates things.  Because the class could have precise semantics
it is also required that the inlining of any constructor or static method must
also do the CORINFO_FLG_NEEDS_INIT, CORINFO_FLG_INITIALIZED, initClass check.  
(Instance methods don't have to because constructors always come first).  In 
addition inlined functions must also check the CORINFO_FLG_RUN_CCTOR bit.  In
either case, the inliner has the option of inserting any required runtime check
or simply not inlining the function.  
                    
Finally, the JIT does has the option of skipping the CORINFO_FLG_NEEDS_INIT, 
CORINFO_FLG_INITIALIZED, initClass check in the following cases

    1) A static field access from an instance method of the same class
    2) When inlining a method from and instance method of the same class.

The rationale here is that it can be assumed that if you are in an instance 
method, the .cctor has already been called, and thus the check is not needed.
Historyically the getClassAttribs function did not have the method being 
compiled passed to it so the EE could not fold this into its .cctor logic and
thus the JIT needed to do this.  TODO: Now that this has changed, we should
have the EE do this. 

-------------------------------------------------------------------------------

Static fields

The first 4 options are mutially exclusive 

CORINFO_FLG_HELPER  If the field has this set, then the JIT must call
                    getFieldHelper and call the returned helper with
                    the object ref (for an instance field) and a fieldDesc.  
                    Note that this should be able to handle ANY field
                    so to get a JIT up quickly, it has the option of
                    using helper calls for all field access (and skip
                    the complexity below).  Note that for statics it
                    is assumed that you will alwasy ask for the 
                    ADDRESSS helper and to the fetch in the JIT. 

CORINFO_FLG_SHARED_HELPER This is currently only used for static fields.
                    If this bit is set it means that the field is
                    feched by a helper call that takes a module
                    identifier (see getModuleDomainID) and a class
                    identifier (see getClassDomainID) as arguments.
                    The exact helper to call is determined by 
                    getSharedStaticBaseHelper.  The return value is
                    of this function is the base of all statics in
                    the module. The offset from getFieldOffset must
                    be added to this value to get the address of the
                    field itself. (see also CORINFO_FLG_STATIC_IN_HEAP).


CORINFO_FLG_GENERICS_STATIC This is currently only used for static fields
                    (of generic type).  This function is intended to
                    be called with a Generic handle as a argument
                    (from embedGenericHandle).  The exact helper to
                    call is determined by getSharedStaticBaseHelper.  
                    The returned value is the base of all statics
                    in the class.  The offset from getFieldOffset must
                    be added to this value to get the address of the
                    (see also CORINFO_FLG_STATIC_IN_HEAP).

CORINFO_FLG_TLS     This indicate that the static field is a Windows
                    style Thread Local Static.  (We also have managed
                    thread local statics, which work through the HELPER.
                    Support for this is considered legacy, and going
                    forward, the EE should 

<NONE>                    This is a normal static field.   Its address in 
                    in memory is determined by getFieldAddress.
                    (see also CORINFO_FLG_STATIC_IN_HEAP).


This last field can modify any of the cases above except CORINFO_FLG_HELPER

CORINFO_FLG_STATIC_IN_HEAP This is currently only used for static fields
                    of value classes.  If the field has this set then 
                    after computing what would normally be the field, 
                    what you actually get is a object poitner (that 
                    must be reported to the GC) to a boxed version 
                    of the value.  Thus the actual field address is
                    computed by addr = (*addr+sizeof(OBJECTREF))

Instance fields

CORINFO_FLG_HELPER  This is used if the class is MarshalByRef, which
                    means that the object might be a proxyt to the
                    real object in some other appdomain or process.
                    If the field has this set, then the JIT must call
                    getFieldHelper and call the returned helper with
                    the object ref.  If the helper returned is helpers
                    that are for structures the args are as follows

CORINFO_HELP_GETFIELDSTRUCT - args are: retBuff, object, fieldDesc
CORINFO_HELP_SETFIELDSTRUCT - args are: object fieldDesc value

The other GET helpers take an object fieldDesc and return the value
        The other SET helpers take an object fieldDesc and value

    Note that unlike static fields there is no helper to take the address
    of a field because in general there is no address for proxies (LDFLDA
    is illegal on proxies). 

    CORINFO_FLG_EnC         This is to support adding new field for edit and
                            continue.  This field also indicates that a helper 
                            is needed to access this field.  However this helper
                            is always CORINFO_HELP_GETFIELDADDR, and this
                            helper always takes the object and field handle
                            and returns the address of the field.  It is the
                            JIT's responcibility to do the fetch or set. 

-------------------------------------------------------------------------------

TODO: Talk about initializing strutures before use 


*******************************************************************************
*/

#ifndef _COR_INFO_H_
#define _COR_INFO_H_

typedef ULONG32 mdToken;

// Cookie types consumed by the code generator (these are opaque values
// not inspected by the code generator):

typedef struct CORINFO_ASSEMBLY_STRUCT_*    CORINFO_ASSEMBLY_HANDLE;
typedef struct CORINFO_MODULE_STRUCT_*      CORINFO_MODULE_HANDLE;
typedef struct CORINFO_DEPENDENCY_STRUCT_*  CORINFO_DEPENDENCY_HANDLE;
typedef struct CORINFO_CLASS_STRUCT_*       CORINFO_CLASS_HANDLE;
typedef struct CORINFO_METHOD_STRUCT_*      CORINFO_METHOD_HANDLE;
typedef struct CORINFO_FIELD_STRUCT_*       CORINFO_FIELD_HANDLE;
typedef struct CORINFO_ARG_LIST_STRUCT_*    CORINFO_ARG_LIST_HANDLE;    // represents a list of argument types
typedef struct CORINFO_SIG_STRUCT_*         CORINFO_SIG_HANDLE;         // represents the whole list
typedef struct CORINFO_JUST_MY_CODE_HANDLE_*CORINFO_JUST_MY_CODE_HANDLE;
typedef struct CORINFO_PROFILING_STRUCT_*   CORINFO_PROFILING_HANDLE;   // a handle guaranteed to be unique per process
typedef DWORD*                              CORINFO_SHAREDMODULEID_HANDLE;
typedef struct CORINFO_GENERIC_STRUCT_*     CORINFO_GENERIC_HANDLE;     // a generic handle (could be any of the above)

// what is actually passed on the varargs call
typedef struct CORINFO_VarArgInfo *         CORINFO_VARARGS_HANDLE;

// Generic tokens are resolved with respect to a context, which is usually the method
// being compiled. The CORINFO_CONTEXT_HANDLE indicates which exact instantiation
// (or the open instantiation) is being referred to.
// CORINFO_CONTEXT_HANDLE is more tightly scoped than CORINFO_MODULE_HANDLE. For cases 
// where the exact instantiation does not matter, CORINFO_MODULE_HANDLE is used.
typedef CORINFO_METHOD_HANDLE               CORINFO_CONTEXT_HANDLE;

// Bit-twiddling of contexts assumes word-alignment of method handles and type handles
// If this ever changes, some other encoding will be needed
enum CorInfoContextFlags
{
    CORINFO_CONTEXTFLAGS_METHOD = 0x00, // CORINFO_CONTEXT_HANDLE is really a CORINFO_METHOD_HANDLE
    CORINFO_CONTEXTFLAGS_CLASS  = 0x01, // CORINFO_CONTEXT_HANDLE is really a CORINFO_CLASS_HANDLE
    CORINFO_CONTEXTFLAGS_MASK   = 0x01
};

#define MAKE_CLASSCONTEXT(c)  (CORINFO_CONTEXT_HANDLE((size_t) (c) | CORINFO_CONTEXTFLAGS_CLASS))
#define MAKE_METHODCONTEXT(m) (CORINFO_CONTEXT_HANDLE((size_t) (m) | CORINFO_CONTEXTFLAGS_METHOD))

enum CorInfoSigInfoFlags
{
    CORINFO_SIGFLAG_IS_LOCAL_SIG = 0x01,
    CORINFO_SIGFLAG_IL_STUB      = 0x02,
};

enum CorInfoCallConv
{
	// These correspond to CorCallingConvention

	CORINFO_CALLCONV_DEFAULT = 0x0,
	CORINFO_CALLCONV_C = 0x1,
	CORINFO_CALLCONV_STDCALL = 0x2,
	CORINFO_CALLCONV_THISCALL = 0x3,
	CORINFO_CALLCONV_FASTCALL = 0x4,
	CORINFO_CALLCONV_VARARG = 0x5,
	CORINFO_CALLCONV_FIELD = 0x6,
	CORINFO_CALLCONV_LOCAL_SIG = 0x7,
	CORINFO_CALLCONV_PROPERTY = 0x8,
	CORINFO_CALLCONV_NATIVEVARARG = 0xb,    // used ONLY for IL stub PInvoke vararg calls

	CORINFO_CALLCONV_MASK = 0x0f,     // Calling convention is bottom 4 bits
	CORINFO_CALLCONV_GENERIC = 0x10,
	CORINFO_CALLCONV_HASTHIS = 0x20,
	CORINFO_CALLCONV_EXPLICITTHIS = 0x40,
	CORINFO_CALLCONV_PARAMTYPE = 0x80,     // Passed last. Same as CORINFO_GENERICS_CTXT_FROM_PARAMTYPEARG
};

enum CorInfoType
{
	CORINFO_TYPE_UNDEF = 0x0,
	CORINFO_TYPE_VOID = 0x1,
	CORINFO_TYPE_BOOL = 0x2,
	CORINFO_TYPE_CHAR = 0x3,
	CORINFO_TYPE_BYTE = 0x4,
	CORINFO_TYPE_UBYTE = 0x5,
	CORINFO_TYPE_SHORT = 0x6,
	CORINFO_TYPE_USHORT = 0x7,
	CORINFO_TYPE_INT = 0x8,
	CORINFO_TYPE_UINT = 0x9,
	CORINFO_TYPE_LONG = 0xa,
	CORINFO_TYPE_ULONG = 0xb,
	CORINFO_TYPE_NATIVEINT = 0xc,
	CORINFO_TYPE_NATIVEUINT = 0xd,
	CORINFO_TYPE_FLOAT = 0xe,
	CORINFO_TYPE_DOUBLE = 0xf,
	CORINFO_TYPE_STRING = 0x10,         // Not used, should remove
	CORINFO_TYPE_PTR = 0x11,
	CORINFO_TYPE_BYREF = 0x12,
	CORINFO_TYPE_VALUECLASS = 0x13,
	CORINFO_TYPE_CLASS = 0x14,
	CORINFO_TYPE_REFANY = 0x15,

	// CORINFO_TYPE_VAR is for a generic type variable.
	// Generic type variables only appear when the JIT is doing
	// verification (not NOT compilation) of generic code
	// for the EE, in which case we're running
	// the JIT in "import only" mode.

	CORINFO_TYPE_VAR = 0x16,
	CORINFO_TYPE_COUNT,                         // number of jit types
};

enum CorInfoOptions
{
	CORINFO_OPT_INIT_LOCALS = 0x00000010, // zero initialize all variables

	CORINFO_GENERICS_CTXT_FROM_THIS = 0x00000020, // is this shared generic code that access the generic context from the this pointer?  If so, then if the method has SEH then the 'this' pointer must always be reported and kept alive.
	CORINFO_GENERICS_CTXT_FROM_PARAMTYPEARG = 0x00000040, // is this shared generic code that access the generic context from the ParamTypeArg?  If so, then if the method has SEH then the 'ParamTypeArg' must always be reported and kept alive. Same as CORINFO_CALLCONV_PARAMTYPE
	CORINFO_GENERICS_CTXT_MASK = (CORINFO_GENERICS_CTXT_FROM_THIS |
	CORINFO_GENERICS_CTXT_FROM_PARAMTYPEARG),
	CORINFO_GENERICS_CTXT_KEEP_ALIVE = 0x00000080, // Keep the generics context alive throughout the method even if there is no explicit use, and report its location to the CLR

};

struct CORINFO_SIG_INST
{
    unsigned                classInstCount;
    CORINFO_CLASS_HANDLE *  classInst; // (representative, not exact) instantiation for class type variables in signature
    unsigned                methInstCount;
    CORINFO_CLASS_HANDLE *  methInst; // (representative, not exact) instantiation for method type variables in signature
};

struct CORINFO_SIG_INFO
{
    CorInfoCallConv         callConv;
    CORINFO_CLASS_HANDLE    retTypeClass;   // if the return type is a value class, this is its handle (enums are normalized)
    CORINFO_CLASS_HANDLE    retTypeSigClass;// returns the value class as it is in the sig (enums are not converted to primitives)
    CorInfoType             retType : 8;
    unsigned                flags   : 8;    // used by IL stubs code
    unsigned                numArgs : 16;
    struct CORINFO_SIG_INST sigInst;  // information about how type variables are being instantiated in generic code
    CORINFO_ARG_LIST_HANDLE args;
    CORINFO_SIG_HANDLE      sig;
    CORINFO_MODULE_HANDLE   scope;          // passed to getArgClass
    mdToken                 token;

    bool                hasRetBuffArg()     { return retType == CORINFO_TYPE_VALUECLASS || retType == CORINFO_TYPE_REFANY; }
    CorInfoCallConv     getCallConv()       { return CorInfoCallConv((callConv & CORINFO_CALLCONV_MASK)); }
    bool                hasThis()           { return ((callConv & CORINFO_CALLCONV_HASTHIS) != 0); }
    unsigned            totalILArgs()       { return (numArgs + hasThis()); }
    unsigned            totalNativeArgs()   { return (totalILArgs() + hasRetBuffArg()); }
    bool                isVarArg()          { return ((getCallConv() == CORINFO_CALLCONV_VARARG) || (getCallConv() == CORINFO_CALLCONV_NATIVEVARARG)); }
    bool                hasTypeArg()        { return ((callConv & CORINFO_CALLCONV_PARAMTYPE) != 0); }
};

struct CORINFO_METHOD_INFO
{
    CORINFO_METHOD_HANDLE       ftn;
    CORINFO_MODULE_HANDLE       scope;
    BYTE *                      ILCode;
    unsigned                    ILCodeSize;
    unsigned short              maxStack;
    unsigned short              EHcount;
    CorInfoOptions              options;
    CORINFO_SIG_INFO            args;
    CORINFO_SIG_INFO            locals;
};

#endif // _COR_INFO_H_
