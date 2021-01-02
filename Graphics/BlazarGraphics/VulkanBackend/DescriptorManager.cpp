#include "DescriptorManager.h"

#include <utility>


NAMESPACES( ENGINE_NAMESPACE, Graphics )

const uint32_t DescriptorManager::texturePreallocateCount = 12;

DescriptorManager::DescriptorManager( VulkanContext * context, std::shared_ptr< GLSLShaderSet > shaderSet ) : context( context ), shaderSet( std::move( shaderSet ) )
{
    createDescriptorPool( );

    uniformLayouts.resize( this->shaderSet->getDescriptorSets( ).size( ) );
    textureLayouts.resize( uniformLayouts.size( ) );
    layouts.resize( uniformLayouts.size( ) );

    for ( const auto &sets: this->shaderSet->getDescriptorSets( ) )
    {
        uint32_t setIndex = sets.id;

        for ( const auto &bindings: sets.descriptorSetBindings )
        {
            orders.push_back( { bindings.type, bindings.name, setIndex, bindings.index } );

            uniformLocations[ bindings.name ] = UniformLocation {
                    true, setIndex, bindings.index,
            };
        }

        const DescriptorSet &set = this->shaderSet->getDescriptorSetBySetId( setIndex );

        vk::DescriptorSetLayoutCreateInfo createInfo { };

        std::vector< vk::DescriptorSetLayoutBinding > vkDescriptorSetBindings = set.descriptorSetLayoutBindings;

        createInfo.bindingCount = vkDescriptorSetBindings.size( );
        createInfo.pBindings = vkDescriptorSetBindings.data( );

        if ( vkDescriptorSetBindings[ 0 ].descriptorType == vk::DescriptorType::eCombinedImageSampler )
        {
            textureLayouts[ setIndex ] = this->context->logicalDevice.createDescriptorSetLayout( createInfo );
            uniformLayouts[ setIndex ] = nullptr;
            layouts[ setIndex ] = textureLayouts[ setIndex ];
        }
        else
        {
            textureLayouts[ setIndex ] = nullptr;
            uniformLayouts[ setIndex ] = this->context->logicalDevice.createDescriptorSetLayout( createInfo );
            layouts[ setIndex ] = uniformLayouts[ setIndex ];
        }
    }

    for ( uint32_t i = 0; i < this->context->swapChainImages.size( ); ++i )
    {
        for ( auto &pushConstantDetail: this->shaderSet->getPushConstantDetails( ) )
        {
            auto &parent = pushConstantParents[ pushConstantDetail.stage ].emplace_back( );

            parent.data = ( char * ) malloc( pushConstantDetail.size );
            parent.totalSize = pushConstantDetail.size;
            parent.stage = pushConstantDetail.stage;

            if ( i == 0 )
            {
                stagesWithPushConstants.push_back( pushConstantDetail.stage );
            }

            for ( const auto& childElement: pushConstantDetail.children )
            {
                auto & binding = pushConstants[ childElement.name ].emplace_back( );
                binding.ref = childElement;
                binding.parent = parent;
            }
        }
    }

    std::sort( orders.begin( ), orders.end( ), [ ]( const DescriptorOrderInfo &o1, const DescriptorOrderInfo &o2 )
    {
        if ( o1.set == o2.set )
        {
            return o1.location <= o2.location;
        }

        return o1.set < o2.set;
    } );
}

