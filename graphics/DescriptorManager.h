#pragma once

#include "../core/Common.h"
#include "Texture.h"
#include "InstanceContext.h"
#include "ShaderLayout.h"
#include "GraphicsException.h"
#include "RendererTypes.h"

NAMESPACES( SomeVulkan, Graphics )

class Texture;

typedef struct TextureDescription {
    vk::ImageView imageView;
} TextureDescription;

struct BindingUpdateInfo {
    uint32_t index{};
    vk::DescriptorSet parent{};
    DeviceBuffer buffer;
    BindingUpdateInfo() = default;
};

typedef struct TextureBindingUpdateInfo {
    BindingUpdateInfo updateInfo { };
    std::shared_ptr< Texture > texture;
} TextureBindingUpdateInfo;

class InstanceContext;

class DescriptorManager {
private:
    std::shared_ptr< InstanceContext > context;
    std::shared_ptr< ShaderLayout > shaderLayout;

    vk::Sampler sampler{ };
    vk::ImageView imageView{ };
public:
    explicit DescriptorManager( const std::shared_ptr< InstanceContext >& renderContext,
                       const std::shared_ptr< ShaderLayout >& shaderLayout );

    void updateUniformDescriptorSetBinding( const BindingUpdateInfo& updateInfo );
    void updateTextureDescriptorSetBinding( const TextureBindingUpdateInfo& updateInfo );

    DescriptorManager( const DescriptorManager & ) = delete;
    DescriptorManager &operator=( const DescriptorManager & ) = delete;

    ~DescriptorManager();
private:
    void createDescriptorSets( );
    vk::WriteDescriptorSet getCommonWriteDescriptorSet( const BindingUpdateInfo &updateInfo );
};

END_NAMESPACES

