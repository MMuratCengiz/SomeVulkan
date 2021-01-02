#pragma once

#include <BlazarCore/Common.h>
#include "Pass.h"
#include "../IRenderDevice.h"

NAMESPACES( ENGINE_NAMESPACE, Graphics )

class CommonPasses
{
private:
    CommonPasses( ) = default;
public:
    static std::shared_ptr< Pass > createGBufferPass(  IRenderDevice* renderDevice );
    static std::shared_ptr< Pass > createDefaultPass(  IRenderDevice* renderDevice );
};

END_NAMESPACES