void DescriptorManager::addUniformDescriptorSet( const std::string &uniformName, UniformLocation &location, vk::DescriptorSetLayout &layout, const uint32_t& objectIndex  )
{
    uint32_t swapChainImageCount = context->swapChainImages.size( );

    std::vector< vk::DescriptorSetLayout > layoutsPtr { swapChainImageCount, layout };

    vk::DescriptorSetAllocateInfo allocateInfo { };
    allocateInfo.descriptorPool = uniformDescriptorPool;
    allocateInfo.descriptorSetCount = layoutsPtr.size( );
    allocateInfo.pSetLayouts = layoutsPtr.data( );

    if ( uniformSetMaps.find( uniformName ) == uniformSetMaps.end( ) )
    {
        uniformSetMaps[ uniformName ] = { };
    }

    if ( uniformSetMaps[ uniformName ].size( ) <= objectIndex )
    {
        uniformSetMaps[ uniformName ].resize( objectIndex + 100 );
    }

    uniformSetMaps[ uniformName ][ objectIndex ] = context->logicalDevice.allocateDescriptorSets( allocateInfo );
}

void DescriptorManager::addTextureDescriptorSet( const std::string &uniformName, UniformLocation &location, vk::DescriptorSetLayout &layout, const uint32_t& objectIndex )
{
    uint32_t swapChainImageCount = context->swapChainImages.size( );

    std::vector< vk::DescriptorSetLayout > layoutsPtr { swapChainImageCount, layout };

    vk::DescriptorSetAllocateInfo allocateInfo { };
    allocateInfo.descriptorPool = samplerDescriptorPool;
    allocateInfo.descriptorSetCount = layoutsPtr.size( );
    allocateInfo.pSetLayouts = layoutsPtr.data( );

    if ( textureSetMaps.find( uniformName ) == textureSetMaps.end( ) )
    {
        textureSetMaps[ uniformName ] = { };
    }

    if ( textureSetMaps[ uniformName ].size( ) <= objectIndex )
    {
        textureSetMaps[ uniformName ].resize( objectIndex + 100 );
    }

    textureSetMaps[ uniformName ][ objectIndex ] = context->logicalDevice.allocateDescriptorSets( allocateInfo );
}

// Todo create more pools as necessary
void DescriptorManager::createDescriptorPool( )
{
    auto swapChainImageCount = static_cast< uint32_t >( context->swapChainImages.size( ) );

    vk::DescriptorPoolSize uniformPoolSize { };
    uniformPoolSize.type = vk::DescriptorType::eUniformBuffer;
    uniformPoolSize.descriptorCount = swapChainImageCount;

    vk::DescriptorPoolCreateInfo uniformDescriptorPoolCreateInfo { };
    uniformDescriptorPoolCreateInfo.poolSizeCount = 1;
    uniformDescriptorPoolCreateInfo.pPoolSizes = &uniformPoolSize;
    uniformDescriptorPoolCreateInfo.maxSets = swapChainImageCount * 120;

    uniformDescriptorPool = context->logicalDevice.createDescriptorPool( uniformDescriptorPoolCreateInfo );

    vk::DescriptorPoolSize samplerPoolSize { };
    samplerPoolSize.type = vk::DescriptorType::eCombinedImageSampler;
    samplerPoolSize.descriptorCount = swapChainImageCount;

    vk::DescriptorPoolCreateInfo samplerDescriptorPoolCreateInfo { };
    samplerDescriptorPoolCreateInfo.poolSizeCount = 1;
    samplerDescriptorPoolCreateInfo.pPoolSizes = &samplerPoolSize;
    samplerDescriptorPoolCreateInfo.maxSets = swapChainImageCount * 120;

    samplerDescriptorPool = context->logicalDevice.createDescriptorPool( samplerDescriptorPoolCreateInfo );
}

vk::WriteDescriptorSet DescriptorManager::getCommonWriteDescriptorSet( const UniformLocation &uniformLocation, const BindingUpdateInfo &updateInfo )
{
    const DescriptorSet &set = shaderSet->getDescriptorSetBySetId( uniformLocation.set );

    const DescriptorSetBinding &ref = set.descriptorSetBindings[ updateInfo.index ];
    vk::WriteDescriptorSet writeDescriptorSet { };

    writeDescriptorSet.dstSet = updateInfo.parent;
    writeDescriptorSet.dstBinding = uniformLocation.binding;
    writeDescriptorSet.dstArrayElement = updateInfo.arrayElement;
    writeDescriptorSet.descriptorType = ref.type;
    writeDescriptorSet.descriptorCount = 1;

    return writeDescriptorSet;
}

