/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <BlazarCore/Common.h>
#include <BlazarCore/Utilities.h>
#include "BlazarECS/ECS.h"

NAMESPACES( ENGINE_NAMESPACE, Physics )

class CollisionShapeInitializer
{
private:
    std::shared_ptr< ECS::CTransform > transform;
    std::shared_ptr< ECS::CCollisionObject > collisionObject;
    std::shared_ptr< ECS::CRigidBody > rigidBody;
    std::shared_ptr< btCollisionShape > shape;
public:
    CollisionShapeInitializer( std::shared_ptr< ECS::CCollisionObject >  collisionObject, std::shared_ptr< ECS::CTransform > transform );
    CollisionShapeInitializer( std::shared_ptr< ECS::CRigidBody > rigidBody, std::shared_ptr< ECS::CTransform > transform );

    void initializeBoxCollisionShape( const glm::vec3& dimensions );
    void initializeSphereCollisionShape( const float& radius );
private:
    void initializeRequestedComponent( );
};

END_NAMESPACES

