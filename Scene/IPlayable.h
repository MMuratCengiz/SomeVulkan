#pragma once

#include "../Core/Common.h"

NAMESPACES( SomeVulkan, Scene )

class IPlayable {
public:
    virtual void init( ) = 0;
    virtual void update( ) = 0;
    virtual void dispose( ) = 0;
};

END_NAMESPACES