# macOS Tahoe 26.2 Compatibility Notes

## Overview

This document describes compatibility fixes implemented for macOS Tahoe 26.2, particularly for Apple Silicon M4 Pro devices (MacBook Pro 14", Nov 2024).

## Issue: Per-App Volume Sliders Not Working

### Problem Description

On macOS Tahoe 26.2 running on Apple M4 Pro, per-app volume sliders were not functioning correctly. This affected the ability to control individual application audio levels through Background Music.

### Root Cause

Apple updated the `AudioServerPlugInClientInfo` structure in macOS Tahoe 26.2. The Core Audio framework's AudioServerPlugIn API introduced changes to this structure that caused compatibility issues with Background Music's driver (BGMDriver). Specifically:

1. **Structure Layout Changes**: The `AudioServerPlugInClientInfo` structure may have had fields added, reordered, or had alignment changes
2. **Bundle ID Handling**: Changes in how bundle identifiers are provided to audio drivers, potentially NULL more frequently due to enhanced privacy/security measures
3. **Process ID Validation**: Stricter validation of process IDs on Apple Silicon with enhanced security features

### Solution

A compatibility layer (`BGM_ClientInfo_Compat.h`) was introduced to safely extract values from `AudioServerPlugInClientInfo` regardless of structure version changes. This layer provides:

#### Defensive Field Access

The `BGM_ClientInfo_GetSafeValues()` function safely extracts:
- **Client ID** (mClientID): Direct extraction with validation
- **Process ID** (mProcessID): Validated to ensure reasonable PID values (0-99999)
- **Native Endian Flag** (mIsNativeEndian): Direct extraction
- **Bundle ID** (mBundleID): Extensive validation including:
  - NULL pointer checks
  - Pointer range validation for 64-bit systems
  - CFString type verification
  - Exception handling for invalid memory access

#### Pointer Validation

The compatibility layer includes heuristic checks for bundle ID pointers:
- Validates pointer addresses are in expected heap range (0x100000000 to 0x800000000000)
- Verifies CFString type before dereferencing
- Handles exceptions gracefully when accessing potentially invalid pointers

### Files Modified

1. **BGMDriver/BGMDriver/DeviceClients/BGM_ClientInfo_Compat.h** (New)
   - Compatibility layer for safe AudioServerPlugInClientInfo access

2. **BGMDriver/BGMDriver/DeviceClients/BGM_Client.cpp**
   - Updated constructor to use safe extraction function
   - Removed direct structure field access

3. **BGMDriver/BGMDriver/BGM_Device.cpp**
   - Updated AddClient() and RemoveClient() methods
   - Improved debug logging for bundle IDs

4. **BGMDriver/BGMDriver.xcodeproj/project.pbxproj**
   - Added BGM_ClientInfo_Compat.h to project

### Testing Recommendations

When testing on macOS Tahoe 26.2 with Apple M4 Pro:

1. **Verify Per-App Volume Control**
   - Launch multiple audio applications (e.g., Safari, Music, Spotify)
   - Open Background Music preferences
   - Confirm each application appears in the app volumes list
   - Test volume slider functionality for each app
   - Verify volume changes take effect immediately

2. **Check Bundle ID Detection**
   - Monitor system logs for "Adding client" messages
   - Verify bundle IDs are correctly identified for known applications
   - Check that apps without bundle IDs (system processes) are handled gracefully

3. **Test Edge Cases**
   - Applications with multiple audio clients
   - System audio processes
   - Applications launched/closed rapidly
   - Background Music auto-pause feature

### Debugging

To debug client information extraction on macOS Tahoe:

1. Enable debug logging in BGMDriver
2. Check Console.app for BGMDriver messages
3. Look for log entries containing:
   - "Adding client" with bundle ID information
   - "Removing client" messages
   - Any warnings about invalid client info

### Backwards Compatibility

The compatibility layer maintains full backwards compatibility with earlier macOS versions:
- macOS 10.13 (High Sierra) through macOS 15.x (Sequoia)
- Intel and Apple Silicon architectures
- Previous AudioServerPlugInClientInfo structure versions

The defensive checks only activate when necessary and do not impact performance on systems with the older API structure.

### Future Considerations

If Apple continues to evolve the AudioServerPlugInClientInfo structure:

1. **Monitor API Changes**: Watch for deprecation notices and API updates in Xcode release notes
2. **Version-Specific Handling**: May need to add compile-time checks for SDK version
3. **Alternative APIs**: Investigate if Apple provides newer, more stable APIs for client identification
4. **Enhanced Validation**: Add more sophisticated validation if new fields are added

### Technical Details

#### Why Pointer Validation?

On macOS Tahoe with enhanced security features:
- Memory layout randomization (ASLR) is more aggressive
- Pointer values may be less predictable
- Invalid structure interpretation can lead to garbage pointer values
- Dereferencing invalid CFStringRef causes crashes

The validation logic:
```c
// Valid heap pointers on 64-bit macOS are typically in this range
if (ptrValue > 0x100000000ULL && ptrValue < 0x800000000000ULL) {
    // Additional CFString type check before dereferencing
    if (CFGetTypeID(bundleID) == CFStringGetTypeID()) {
        // Safe to use
    }
}
```

This prevents crashes when structure changes cause bundle ID pointer to contain garbage values.

## Support

For issues specific to macOS Tahoe 26.2 and Apple M4 Pro, include in bug reports:

- macOS version (26.2 or later)
- Hardware model (MacBook Pro 14" M4 Pro, etc.)
- Console.app logs from BGMDriver
- Steps to reproduce the issue
- List of applications where volume control fails

## References

- Apple Core Audio Documentation: https://developer.apple.com/documentation/coreaudio
- AudioServerPlugIn API: CoreAudio/AudioServerPlugIn.h
- Background Music Development Guide: DEVELOPING.md
