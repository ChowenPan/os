/*++

Copyright (c) 2016 Minoca Corp. All Rights Reserved

Module Name:

    capilib.c

Abstract:

    This module implements higher level helper functions on top of the base
    Chalk C API.

Author:

    Evan Green 20-Aug-2016

Environment:

    C

--*/

//
// ------------------------------------------------------------------- Includes
//

#include "chalkp.h"
#include <stdarg.h>

//
// ---------------------------------------------------------------- Definitions
//

//
// ------------------------------------------------------ Data Type Definitions
//

//
// ----------------------------------------------- Internal Function Prototypes
//

//
// -------------------------------------------------------------------- Globals
//

PCSTR CkApiTypeNames[CkTypeCount] = {
    "INVALID",  // CkTypeInvalid
    "null",     // CkTypeNull
    "integer",  // CkTypeInteger
    "string",   // CkTypeString
    "dict",     // CkTypeDict
    "list",     // CkTypeList
    "function", // CkTypeFunction
    "object",   // CkTypeObject
    "data",     // CkTypeData
};

//
// ------------------------------------------------------------------ Functions
//

CK_API
BOOL
CkCheckArguments (
    PCK_VM Vm,
    UINTN Count,
    ...
    )

/*++

Routine Description:

    This routine validates that the given arguments are of the correct type. If
    any of them are not, it throws a nicely formatted error.

Arguments:

    Vm - Supplies a pointer to the virtual machine.

    Count - Supplies the number of arguments coming next.

    ... - Supplies the remaining type arguments.

Return Value:

    TRUE if the given arguments match the required type.

    FALSE if an argument is not of the right type. In that case, an error
    will be created.

--*/

{

    va_list ArgumentList;
    INTN Index;
    CK_API_TYPE Type;
    BOOL Valid;

    Valid = TRUE;
    va_start(ArgumentList, Count);
    for (Index = 0; Index < Count; Index += 1) {
        Type = va_arg(ArgumentList, CK_API_TYPE);
        Valid = CkCheckArgument(Vm, Index + 1, Type);
        if (Valid == FALSE) {
            break;
        }
    }

    va_end(ArgumentList);
    return Valid;
}

CK_API
BOOL
CkCheckArgument (
    PCK_VM Vm,
    INTN StackIndex,
    CK_API_TYPE Type
    )

/*++

Routine Description:

    This routine validates that the given argument is of the correct type. If
    it is not, it throws a nicely formatted error.

Arguments:

    Vm - Supplies a pointer to the virtual machine.

    StackIndex - Supplies the stack index to check. Remember that 1 is the
        first argument index.

    Type - Supplies the type to check.

Return Value:

    TRUE if the given argument matches the required type.

    FALSE if the argument is not of the right type. In that case, an error
    will be created.

--*/

{

    PCK_CLOSURE Closure;
    CK_VALUE Error;
    PCK_FIBER Fiber;
    CK_API_TYPE FoundType;
    PCK_CALL_FRAME Frame;
    PSTR Name;

    FoundType = CkGetType(Vm, StackIndex);

    CK_ASSERT((FoundType < CkTypeCount) && (Type < CkTypeCount));

    if (FoundType == Type) {
        return TRUE;
    }

    Fiber = Vm->Fiber;

    CK_ASSERT((Fiber != NULL) && (Fiber->FrameCount != 0));

    Frame = &(Fiber->Frames[Fiber->FrameCount - 1]);
    Closure = Frame->Closure;
    Name = CkpGetFunctionName(Closure);
    Error = CkpStringFormat(Vm,
                            "%s expects %s for argument %d, got %s",
                            Name,
                            CkApiTypeNames[Type],
                            (INT)StackIndex,
                            CkApiTypeNames[FoundType]);

    Fiber->Error = Error;
    return FALSE;
}

CK_API
VOID
CkDeclareVariables (
    PCK_VM Vm,
    INTN ModuleIndex,
    PCK_VARIABLE_DESCRIPTION Variables
    )

/*++

Routine Description:

    This routine registers an array of Chalk objects in the given module.

Arguments:

    Vm - Supplies a pointer to the virtual machine.

    ModuleIndex - Supplies the stack index of the module to add the variables
        to.

    Variables - Supplies a pointer to an array of variables. The array should
        be NULL terminated.

Return Value:

    None.

--*/

{

    if (ModuleIndex < 0) {
        ModuleIndex += CkGetStackSize(Vm);

        CK_ASSERT(ModuleIndex >= 0);
    }

    while (Variables->Name != NULL) {
        switch (Variables->Type) {
        case CkTypeNull:
            CkPushNull(Vm);
            break;

        case CkTypeInteger:
            CkPushInteger(Vm, Variables->Integer);
            break;

        case CkTypeString:
            CkPushString(Vm, Variables->Value, strlen(Variables->Value));
            break;

        case CkTypeDict:
            CkPushDict(Vm);
            break;

        case CkTypeList:
            CkPushList(Vm);
            break;

        case CkTypeFunction:
            CkPushFunction(Vm,
                           Variables->Value,
                           Variables->Name,
                           Variables->Integer,
                           ModuleIndex);

            break;

        case CkTypeObject:

            //
            // Call the object specified with the given name (perhaps a class
            // name to initialize an object instance) and use the result.
            //

            CkGetVariable(Vm, ModuleIndex, Variables->Value);
            if (!CkCall(Vm, 0)) {
                CkStackPop(Vm);
                CkPushNull(Vm);
            }

            break;

        case CkTypeData:
            CkPushData(Vm, Variables->Value, NULL);
            break;

        default:

            CK_ASSERT(FALSE);

            break;
        }

        CkSetVariable(Vm, ModuleIndex, Variables->Name);
        Variables += 1;
    }

    return;
}

CK_API
VOID
CkReturnNull (
    PCK_VM Vm
    )

/*++

Routine Description:

    This routine sets null as the return value.

Arguments:

    Vm - Supplies a pointer to the virtual machine.

Return Value:

    None.

--*/

{

    CkPushNull(Vm);
    CkStackReplace(Vm, 0);
    return;
}

CK_API
VOID
CkReturnInteger (
    PCK_VM Vm,
    CK_INTEGER Integer
    )

/*++

Routine Description:

    This routine sets an integer as the return value.

Arguments:

    Vm - Supplies a pointer to the virtual machine.

    Integer - Supplies the integer to set as the foreign function return.

Return Value:

    None.

--*/

{

    CkPushInteger(Vm, Integer);
    CkStackReplace(Vm, 0);
    return;
}

CK_API
VOID
CkReturnString (
    PCK_VM Vm,
    PCSTR String,
    UINTN Length
    )

/*++

Routine Description:

    This routine creates a new string and sets it as the return value.

Arguments:

    Vm - Supplies a pointer to the virtual machine.

    String - Supplies a pointer to the buffer containing the string. A copy of
        this buffer will be made.

    Length - Supplies the length of the buffer, in bytes, not including the
        null terminator.

Return Value:

    None.

--*/

{

    CkPushString(Vm, String, Length);
    CkStackReplace(Vm, 0);
    return;
}

//
// --------------------------------------------------------- Internal Functions
//

