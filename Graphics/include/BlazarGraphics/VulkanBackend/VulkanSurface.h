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

#include "VulkanContext.h"
#include "VulkanUtilities.h"
#include "GLSLShaderSet.h"
#include "BlazarECS/CCamera.h"
#include <BlazarInput/GlobalEventHandler.h>
#include <BlazarECS/ECS.h>

NAMESPACES( ENGINE_NAMESPACE, Graphics )

class VulkanSurface
{
private:
    VulkanContext * context;
public:
    explicit VulkanSurface( VulkanContext * context );

    ~VulkanSurface( );
private:
    void createSurface( );
    void createSwapChain( const vk::SurfaceCapabilitiesKHR &surfaceCapabilities );
    void createImageView( vk::ImageView &imageView, const vk::Image &image, const vk::Format &format, const vk::ImageAspectFlags &aspectFlags );
    void chooseExtent2D( const vk::SurfaceCapabilitiesKHR &capabilities );
    void createSwapChainImages( vk::Format format );
    void dispose( );
};
END_NAMESPACES