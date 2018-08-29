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

#ifndef __OPENVR_METAL_H__
#define __OPENVR_METAL_H__

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(__cplusplus) || !defined(__OBJC__)
#   error "This header is designed to work with Objective-C++!"
#endif // __cplusplus, __OBJC__

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import <Metal/Metal.h>
#import <OpenVR/OpenVR.h>
#import <simd/simd.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define OPENVR_METAL_EXPORT __attribute__((visibility ("default")))
#define OPENVR_METAL_EXTERN extern OPENVR_METAL_EXPORT

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define METAL_OPENVR_NAMESPACE_BEGIN namespace vr { namespace metal {
#define METAL_OPENVR_NAMESPACE_END }}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

METAL_OPENVR_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::unique_ptr<std::remove_pointer<IOSurfaceRef>::type, void(*)(IOSurfaceRef)> io_surface_wrapper_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Utility class for generating/drawing hidden area meshes.
//
// An internal vertex buffer (array of simd_float2) is created from an OpenVR
// hidden mesh. The buffer is initially allocated in shared storage on iOS and
// on macOS if the given Metal device is an integrated GPUs. If the Metal device
// is a discreet/external GPUs managed storage is used. Optionally the buffer
// can be moved to private storage after construction.
//------------------------------------------------------------------------------

class HiddenAreaMesh
{
    //------------------------------------------------------------------------------
    // Construction/Destruction
public:

    //------------------------------------------------------------------------------
    // Construct a Metal hidden area mesh to be used with the given device from the
    // given OpenVR hidden area mesh.
    HiddenAreaMesh(id<MTLDevice> device, EHiddenAreaMeshType type, const HiddenAreaMesh_t& mesh);

    //------------------------------------------------------------------------------
    // Vertex Buffer Management
public:

    //------------------------------------------------------------------------------
    // The vertex buffer containing an array of simd_float2.
    id<MTLBuffer> vertex_buffer() const { return m_vertex_buffer; }

    //------------------------------------------------------------------------------
    // Move the vertex buffer to GPU private storage. See copy_to_private_storage()
    // for details.
    void move_to_private_storage(id encoder, bool wait_until_completed);

    //------------------------------------------------------------------------------
    // Mesh Drawing Support
public:

    //------------------------------------------------------------------------------
    // Add the layout/attribute descriptor required for drawing a hidden area mesh
    // to the given vertex descriptor.
    static void add_to_vertex_descriptor(MTLVertexDescriptor* const vertex_desciptor,
                                         NSUInteger buffer_index,
                                         NSUInteger position_attribute_index);

    //------------------------------------------------------------------------------
    // The primitive type (triangles/lines) used for the mesh (outline).
    MTLPrimitiveType primitive_type() const { return m_primitive_type; }

    //------------------------------------------------------------------------------
    // The number of vertices in the vertex buffer.
    NSUInteger num_vertices() const { return m_num_vertices; }

    //------------------------------------------------------------------------------
    // Encode a draw primitives command for the mesh on the given render command
    // encoder after binding the vertex buffer to the given index. A matching render
    // pipeline state must have been set.
    void draw_primitives(id<MTLRenderCommandEncoder> render_command_encoder, NSUInteger buffer_index)
    {
        [render_command_encoder setVertexBuffer:m_vertex_buffer offset:0 atIndex:buffer_index];
        [render_command_encoder drawPrimitives:m_primitive_type vertexStart:0 vertexCount:m_num_vertices];
    }

    //------------------------------------------------------------------------------
    // {Debugging}
public:

    static HiddenAreaMesh_t create_rectangular_mesh(float coverage);
    static void destroy_rectangular_mesh(HiddenAreaMesh_t& mesh);

    //------------------------------------------------------------------------------
    // {Private}
private:

    id<MTLBuffer>           m_vertex_buffer;
    MTLPrimitiveType        m_primitive_type;
    NSUInteger              m_num_vertices;
};

//------------------------------------------------------------------------------
// Utility class for working with OpenVR and Metal.
//------------------------------------------------------------------------------

class VRSystem
{
    //------------------------------------------------------------------------------
    // Construction/Destruction
public:

    explicit VRSystem(IVRSystem* const system) : m_system(system) {}
    ~VRSystem() {}

    //------------------------------------------------------------------------------
    // Wrappers for OpenVR IVRSystem methods (of the same name) modified for working
    // directly with Metal specific arguments and objects.
public:

    //------------------------------------------------------------------------------
    // The Metal device used by OpenVR.
    id<MTLDevice> GetOutputDevice();

    //------------------------------------------------------------------------------
    // The recommended render target size.
    void GetRecommendedRenderTargetSize(NSUInteger& width, NSUInteger& height);

    //------------------------------------------------------------------------------
    // Create a hidden area mesh for the given eye for drawing on the given device.
    std::unique_ptr<HiddenAreaMesh> GetHiddenAreaMesh(id<MTLDevice> device, EVREye eye, EHiddenAreaMeshType type);

    //------------------------------------------------------------------------------
    // {Private}
private:

    IVRSystem* const        m_system;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Utils
{
    //------------------------------------------------------------------------------
    // Math Types
public:

    //------------------------------------------------------------------------------
    // Convert a OpenVR 3x3 matrix to a simd 3x3 matrix.
    static simd_float3x3 simd_from_hmd_matrix(const HmdMatrix33_t& m)
    {
        return simd_matrix(vector3(m.m[0][0], m.m[1][0], m.m[2][0]),
                           vector3(m.m[0][1], m.m[1][1], m.m[2][1]),
                           vector3(m.m[0][2], m.m[1][2], m.m[2][2]));
    }

    //------------------------------------------------------------------------------
    // Convert a OpenVR 3x4 matrix to a simd 4x4 matrix.
    static simd_float4x4 simd_from_hmd_matrix(const HmdMatrix34_t& m)
    {
        static_assert(sizeof(m.m[0]) == sizeof(simd_packed_float4), "!");
        static_assert(alignof(decltype(m.m[0])) == alignof(simd_packed_float4), "!");

        return simd_matrix_from_rows((simd_packed_float4&)m.m[0],
                                     (simd_packed_float4&)m.m[1],
                                     (simd_packed_float4&)m.m[2],
                                     vector4(0.0f, 0.0f, 0.0f, 1.0f));
    }

    //------------------------------------------------------------------------------
    // Convert a OpenVR 4x4 matrix to a simd 4x4 matrix.
    static simd_float4x4 simd_from_hmd_matrix(const HmdMatrix44_t& m)
    {
        static_assert(sizeof(m.m[0]) == sizeof(simd_packed_float4), "!");
        static_assert(alignof(decltype(m.m[0])) == alignof(simd_packed_float4), "!");

        return simd_matrix_from_rows((simd_packed_float4&)m.m[0],
                                     (simd_packed_float4&)m.m[1],
                                     (simd_packed_float4&)m.m[2],
                                     (simd_packed_float4&)m.m[3]);
    }

    //------------------------------------------------------------------------------
    // Texture Descriptors for Eye Textures
public:

    //------------------------------------------------------------------------------
    // Verify Metal pixel format is a valid format for eye textures.
    static uint32_t io_surface_pixel_format_from_supported_metal_pixel_format(MTLPixelFormat pixel_format);

#if (__MAC_OS_X_VERSION_MIN_REQUIRED >= 101400)
    static MTLTextureDescriptor* new_texture_desc_for_eye_texture(MTLPixelFormat pixel_format,
                                                                  NSUInteger width,
                                                                  NSUInteger height,
                                                                  bool array,
                                                                  NSUInteger sample_count = 1);
#endif // __MAC_OS_X_VERSION_MIN_REQUIRED

    //------------------------------------------------------------------------------
    // Create a Metal texture descriptor for new_io_surface_backed_eye_texture().
    static MTLTextureDescriptor* new_texture_desc_for_io_surface_backed_eye_texture(MTLPixelFormat pixel_format,
                                                                                    NSUInteger width,
                                                                                    NSUInteger height);

    //------------------------------------------------------------------------------
    // Creating IOSurface Backed Eye Textures
public:

    //------------------------------------------------------------------------------
    // Create an IOSurface backed Metal eye texture using the given texture desc.
    static id<MTLTexture> new_io_surface_backed_eye_texture(id<MTLDevice> device, MTLTextureDescriptor* const texture_desc);

    //------------------------------------------------------------------------------
    // Create an IOSurface backed Metal eye texture using the given format.
    static id<MTLTexture> new_io_surface_backed_eye_texture(id<MTLDevice> device,
                                                            MTLPixelFormat pixel_format,
                                                            NSUInteger width,
                                                            NSUInteger height);

    //------------------------------------------------------------------------------
    // Creating IOSurface(s) for Backing Eye Textures
public:

    //------------------------------------------------------------------------------
    // Create an IOSurface for backing an eye texture using the given texture desc.
    static io_surface_wrapper_t new_io_surface_for_eye_texture(MTLTextureDescriptor* const texture_desc);

    //------------------------------------------------------------------------------
    // Create an IOSurface for backing an eye texture using the given format.
    static io_surface_wrapper_t new_io_surface_for_eye_texture(MTLPixelFormat pixel_format,
                                                               NSUInteger width,
                                                               NSUInteger height);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

METAL_OPENVR_NAMESPACE_END

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // __OPENVR_METAL_H__

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
