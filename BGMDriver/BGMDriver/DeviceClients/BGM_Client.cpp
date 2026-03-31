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
//  BGM_Client.cpp
//  BGMDriver
//
//  Copyright © 2016 Kyle Neideck
//

// Self Include
#include "BGM_Client.h"

// Local Includes
#include "BGM_ClientInfo_Compat.h"


BGM_Client::BGM_Client(const AudioServerPlugInClientInfo* inClientInfo)
{
    // Use safe extraction to handle potential AudioServerPlugInClientInfo structure changes
    // in macOS Tahoe 26.x and later versions, especially on Apple Silicon (M4 Pro and newer).
    UInt32 clientID = 0;
    pid_t processID = 0;
    Boolean isNativeEndian = true;
    CFStringRef bundleID = NULL;
    
    BGM_ClientInfo_GetSafeValues(inClientInfo, &clientID, &processID, &isNativeEndian, &bundleID);
    
    mClientID = clientID;
    mProcessID = processID;
    mIsNativeEndian = isNativeEndian;
    mBundleID = bundleID;
    
    // The bundle ID ref we were passed is only valid until our plugin returns control to the HAL, so we need to retain
    // it. (CACFString will handle the rest of its ownership/destruction.)
    if(bundleID != NULL)
    {
        CFRetain(bundleID);
    }
}

void    BGM_Client::Copy(const BGM_Client& inClient)
{
    mClientID = inClient.mClientID;
    mProcessID = inClient.mProcessID;
    mBundleID = inClient.mBundleID;
    mIsNativeEndian = inClient.mIsNativeEndian;
    mDoingIO = inClient.mDoingIO;
    mIsMusicPlayer = inClient.mIsMusicPlayer;
    mRelativeVolume = inClient.mRelativeVolume;
    mPanPosition = inClient.mPanPosition;
}

