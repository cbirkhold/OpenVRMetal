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

#include "OpenVRMetal.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MetalUtils.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

METAL_OPENVR_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HiddenAreaMesh::HiddenAreaMesh(id<MTLDevice> device, EHiddenAreaMeshType type, const HiddenAreaMesh_t& mesh)
{
    //------------------------------------------------------------------------------
    // Evaluate layout.
    switch (type) {
        case k_eHiddenAreaMesh_Standard:
        case k_eHiddenAreaMesh_Inverse:
            m_primitive_type = MTLPrimitiveTypeTriangle;
            m_num_vertices = (3 * mesh.unTriangleCount);
            break;

        case k_eHiddenAreaMesh_LineLoop:
            m_primitive_type = MTLPrimitiveTypeLineStrip;
            m_num_vertices = (mesh.unTriangleCount + 1);
            break;

        default:
            throw std::runtime_error("Invalid hidden area mesh type!");
            break;
    }

    const NSUInteger buffer_length = (sizeof(simd_float2) * m_num_vertices);

    //------------------------------------------------------------------------------
    // Fill the buffer with the vertex data.
    const MTLResourceOptions options = (MTLResourceCPUCacheModeWriteCombined | MTLResourceStorageModeManaged);
    m_vertex_buffer = [device newBufferWithLength:buffer_length options:options];

    simd_float2* const vertices = reinterpret_cast<simd_float2*>(m_vertex_buffer.contents);

    switch (type) {
        case k_eHiddenAreaMesh_Standard:
        case k_eHiddenAreaMesh_Inverse:
            for (size_t vertex_index = 0; vertex_index < m_num_vertices; ++vertex_index) {
                vertices[vertex_index] = simd_make_float2(mesh.pVertexData[vertex_index].v[0],
                                                          mesh.pVertexData[vertex_index].v[1]);
            }
            break;

        case k_eHiddenAreaMesh_LineLoop:
            for (size_t vertex_index = 0; vertex_index < (m_num_vertices - 1); ++vertex_index) {
                vertices[vertex_index] = simd_make_float2(mesh.pVertexData[vertex_index].v[0],
                                                          mesh.pVertexData[vertex_index].v[1]);
            }

            vertices[m_num_vertices - 1] = simd_make_float2(mesh.pVertexData[0].v[0],
                                                            mesh.pVertexData[0].v[1]);
            break;

        default:
            assert(false);
            break;
    }

    [m_vertex_buffer didModifyRange:NSMakeRange(0, m_vertex_buffer.length)];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
HiddenAreaMesh::move_to_private_storage(id encoder, bool wait_until_completed)
{
    m_vertex_buffer = copy_to_private_storage(m_vertex_buffer, encoder, wait_until_completed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
HiddenAreaMesh::add_to_vertex_descriptor(MTLVertexDescriptor* const vertex_desciptor,
                                         NSUInteger buffer_index,
                                         NSUInteger position_attribute_index)
{
    vertex_desciptor.layouts[buffer_index].stride = sizeof(simd_float2);
    vertex_desciptor.layouts[buffer_index].stepRate = 1;
    vertex_desciptor.layouts[buffer_index].stepFunction = MTLVertexStepFunctionPerVertex;

    vertex_desciptor.attributes[position_attribute_index].format = MTLVertexFormatFloat2;
    vertex_desciptor.attributes[position_attribute_index].offset = 0;
    vertex_desciptor.attributes[position_attribute_index].bufferIndex = buffer_index;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HiddenAreaMesh_t
HiddenAreaMesh::create_rectangular_mesh(float coverage)
{
    const size_t num_vertices = (3 * 8);
    const float coverage_2 = (coverage / 2.0f);
    const simd_float4 p = simd_make_float4(coverage_2, coverage_2, (1.0f - coverage_2), (1.0f - coverage_2));

    HmdVector2_t* const vertices = new HmdVector2_t[num_vertices];
    {
        size_t vertex_index = 0;

        vertices[vertex_index++] = HmdVector2_t { .v = { 0.0, 0.0 } };
        vertices[vertex_index++] = HmdVector2_t { .v = { p.x, p.y } };
        vertices[vertex_index++] = HmdVector2_t { .v = { 1.0, 0.0 } };

        vertices[vertex_index++] = HmdVector2_t { .v = { 1.0, 0.0 } };
        vertices[vertex_index++] = HmdVector2_t { .v = { p.x, p.y } };
        vertices[vertex_index++] = HmdVector2_t { .v = { p.z, p.y } };

        vertices[vertex_index++] = HmdVector2_t { .v = { 1.0, 0.0 } };
        vertices[vertex_index++] = HmdVector2_t { .v = { p.z, p.y } };
        vertices[vertex_index++] = HmdVector2_t { .v = { 1.0, 1.0 } };

        vertices[vertex_index++] = HmdVector2_t { .v = { 1.0, 1.0 } };
        vertices[vertex_index++] = HmdVector2_t { .v = { p.z, p.y } };
        vertices[vertex_index++] = HmdVector2_t { .v = { p.z, p.w } };

        vertices[vertex_index++] = HmdVector2_t { .v = { 1.0, 1.0 } };
        vertices[vertex_index++] = HmdVector2_t { .v = { p.z, p.w } };
        vertices[vertex_index++] = HmdVector2_t { .v = { 0.0, 1.0 } };

        vertices[vertex_index++] = HmdVector2_t { .v = { 0.0, 1.0 } };
        vertices[vertex_index++] = HmdVector2_t { .v = { p.z, p.w } };
        vertices[vertex_index++] = HmdVector2_t { .v = { p.x, p.w } };

        vertices[vertex_index++] = HmdVector2_t { .v = { 0.0, 1.0 } };
        vertices[vertex_index++] = HmdVector2_t { .v = { p.x, p.w } };
        vertices[vertex_index++] = HmdVector2_t { .v = { 0.0, 0.0 } };

        vertices[vertex_index++] = HmdVector2_t { .v = { 0.0, 0.0 } };
        vertices[vertex_index++] = HmdVector2_t { .v = { p.x, p.w } };
        vertices[vertex_index++] = HmdVector2_t { .v = { p.x, p.y } };

        assert(vertex_index == num_vertices);
    }

    return {
        .pVertexData = vertices,
        .unTriangleCount = 8,
    };
}

void
HiddenAreaMesh::destroy_rectangular_mesh(HiddenAreaMesh_t& mesh)
{
    delete[] mesh.pVertexData;

    mesh.pVertexData = nullptr;
    mesh.unTriangleCount = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

id<MTLDevice>
VRSystem::GetOutputDevice()
{
    id<MTLDevice> device = nil;

    static_assert(sizeof(id<MTLDevice>) == sizeof(uint64_t), "!");
    uint64_t* const device_u64 = reinterpret_cast<uint64_t*>(uintptr_t(&device));
    m_system->GetOutputDevice(device_u64, TextureType_IOSurface);

    return device;
}

void
VRSystem::GetRecommendedRenderTargetSize(NSUInteger& width, NSUInteger& height)
{
    uint32_t width_u32 = 0, height_u32 = 0;

    m_system->GetRecommendedRenderTargetSize(&width_u32, &height_u32);

    width = width_u32;
    height = height_u32;
}

std::unique_ptr<HiddenAreaMesh>
VRSystem::GetHiddenAreaMesh(id<MTLDevice> device, EVREye eye, EHiddenAreaMeshType type)
{
    return std::make_unique<HiddenAreaMesh>(device, type, m_system->GetHiddenAreaMesh(eye, type));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t
Utils::io_surface_pixel_format_from_supported_metal_pixel_format(MTLPixelFormat pixel_format)
{
    switch (pixel_format) {
        case MTLPixelFormatRGBA8Unorm:
        case MTLPixelFormatRGBA8Unorm_sRGB:
        case MTLPixelFormatRGBA8Uint:
            return 'RGBA';

        case MTLPixelFormatBGRA8Unorm:
        case MTLPixelFormatBGRA8Unorm_sRGB:
            return 'BGRA';

        default:
            return 0;
    }
}

#if (__MAC_OS_X_VERSION_MIN_REQUIRED >= 101400)
MTLTextureDescriptor*
Utils::new_texture_desc_for_eye_texture(MTLPixelFormat pixel_format,
                                        NSUInteger width,
                                        NSUInteger height,
                                        bool array,
                                        NSUInteger sample_count)
{
    //------------------------------------------------------------------------------
    // Create the texture descriptor.
    MTLTextureDescriptor* const texture_desc = [MTLTextureDescriptor new];
    {
        if (sample_count > 1) {
            if (array) {
                texture_desc.textureType = MTLTextureType2DMultisampleArray;
            }
            else {
                texture_desc.textureType = MTLTextureType2DMultisample;
            }
        }
        else {
            if (array) {
                texture_desc.textureType = MTLTextureType2DArray;
            }
            else {
                texture_desc.textureType = MTLTextureType2D;
            }
        }

        texture_desc.pixelFormat = pixel_format;
        texture_desc.width = width;
        texture_desc.height = height;
        texture_desc.sampleCount = sample_count;

        if (array) {
            texture_desc.arrayLength = 2;
        }

        texture_desc.storageMode = MTLStorageModePrivate;
        texture_desc.usage = (MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget);
    }

    //------------------------------------------------------------------------------
    // ...
    return texture_desc;
}
#endif // __MAC_OS_X_VERSION_MIN_REQUIRED

MTLTextureDescriptor*
Utils::new_texture_desc_for_io_surface_backed_eye_texture(MTLPixelFormat pixel_format,
                                                          NSUInteger width,
                                                          NSUInteger height)
{
    //------------------------------------------------------------------------------
    // Check pixel format.
    const uint32_t io_surface_pixel_format = io_surface_pixel_format_from_supported_metal_pixel_format(pixel_format);

    if (io_surface_pixel_format == 0) {
        return nil;
    }

    //------------------------------------------------------------------------------
    // Create the texture descriptor.
    MTLTextureDescriptor* const texture_desc = [MTLTextureDescriptor new];
    {
        texture_desc.textureType = MTLTextureType2D;

        texture_desc.pixelFormat = pixel_format;
        texture_desc.width = width;
        texture_desc.height = height;

        texture_desc.storageMode = MTLStorageModeManaged;
        texture_desc.usage = (MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget);
    }

    return texture_desc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

id<MTLTexture>
Utils::new_io_surface_backed_eye_texture(id<MTLDevice> device, MTLTextureDescriptor* const texture_desc)
{
    const metal::io_surface_wrapper_t io_surface = new_io_surface_for_eye_texture(texture_desc);
    return [device newTextureWithDescriptor:texture_desc iosurface:io_surface.get() plane:0];
}

id<MTLTexture>
Utils::new_io_surface_backed_eye_texture(id<MTLDevice> device,
                                         MTLPixelFormat pixel_format,
                                         NSUInteger width,
                                         NSUInteger height)
{
    MTLTextureDescriptor* const texture_desc = new_texture_desc_for_io_surface_backed_eye_texture(pixel_format, width, height);
    return new_io_surface_backed_eye_texture(device, texture_desc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

io_surface_wrapper_t
Utils::new_io_surface_for_eye_texture(MTLTextureDescriptor* const texture_desc)
{
    io_surface_wrapper_t io_surface(nullptr, [](IOSurfaceRef const surface) {
        if (surface) {
            CFRelease(surface);
        }
    });

    //------------------------------------------------------------------------------
    // Check pixel format.
    const uint32_t io_surface_pixel_format = io_surface_pixel_format_from_supported_metal_pixel_format(texture_desc.pixelFormat);

    if (io_surface_pixel_format == 0) {
        return io_surface;
    }

    //------------------------------------------------------------------------------
    // Create IOSurface properties.
    NSDictionary* const io_surface_properties =  @{
        (id)kIOSurfaceWidth           : @(texture_desc.width),
        (id)kIOSurfaceHeight          : @(texture_desc.height),

        //------------------------------------------------------------------------------
        // is_valid_pixel_format_for_eye_texture() only allows four byes/pixel formats.
        (id)kIOSurfaceBytesPerElement : @(4),

        //------------------------------------------------------------------------------
        // kIOSurfaceIsGlobal was deprecated in favor of using IOSurfaceCreateMachPort
        // or IOSurfaceCreateXPCObject but SteamVR still uses (as of 2018-08-25) shared
        // memory to submit IOSurface-backed textures to the compositor so this property
        // must be set for now.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        (id)kIOSurfaceIsGlobal        : @YES,
#pragma GCC diagnostic pop

        //------------------------------------------------------------------------------
        // We just need a four component format.
        (id)kIOSurfacePixelFormat     : @(io_surface_pixel_format)
    };

    io_surface.reset(IOSurfaceCreate((__bridge CFDictionaryRef)io_surface_properties));

    return io_surface;
}

io_surface_wrapper_t
Utils::new_io_surface_for_eye_texture(MTLPixelFormat pixel_format,
                                      NSUInteger width,
                                      NSUInteger height)
{
    MTLTextureDescriptor* const texture_desc = new_texture_desc_for_io_surface_backed_eye_texture(pixel_format, width, height);
    return new_io_surface_for_eye_texture(texture_desc);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

METAL_OPENVR_NAMESPACE_END

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
