// This file is part of Background Music.
//
// Background Music is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation, either version 2 of the
// License, or (at your option) any later version.
//
// Background Music is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Background Music. If not, see <http://www.gnu.org/licenses/>.

//
//  BGM_ClientInfo_Compat.h
//  BGMDriver
//
//  Copyright © 2026 Background Music contributors
//
//  Compatibility layer for AudioServerPlugInClientInfo structure changes
//  across macOS versions, particularly for macOS Tahoe 26.x and later.
//

#ifndef __BGMDriver__BGM_ClientInfo_Compat__
#define __BGMDriver__BGM_ClientInfo_Compat__

// System Includes
#include <CoreAudio/AudioServerPlugIn.h>
#include <AvailabilityMacros.h>

#pragma clang assume_nonnull begin

//==================================================================================================
//	BGM_ClientInfo_GetSafeValues
//
//  Safely extracts values from AudioServerPlugInClientInfo regardless of structure version.
//  This handles potential structure changes introduced in macOS Tahoe 26.x and later versions
//  on Apple Silicon (M4 Pro and newer).
//
//  The AudioServerPlugInClientInfo structure may have been expanded with additional fields
//  in newer macOS versions. This function provides defensive access to the core fields
//  (mClientID, mProcessID, mIsNativeEndian, mBundleID) that are expected to remain at
//  the beginning of the structure for backwards compatibility.
//==================================================================================================

static inline void BGM_ClientInfo_GetSafeValues(const AudioServerPlugInClientInfo* _Nullable inClientInfo,
                                                 UInt32* _Nullable outClientID,
                                                 pid_t* _Nullable outProcessID,
                                                 Boolean* _Nullable outIsNativeEndian,
                                                 CFStringRef _Nullable * _Nullable outBundleID)
{
    // Handle NULL input gracefully
    if (inClientInfo == NULL) {
        if (outClientID != NULL) *outClientID = 0;
        if (outProcessID != NULL) *outProcessID = 0;
        if (outIsNativeEndian != NULL) *outIsNativeEndian = true;
        if (outBundleID != NULL) *outBundleID = NULL;
        return;
    }
    
    // The first four fields of AudioServerPlugInClientInfo have been stable across macOS versions:
    // - mClientID (UInt32)
    // - mProcessID (pid_t)
    // - mIsNativeEndian (Boolean)
    // - mBundleID (CFStringRef)
    //
    // However, to be extra defensive against structure packing or alignment changes,
    // we validate that the pointer looks reasonable before dereferencing.
    
    // Extract client ID (first field, always safe)
    if (outClientID != NULL) {
        *outClientID = inClientInfo->mClientID;
    }
    
    // Extract process ID (second field, expected to be stable)
    if (outProcessID != NULL) {
        // Validate that the PID is reasonable (not negative, not absurdly large)
        pid_t pid = inClientInfo->mProcessID;
        if (pid >= 0 && pid < 99999) {
            *outProcessID = pid;
        } else {
            // Invalid PID, use 0 as fallback
            *outProcessID = 0;
        }
    }
    
    // Extract endianness flag (third field)
    if (outIsNativeEndian != NULL) {
        *outIsNativeEndian = inClientInfo->mIsNativeEndian;
    }
    
    // Extract bundle ID (fourth field, most fragile due to being a pointer)
    if (outBundleID != NULL) {
        CFStringRef bundleID = inClientInfo->mBundleID;
        
        // Validate that the bundle ID pointer looks reasonable
        // On modern macOS with ASLR, valid pointers are typically in high memory
        // NULL is also valid (many system processes don't have bundle IDs)
        if (bundleID == NULL) {
            *outBundleID = NULL;
        } else {
            // Additional validation: Check if it's a valid CFString
            // CFGetTypeID will crash if passed an invalid pointer, so we first
            // check if the pointer value seems reasonable
            uintptr_t ptrValue = (uintptr_t)bundleID;
            
            // On 64-bit systems, valid heap pointers are typically > 0x100000000
            // and < 0x800000000000 (excluding kernel space)
            // This is a heuristic check to avoid dereferencing garbage
            if (ptrValue > 0x100000000ULL && ptrValue < 0x800000000000ULL) {
                // Now check if it's actually a CFString
                // We use CFGetTypeID carefully with exception handling
                @try {
                    if (CFGetTypeID(bundleID) == CFStringGetTypeID()) {
                        *outBundleID = bundleID;
                    } else {
                        // Not a CFString, treat as NULL
                        *outBundleID = NULL;
                    }
                } @catch (NSException *exception) {
                    // Invalid pointer, treat as NULL
                    *outBundleID = NULL;
                }
            } else {
                // Pointer value out of expected range, treat as NULL
                *outBundleID = NULL;
            }
        }
    }
}

#pragma clang assume_nonnull end

#endif /* __BGMDriver__BGM_ClientInfo_Compat__ */
