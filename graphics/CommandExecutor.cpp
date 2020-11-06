//
// Created by Murat on 10/24/2020.
//

#include "CommandExecutor.h"

using namespace SomeVulkan::Graphics;

CommandExecutor::CommandExecutor( const std::shared_ptr< RenderContext > &context ) : context( context ) {
    VkCommandPoolCreateInfo commandPoolCreateInfo { };

    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = context->queueFamilies[ QueueType::Graphics ].index;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if ( vkCreateCommandPool( context->logicalDevice, &commandPoolCreateInfo, nullptr, &commandPool ) !=
         VK_SUCCESS ) {
        throw std::runtime_error( "failed to create command pool!" );
    }
}

std::shared_ptr< BeginCommandExecution > CommandExecutor::startCommandExecution( ) {
    auto *pExecution = new BeginCommandExecution { this };
    return std::shared_ptr< BeginCommandExecution >( pExecution );
}

CommandExecutor::~CommandExecutor( ) {
    vkDestroyCommandPool( context->logicalDevice, commandPool, nullptr );
}

BeginCommandExecution::BeginCommandExecution( CommandExecutor *executor ) : executor( executor ) { }

std::shared_ptr< CommandList >
BeginCommandExecution::generateBuffers( VkCommandBufferUsageFlags usage, uint16_t bufferCount ) {

    buffers.resize( bufferCount );

    VkCommandBufferAllocateInfo bufferAllocateInfo { };
    bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    bufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    bufferAllocateInfo.commandPool = executor->commandPool;
    bufferAllocateInfo.commandBufferCount = bufferCount;

    vkAllocateCommandBuffers( executor->context->logicalDevice, &bufferAllocateInfo,
                                                buffers.data( ) );

    auto *pCommandList = new CommandList { executor, buffers, usage };
    return std::move( std::shared_ptr< CommandList >( pCommandList ) );
}

CommandList::CommandList(
        CommandExecutor *executor,
        std::vector< VkCommandBuffer > buffers,
        VkCommandBufferUsageFlags usage ) : executor( executor ), buffers( std::move( buffers ) ), usage( usage ) {
}

CommandList *CommandList::beginCommand( ) {
    for ( VkCommandBuffer buffer: buffers ) {
        VkCommandBufferBeginInfo bufferBeginInfo { };

        bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bufferBeginInfo.flags = usage;

        vkBeginCommandBuffer( buffer, &bufferBeginInfo );
    }

    return this;
}

CommandList *CommandList::copyBuffer( const VkDeviceSize &size, VkBuffer &src, VkBuffer &dst ) {
    ENSURE_FILTER

    for ( VkCommandBuffer buffer: buffers ) {
        VkBufferCopy bufferCopy { };
        bufferCopy.size = size;

        vkCmdCopyBuffer( buffer, src, dst, 1, &bufferCopy );
    }

    return this;
}

