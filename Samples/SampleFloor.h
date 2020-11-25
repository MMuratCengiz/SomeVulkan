#pragma once

#include "../ECS/IGameEntity.h"
#include "../ECS/CMesh.h"
#include "../ECS/CMaterial.h"
#include "../ECS/CTransform.h"
#include "../Graphics/BuiltinPrimitives.h"

using namespace SomeVulkan;

namespace Sample {

class SampleFloor : public ECS::IGameEntity {
public:
    SampleFloor( ) {
        auto mesh = createComponent< ECS::CMesh >( );
        mesh->path = Graphics::BuiltinPrimitives::getPrimitivePath( Graphics::PrimitiveType::Cube );

        auto texture = createComponent< ECS::CMaterial >( );
        auto &texInfo = texture->textures.emplace_back( SomeVulkan::ECS::Material::TextureInfo { } );
        texInfo.path = "/assets/textures/asphalt.png";

        auto transform = createComponent< ECS::CTransform >( );
        transform->position = glm::vec3( 0.0f, 0.4f, -2.8f );
    }
};

}