void DescriptorManager::ensureTextureHasDescriptor( const uint32_t &frameIndex, const std::string &uniformName, const uint32_t& objectIndex  )
{
    const std::string &key = getUniformKey( uniformName );

    auto findResult = textureSetMaps.find( key );

    if ( findResult == textureSetMaps.end( ) || findResult->second.size( ) <= objectIndex || findResult->second[ objectIndex ].empty( ) )
    {
        auto &uniformSet = uniformLocations[ uniformName ];
        auto &layout = textureLayouts[ uniformSet.set ];

        addTextureDescriptorSet( uniformName, uniformSet, layout, objectIndex );
    }
}

void DescriptorManager::ensureUniformHasDescriptor( const uint32_t &frameIndex, const std::string &uniformName, const uint32_t& objectIndex, const int &arrayIndex  )
{
    const std::string &key = getUniformKey( uniformName, arrayIndex );

    auto findResult = uniformSetMaps.find( key );

    if ( findResult == uniformSetMaps.end( ) || findResult->second.size( ) <= objectIndex || findResult->second[ objectIndex ].empty( ) )
    {
        auto &uniformSet = uniformLocations[ uniformName ];
        auto &layout = uniformLayouts[ uniformSet.set ];

        addUniformDescriptorSet( uniformName, uniformSet, layout, objectIndex );
    }
}

vk::DescriptorSet &DescriptorManager::getUniformDescriptorSet( const uint32_t &frameIndex, const std::string &uniformName, const uint32_t& objectIndex, const uint32_t &arrayIndex )
{
    ensureUniformHasDescriptor( frameIndex, uniformName, objectIndex, arrayIndex );

    return uniformSetMaps[ getUniformKey( uniformName, arrayIndex ) ][ objectIndex ][ frameIndex ];
}

vk::DescriptorSet &DescriptorManager::getTextureDescriptorSet( const uint32_t &frameIndex, const std::string &uniformName, const uint32_t& objectIndex, const uint32_t &textureIndex )
{
    ensureTextureHasDescriptor( frameIndex, uniformName, objectIndex);

    return textureSetMaps[ uniformName ][ objectIndex ][ frameIndex ];
}

void DescriptorManager::updatePushConstant( const uint32_t &frameIndex, const std::string &uniformName, void *data )
{
    auto &pushConstantChild = pushConstants[ uniformName ][ frameIndex ];

    memcpy( pushConstantChild.parent.data + pushConstantChild.ref.offset, data, pushConstantChild.ref.size );
}

void DescriptorManager::updateUniform( const uint32_t &frameIndex, const std::string &uniformName, const std::pair< vk::Buffer, vma::Allocation > &buffer,
                                       const uint32_t& objectIndex, const int &arrayIndex )
{
    ensureUniformHasDescriptor( frameIndex, uniformName, objectIndex, arrayIndex );

    const std::string &key = getUniformKey( uniformName, arrayIndex );

    UniformLocation &uniformLocation = uniformLocations[ key ];

    BindingUpdateInfo updateInfo {
            uniformLocation.binding,
            uniformSetMaps[ key ][ objectIndex ][ frameIndex ],
            arrayIndex < 0 ? 0 : uint32_t( arrayIndex )
    };

    const DescriptorSet &set = shaderSet->getDescriptorSetBySetId( uniformLocation.set );
    const DescriptorSetBinding &ref = set.descriptorSetBindings[ uniformLocation.binding ];
    vk::WriteDescriptorSet writeDescriptorSet = getCommonWriteDescriptorSet( uniformLocation, updateInfo );

    vk::DescriptorBufferInfo descriptorBufferInfo { };
    descriptorBufferInfo.buffer = buffer.first;
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = ref.size;

    writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    context->logicalDevice.updateDescriptorSets( 1, &writeDescriptorSet, 0, nullptr );
}

