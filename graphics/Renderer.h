#pragma once

#include "../core/Common.h"
#include "CommandExecutor.h"
#include "../ECS.h"
#include "RenderUtilities.h"
#include "RendererTypes.h"
#include "DefaultShaderLayout.h"
#include "../renderobjects/Model.h"
#include "../renderobjects/Triangle2D.h"

NAMESPACES( SomeVulkan, Graphics )

using namespace ECS;

class Renderer {
private:
    const DeviceBufferSize INITIAL_VBO_SIZE{ 200 * 65536 * sizeof(float) };
    const DeviceBufferSize INITIAL_IBO_SIZE{ 100 * sizeof( uint32_t ) };
    const DeviceBufferSize INITIAL_UBO_SIZE{ 3 * 4 * 4 * sizeof( float ) };

    const DeviceBufferSize INITIAL_TEX_SIZE = DeviceBufferSize { vk::Extent2D{ 100, 100 } };

    uint32_t poolSize = 3;
    uint32_t frameIndex = 0;

    std::shared_ptr< InstanceContext > context;
    std::vector< FrameContext > frameContexts;
    std::vector< vk::CommandBuffer > buffers;

    std::vector< vk::Semaphore > imageAvailableSemaphores;
    std::vector< vk::Semaphore > renderFinishedSemaphores;
    std::vector< vk::Fence > imagesInFlight;
    std::vector< vk::Fence > inFlightFences;
    std::vector< std::shared_ptr< Renderable > > renderObjects;

    bool frameBufferResized = false;
    vk::DeviceSize currentVbBufferSize = 0;
    vk::DeviceSize currentIndexBufferSize = 0;
    std::shared_ptr< ShaderLayout > shaderLayout;
    std::shared_ptr< RenderObject::Triangle2D > triangle;
    RenderObjects::Model model = RenderObjects::Model( PATH( "/assets/models/viking_room.obj" ) );
public:
    explicit Renderer( const std::shared_ptr< InstanceContext > &context, const std::shared_ptr< ShaderLayout >& shaderLayout );
    void addRenderObject( const std::shared_ptr< IGameEntity > &gameEntity );
    void render( );
    void freeBuffers( );
    ~Renderer( );

    Renderer( const Renderer & ) = delete;
    Renderer &operator=( const Renderer & ) = delete;

    inline FrameContext& getFrameContext( uint32_t image ) {
        return frameContexts[ image ];
    }
private:

    template< class T, class V = std::vector< T > >
    void transferData( const V &v, DeviceMemory &targetMemory, vk::DeviceSize offset ) {
        transferData< T, V >( v, targetMemory, offset, DeviceBufferSize { v.size() });
    }

    template< class T, class V = std::vector< T > >
    void transferData( const V &v, DeviceMemory &targetMemory, vk::DeviceSize offset, const DeviceBufferSize& bufferSize ) {
        ensureMemorySize( bufferSize, targetMemory );

        RenderUtilities::copyToDeviceMemory(
                context->logicalDevice,
                targetMemory.memory,
                ( const void * ) v.data( ),
                v.size( ) * sizeof( T ),
                offset
        );
    }

    void drawRenderObjects( );
    void refreshCommands( const std::shared_ptr< Renderable > &renderable );
    void ensureMemorySize( const DeviceBufferSize &requiredSize, DeviceMemory &memory );
    void allocateDeviceMemory( const DeviceBufferSize &size, DeviceMemory &dm );
    void createSynchronizationStructures( const vk::Device &device );
    void clearDeviceMemory( );
    void createFrameContexts( );
    void ensureEnoughTexBuffers( uint32_t size );
};

END_NAMESPACES