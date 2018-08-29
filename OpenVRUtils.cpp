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

#include "OpenVRUtils.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <fstream>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

    void export_hidden_area_outline_as_csv(std::ofstream& csv_stream,
                                           const vr::HiddenAreaMesh_t& mesh,
                                           vr::IVRSystem* const system)
    {
        vr::DistortionCoordinates_t xy;

        for (size_t i = 0; i < mesh.unTriangleCount; ++i) {
            csv_stream << mesh.pVertexData[i].v[0] << "\t";
            csv_stream << mesh.pVertexData[i].v[1] << "\t";

            if (not system->ComputeDistortion(vr::Eye_Left, mesh.pVertexData[i].v[0], mesh.pVertexData[i].v[1], &xy)) {
                xy = {};
            }

            csv_stream << std::max(-1.0f, std::min(xy.rfRed[0]  , 2.0f)) << "\t";
            csv_stream << std::max(-1.0f, std::min(xy.rfRed[1]  , 2.0f)) << "\t";
            csv_stream << std::max(-1.0f, std::min(xy.rfGreen[0], 2.0f)) << "\t";
            csv_stream << std::max(-1.0f, std::min(xy.rfGreen[1], 2.0f)) << "\t";
            csv_stream << std::max(-1.0f, std::min(xy.rfBlue[0] , 2.0f)) << "\t";
            csv_stream << std::max(-1.0f, std::min(xy.rfBlue[1] , 2.0f)) << "\n";
        }

        csv_stream << mesh.pVertexData[0].v[0] << "\t";
        csv_stream << mesh.pVertexData[0].v[1] << "\n";
    }

    void export_distortion_samples_as_csv(std::ofstream& csv_stream, size_t size, vr::EVREye eye, vr::IVRSystem *system)
    {
        vr::DistortionCoordinates_t xy;

        for (size_t y = 0; y < size; ++y) {
            const float v = (float(y) / float(size - 1));

            for (size_t x = 0; x < size; ++x) {
                const float u = (float(x) / float(size - 1));

                csv_stream << u << "\t";
                csv_stream << v << "\t";

                if (not system->ComputeDistortion(eye, u, v, &xy)) {
                    xy = {};
                }

                csv_stream << std::max(-1.0f, std::min(xy.rfRed[0]  , 2.0f)) << "\t";
                csv_stream << std::max(-1.0f, std::min(xy.rfRed[1]  , 2.0f)) << "\t";
                csv_stream << std::max(-1.0f, std::min(xy.rfGreen[0], 2.0f)) << "\t";
                csv_stream << std::max(-1.0f, std::min(xy.rfGreen[1], 2.0f)) << "\t";
                csv_stream << std::max(-1.0f, std::min(xy.rfBlue[0] , 2.0f)) << "\t";
                csv_stream << std::max(-1.0f, std::min(xy.rfBlue[1] , 2.0f)) << "\n";
            }
        }
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char*
OpenVRUtils::compositor_error_as_english_description(vr::VRCompositorError error)
{
    switch (error) {
        case vr::VRCompositorError_None:                         return "None (0)";
        case vr::VRCompositorError_RequestFailed:                return "RequestFailed (1)";
        case vr::VRCompositorError_IncompatibleVersion:          return "IncompatibleVersion (100)";
        case vr::VRCompositorError_DoNotHaveFocus:               return "DoNotHaveFocus (101)";
        case vr::VRCompositorError_InvalidTexture:               return "InvalidTexture (102)";
        case vr::VRCompositorError_IsNotSceneApplication:        return "IsNotSceneApplication (103)";
        case vr::VRCompositorError_TextureIsOnWrongDevice:       return "TextureIsOnWrongDevice (104)";
        case vr::VRCompositorError_TextureUsesUnsupportedFormat: return "TextureUsesUnsupportedFormat (105)";
        case vr::VRCompositorError_SharedTexturesNotSupported:   return "SharedTexturesNotSupported (106)";
        case vr::VRCompositorError_IndexOutOfRange:              return "IndexOutOfRange (107)";
        case vr::VRCompositorError_AlreadySubmitted:             return "AlreadySubmitted (108)";
        case vr::VRCompositorError_InvalidBounds:                return "InvalidBounds (109)";
        default:                                                 return "Unkown VRCompositorError";
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string
OpenVRUtils::get_tracked_device_string(vr::IVRSystem* const system,
                                       vr::TrackedDeviceIndex_t device_index,
                                       vr::TrackedDeviceProperty property,
                                       vr::TrackedPropertyError* const error)
{
    assert(system);

    if (not system) {
        return std::string();
    }

    const uint32_t length = system->GetStringTrackedDeviceProperty(device_index, property, nullptr, 0, error);

    if (length == 0) {
        return std::string();
    }

    std::string result;
    result.resize((length - 1));        // 'length' includes terminator

    if (system->GetStringTrackedDeviceProperty(device_index, property, &result[0], length, error) != length) {
        assert(false);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
OpenVRUtils::export_hidden_area_outline_as_csv(const char* const path, bool overwrite, vr::IVRSystem* const system)
{
    std::ofstream csv_stream(path, (std::ios::out | (overwrite ? std::ios::trunc : std::ios::app)));

    if (not csv_stream.is_open()) {
        return;
    }

    if (not overwrite && (csv_stream.tellp() != 0)) {
        return;
    }

    csv_stream << "Left Eye\n";

    const vr::HiddenAreaMesh_t mesh_left = system->GetHiddenAreaMesh(vr::Eye_Left , vr::k_eHiddenAreaMesh_LineLoop);
    ::export_hidden_area_outline_as_csv(csv_stream, mesh_left, system);

    csv_stream << "Right Eye\n";

    const vr::HiddenAreaMesh_t mesh_right = system->GetHiddenAreaMesh(vr::Eye_Right, vr::k_eHiddenAreaMesh_LineLoop);
    ::export_hidden_area_outline_as_csv(csv_stream, mesh_right, system);
}

void
OpenVRUtils::export_distortion_samples_as_csv(const char* const path, bool overwrite, vr::IVRSystem* const system)
{
    constexpr size_t SIZE = (16 + 1);       // 16 x 16 quads

    std::ofstream csv_stream(path, (std::ios::out | (overwrite ? std::ios::trunc : std::ios::app)));

    if (not csv_stream.is_open()) {
        return;
    }

    if (not overwrite && (csv_stream.tellp() != 0)) {
        return;
    }

    csv_stream << "Left Eye\n";
    ::export_distortion_samples_as_csv(csv_stream, SIZE, vr::Eye_Left, system);

    csv_stream << "Right Eye\n";
    ::export_distortion_samples_as_csv(csv_stream, SIZE, vr::Eye_Right, system);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