void DescriptorManager::updateTexture( const uint32_t &frameIndex, const std::string &uniformName, const VulkanTextureWrapper &buffer, const uint32_t& objectIndex )
{
    ensureTextureHasDescriptor( frameIndex, uniformName, objectIndex );

    // Todo + texture index when texture arrays are implemented
    const std::string &key = getUniformKey( uniformName );

    UniformLocation &uniformLocation = uniformLocations[ key ];
    BindingUpdateInfo updateInfo {
            uniformLocation.binding,
            textureSetMaps[ uniformName ][ objectIndex ][ frameIndex ]
    };

    const DescriptorSet &set = shaderSet->getDescriptorSetBySetId( uniformLocation.set );
    const DescriptorSetBinding &ref = set.descriptorSetBindings[ uniformLocation.binding ];
    vk::WriteDescriptorSet writeDescriptorSet = getCommonWriteDescriptorSet( uniformLocation, updateInfo );

    vk::DescriptorImageInfo descriptorImageInfo { };
    descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    descriptorImageInfo.imageView = buffer.imageView;
    descriptorImageInfo.sampler = buffer.sampler;

    writeDescriptorSet.pImageInfo = &descriptorImageInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    context->logicalDevice.updateDescriptorSets( 1, &writeDescriptorSet, 0, nullptr );
}

const std::vector< vk::DescriptorSetLayout > &DescriptorManager::getLayouts( )
{
    return layouts;
}

std::vector< vk::DescriptorSet > DescriptorManager::getOrderedSets( const uint32_t &frame, const uint32_t& objectIndex )
{
    std::vector< vk::DescriptorSet > set( orders.size( ) );

    for ( uint32_t i = 0; i < set.size( ); ++i )
    {
        if ( orders[ i ].type == vk::DescriptorType::eCombinedImageSampler )
        {
            const auto &objects = textureSetMaps[ orders[ i ].name ];
            // If it is an object updated per frame and not per object, only the 0th index will have values
            // Todo maybe add the possibility to mark an descriptorSet as per frame or per object
            set[ i ] = objects[ objectIndex ].empty( ) ? objects[ 0 ][ frame ] : objects[ objectIndex ][ frame ];
        }
        else
        {
            const auto &objects = uniformSetMaps[ orders[ i ].name ];
            set[ i ] = objects[ objectIndex ].empty( ) ? objects[ 0 ][ frame ] : objects[ objectIndex ][ frame ];
        }
    }

    return set;
}

std::vector< PushConstantParent > DescriptorManager::getPushConstantBindings( const uint32_t &frame )
{
    std::vector< PushConstantParent > result{ };

    for( auto &stage: stagesWithPushConstants )
    {
        result.push_back( pushConstantParents[ stage ][ frame ] );
    }

    return result;
}

std::string DescriptorManager::getUniformKey( const std::string &uniformName, const int &arrayIndex )
{
    if ( arrayIndex == -1 )
    {
        return uniformName;
    }

    std::stringstream key;
    key << uniformName << arrayIndex;
    return key.str( );
}

DescriptorManager::~DescriptorManager( )
{
    context->logicalDevice.destroySampler( sampler );
    context->logicalDevice.destroyImageView( imageView );

    for ( auto &layout: layouts )
    {
        context->logicalDevice.destroyDescriptorSetLayout( layout );
    }

    for ( auto &stage: stagesWithPushConstants )
    {
        for ( auto &frame: pushConstantParents[ stage ] )
        {
            free( frame.data );
        }
    }

    context->logicalDevice.destroyDescriptorPool( uniformDescriptorPool );
    context->logicalDevice.destroyDescriptorPool( samplerDescriptorPool );
}

END_NAMESPACES