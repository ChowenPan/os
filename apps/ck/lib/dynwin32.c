/*++

Copyright (c) 2016 Minoca Corp. All Rights Reserved

Module Name:

    dynwin32.c

Abstract:

    This module implements support for dynamic libraries on Windows platforms.

Author:

    Evan Green 14-Aug-2016

Environment:

    Win32

--*/

//
// ------------------------------------------------------------------- Includes
//

#include <windows.h>

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

//
// Define the shared library extension.
//

PCSTR CkSharedLibraryExtension = ".dll";

//
// ------------------------------------------------------------------ Functions
//

PVOID
CkpLoadLibrary (
    PSTR BinaryName
    )

/*++

Routine Description:

    This routine loads a shared library.

Arguments:

    BinaryName - Supplies the name of the binary to load.

Return Value:

    Returns a handle to the library on success.

    NULL on failure.

--*/

{

    HMODULE NewHandle;

    //
    // Attempt to load the library. Bail on failure.
    //

    NewHandle = LoadLibrary(BinaryName);
    return NewHandle;
}

VOID
CkpFreeLibrary (
    PVOID Handle
    )

/*++

Routine Description:

    This routine unloads a shared library.

Arguments:

    Handle - Supplies the handle to to the loaded library.

Return Value:

    None.

--*/

{

    if (Handle == NULL) {
        return;
    }

    FreeLibrary(Handle);
    return;
}

PVOID
CkpGetLibrarySymbol (
    PVOID Handle,
    PSTR SymbolName
    )

/*++

Routine Description:

    This routine gets the address of a named symbol in a loaded shared library.

Arguments:

    Handle - Supplies the handle to to the loaded library.

    SymbolName - Supplies the name of the symbol to look up.

Return Value:

    Returns a pointer to the symbol (usually a function) on success.

    NULL on failure.

--*/

{

    return GetProcAddress(Handle, SymbolName);
}

//
// --------------------------------------------------------- Internal Functions
//

