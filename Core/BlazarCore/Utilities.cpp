#include <fstream>
#include "Utilities.h"
#include <vector>
#include <glm/gtx/quaternion.hpp>

NAMESPACES( ENGINE_NAMESPACE, Core )

glm::vec3 Utilities::quatToEulerGlm( const btQuaternion &quat3 )
{
    glm::vec3 result;

    quat3.getEulerZYX( result.z, result.y, result.x );

    return result;
}

glm::vec3 Utilities::toGlm( const btVector3 &vec3 )
{
    return glm::vec3(
            vec3.getX(),
            vec3.getY(),
            vec3.getZ()
            );
}

glm::vec4 Utilities::toGlm( const btVector4 &vec4 )
{
    return glm::vec4(
            vec4.getX(),
            vec4.getY(),
            vec4.getZ(),
            vec4.getW()
    );
}

btVector3 Utilities::toBt( glm::vec3 vec )
{
    return btVector3
    {
        vec.x,
        vec.y,
        vec.z
    };
}

btQuaternion Utilities::toBtQuat( glm::vec3 euler, bool isInRadians )
{
    btQuaternion quaternion{ };

    if ( isInRadians )
    {
        quaternion.setEuler(
           euler.x,
           euler.y,
           euler.z
        );
    }
    else
    {
        quaternion.setEuler(
                glm::radians( euler.x ),
                glm::radians( euler.y ),
                glm::radians( euler.z )
        );
    }

    return btQuaternion( );
}


std::vector< char > Utilities::readFile( const std::string &filename )
{
    std::ifstream file( filename, std::ios::ate | std::ios::binary );

    if ( !file.is_open( ) )
    {
        throw std::runtime_error( "failed to open file!" );
    }

    size_t fileSize = static_cast<size_t>( file.tellg( ) );
    std::vector< char > contents( fileSize );

    file.seekg( 0 );
    file.read( contents.data( ), fileSize );

    file.close( );

    return contents;
}

glm::mat4 Utilities::getTRSMatrix(  const glm::vec3& t, const glm::quat& r, const glm::vec3& s )
{
    glm::mat4 identity { 1 };

    auto tMat = glm::translate( identity, t );
    auto rMat = glm::toMat4( r );
    auto sMat = glm::scale( identity, s );

    return tMat * rMat * sMat;
}

glm::quat Utilities::vecToQuat( const glm::vec4 &vec )
{
    auto res = glm::make_quat( glm::value_ptr( vec ) );
    return res;
}

END_NAMESPACES