CommandList *CommandList::beginRenderPass( const VkFramebuffer frameBuffers[], const VkClearValue &clearValue ) {
    ENSURE_FILTER

    uint16_t index = 0;


    VkClearValue clearValues[] = { clearValue, { .depthStencil = { 1.0f, 0 } } };

    for ( VkCommandBuffer buffer: buffers ) {
        VkRenderPassBeginInfo renderPassBeginInfo { };
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = executor->context->renderPass;
        renderPassBeginInfo.framebuffer = frameBuffers[ index++ ];
        renderPassBeginInfo.renderArea.offset = { 0, 0 };
        renderPassBeginInfo.renderArea.extent = executor->context->surfaceExtent;
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;


        vkCmdBeginRenderPass( buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
    }

    return this;
}

CommandList *CommandList::endRenderPass( ) {
    ENSURE_FILTER

    for ( VkCommandBuffer buffer: buffers ) {
        vkCmdEndRenderPass( buffer );
    }

    return this;
}

CommandList *CommandList::bindRenderPass( VkPipelineBindPoint bindPoint ) {
    ENSURE_FILTER

    VkPipeline &pipeline = executor->context->pipeline;

    for ( VkCommandBuffer buffer: buffers ) {
        vkCmdBindPipeline( buffer, bindPoint, pipeline );
    }

    return this;
}

CommandList *CommandList::drawIndexed( const std::vector< uint32_t > &indices ) {
    ENSURE_FILTER

    for ( VkCommandBuffer buffer: buffers ) {
        vkCmdDrawIndexed( buffer, static_cast< uint32_t >( indices.size( ) ), 1, 0, 0, 0 );
    }

    return this;
}

CommandList *CommandList::draw( uint32_t vertexCount ) {
    ENSURE_FILTER

    for ( VkCommandBuffer buffer: buffers ) {
        vkCmdDraw( buffer, vertexCount, 1, 0, 0 );
    }

    return this;
}

CommandList *CommandList::bindVertexMemory( const VkBuffer &vertexBuffer, const VkDeviceSize &offset ) {
    ENSURE_FILTER

    for ( VkCommandBuffer &buffer: buffers ) {
        vkCmdBindVertexBuffers( buffer, 0, 1, &vertexBuffer, &offset );
    }

    return this;
}

CommandList *CommandList::bindIndexMemory( VkBuffer indexBuffer, const VkDeviceSize &offset ) {
    ENSURE_FILTER

    for ( VkCommandBuffer &buffer: buffers ) {
        vkCmdBindIndexBuffer( buffer, indexBuffer, offset, VK_INDEX_TYPE_UINT32 );
    }

    return this;
}

CommandList *CommandList::blitImage( const ImageBlitArgs &args ) {
    ENSURE_FILTER

    VkImageBlit imageBlit { };

    imageBlit.srcOffsets[ 0 ] = args.srcOffsets[ 0 ];
    imageBlit.dstOffsets[ 0 ] = args.dstOffsets[ 0 ];
    imageBlit.srcOffsets[ 1 ] = args.srcOffsets[ 1 ];
    imageBlit.dstOffsets[ 1 ] = args.dstOffsets[ 1 ];

    imageBlit.srcSubresource = args.srcSubresource;
    imageBlit.dstSubresource = args.dstSubresource;


    for ( VkCommandBuffer &buffer: buffers ) {
        vkCmdBlitImage( buffer,
                        args.sourceImage,
                        args.sourceImageLayout,
                        args.destinationImage,
                        args.destinationImageLayout,
                        1,
                        &imageBlit,
                        VK_FILTER_LINEAR );
    }

    return this;
}

CommandList *CommandList::pipelineBarrier( const PipelineBarrierArgs &args ) {
    ENSURE_FILTER

    VkImageMemoryBarrier memoryBarrier { };

    memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    memoryBarrier.oldLayout = args.oldLayout;
    memoryBarrier.newLayout = args.newLayout;
    memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.image = args.image;
    memoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    memoryBarrier.subresourceRange.baseMipLevel = args.baseMipLevel;
    memoryBarrier.subresourceRange.levelCount = args.mipLevel;
    memoryBarrier.subresourceRange.baseArrayLayer = 0;
    memoryBarrier.subresourceRange.layerCount = 1;
    memoryBarrier.srcAccessMask = args.sourceAccess;
    memoryBarrier.dstAccessMask = args.destinationAccess;

    for ( VkCommandBuffer &buffer: buffers ) {
        vkCmdPipelineBarrier(
                buffer,
                args.sourceStage, args.destinationStage,
                0, 0,
                nullptr, 0,
                nullptr,
                1, &memoryBarrier );
    }

    return this;
}

CommandList *CommandList::copyBufferToImage( const CopyBufferToImageArgs &args ) {
    ENSURE_FILTER


    VkBufferImageCopy bufferImageCopy { };

    bufferImageCopy.bufferOffset = 0;
    bufferImageCopy.bufferRowLength = 0;
    bufferImageCopy.bufferImageHeight = 0;
    bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferImageCopy.imageSubresource.mipLevel = 0;
    bufferImageCopy.imageSubresource.baseArrayLayer = 0;
    bufferImageCopy.imageSubresource.layerCount = 1;
    bufferImageCopy.imageOffset = { 0, 0, 0 };
    bufferImageCopy.imageExtent = {
            args.width,
            args.height,
            1
    };

    for ( VkCommandBuffer &buffer: buffers ) {
        vkCmdCopyBufferToImage( buffer, args.sourceBuffer, args.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                &bufferImageCopy );
    }

    return this;
}

CommandList *
CommandList::bindDescriptorSet( const VkPipelineLayout &pipelineLayout, const VkDescriptorSet &descriptorSet ) {
    ENSURE_FILTER

    for ( VkCommandBuffer &buffer: buffers ) {
        vkCmdBindDescriptorSets( buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0,
                                 nullptr );
    }

    return this;
}

VkResult CommandList::execute( ) {
    for ( VkCommandBuffer buffer: buffers ) {
        vkEndCommandBuffer( buffer );
    }

    VkSubmitInfo submitInfo { };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = buffers.size( );
    submitInfo.pCommandBuffers = buffers.data( );

    VkQueue queue = executor->context->queues[ QueueType::Graphics ];
    VkResult result = vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE );
    vkQueueWaitIdle( queue );

    return result;
}

const std::vector< VkCommandBuffer > &CommandList::getBuffers( ) {
    return buffers;
}

CommandList *CommandList::filter( bool condition ) {
    conditionActive = true;
    conditionValue = condition;
    return this;
}

bool CommandList::passesFilter( ) const {
    return !conditionActive || conditionValue;
}

CommandList *CommandList::otherwise( ) {
    conditionValue = !conditionValue;
    isElse = true;
    conditionActive = true;
    return this;
}

CommandList *CommandList::endFilter( ) {
    conditionActive = false;
    isElse = false;
    return this;
}

CommandList::~CommandList( ) {
    freeBuffers( );
}

void CommandList::freeBuffers( ) {
    vkFreeCommandBuffers( executor->context->logicalDevice, executor->commandPool,
                          buffers.size( ),
                          buffers.data( ) );
}
