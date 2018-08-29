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

#include "MetalUtils.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static id<MTLCommandQueue> command_queue_for_device(id<MTLDevice> device)
{
    static NSMutableDictionary* command_queues;
    static dispatch_once_t once;

    dispatch_once(&once, ^{
        command_queues = [NSMutableDictionary dictionary];
    });

    id<MTLCommandQueue> command_queue = [command_queues objectForKey:@(device.registryID)];

    if (command_queue == nil) {
        command_queue = [device newCommandQueue];
        [command_queues setObject:command_queue forKey:@(device.registryID)];
    }

    return command_queue;
}

static id<MTLBuffer> copy_to_private_storage_internal(id<MTLBlitCommandEncoder> blit_command_encoder, id<MTLBuffer> buffer)
{
    id<MTLBuffer> upload_buffer = buffer;

    buffer = [buffer.device newBufferWithLength:upload_buffer.length options:MTLResourceStorageModePrivate];
    [blit_command_encoder copyFromBuffer:upload_buffer sourceOffset:0 toBuffer:buffer destinationOffset:0 size:upload_buffer.length];

    return buffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

id<MTLBuffer> copy_to_private_storage(id<MTLBuffer> buffer, id encoder, bool wait_until_completed)
{
    if (buffer.storageMode == MTLStorageModePrivate) {
        return buffer;
    }

    if (encoder == nil) {
        id<MTLCommandQueue> command_queue = command_queue_for_device(buffer.device);
        id<MTLCommandBuffer> command_buffer = [command_queue commandBuffer];
        id<MTLBlitCommandEncoder> blit_command_encoder = [command_buffer blitCommandEncoder];

        buffer = copy_to_private_storage_internal(blit_command_encoder, buffer);

        [blit_command_encoder endEncoding];
        [command_buffer commit];

        if (wait_until_completed) {
            [command_buffer waitUntilCompleted];
        }
    }
    else if ([encoder conformsToProtocol:@protocol(MTLBlitCommandEncoder)]) {
        buffer = copy_to_private_storage_internal(encoder, buffer);
    }
    else if ([encoder conformsToProtocol:@protocol(MTLCommandBuffer)]) {
        id<MTLBlitCommandEncoder> blit_command_encoder = [encoder blitCommandEncoder];

        buffer = copy_to_private_storage_internal(blit_command_encoder, buffer);

        [blit_command_encoder endEncoding];
    }
    else if ([encoder conformsToProtocol:@protocol(MTLCommandQueue)]) {
        id<MTLCommandBuffer> command_buffer = [encoder commandBuffer];
        id<MTLBlitCommandEncoder> blit_command_encoder = [command_buffer blitCommandEncoder];

        buffer = copy_to_private_storage_internal(blit_command_encoder, buffer);

        [blit_command_encoder endEncoding];
        [command_buffer commit];

        if (wait_until_completed) {
            [command_buffer waitUntilCompleted];
        }
    }
    else {
        NSCAssert(false, @"Valid encoder object expected!");
    }

    return buffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
