//
// MIT License
//
// Copyright (c) 2018 Chris Birkhold
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __OPENVR_UTILS_H__
#define __OPENVR_UTILS_H__

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <OpenVR/OpenVR.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Utility class for working with OpenVR.
//------------------------------------------------------------------------------

class OpenVRUtils
{
public:

    //------------------------------------------------------------------------------
    // This would ideally be part of OpenVR.
    static const char* compositor_error_as_english_description(vr::VRCompositorError error);

    //------------------------------------------------------------------------------
    // Retrieve a tracked device property string as an std::string.
    static std::string get_tracked_device_string(vr::IVRSystem* const system,
                                                 vr::TrackedDeviceIndex_t device_index,
                                                 vr::TrackedDeviceProperty property,
                                                 vr::TrackedPropertyError* const error = nullptr);

public:

    //------------------------------------------------------------------------------
    // Export outlines of the hidden area meshes to a CSV file (X-Y scatter chart).
    static void export_hidden_area_outline_as_csv(const char* const path, bool overwrite, vr::IVRSystem* const system);

    //------------------------------------------------------------------------------
    // Export lens-distorted grids to a CSV file (X-Y scatter chart).
    static void export_distortion_samples_as_csv(const char* const path, bool overwrite, vr::IVRSystem* const system);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // __OPENVR_UTILS_H__

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
