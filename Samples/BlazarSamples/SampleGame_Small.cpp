#include <BlazarECS/CMaterial.h>
#include "SampleGame_Small.h"

namespace Sample
{

void SampleGame_Small::init( )
{
    sceneLights = std::make_shared< ECS::DynamicGameEntity >( );
    sceneLights->createComponent< ECS::CAmbientLight >( );
    sceneLights->getComponent< ECS::CAmbientLight >( )->diffuse = glm::vec3( 0.25f, 0.25f, 0.22f );
    sceneLights->getComponent< ECS::CAmbientLight >( )->power = 0.005f;

    glm::vec3 pos = glm::vec3( 1, 1, 1 );
    glm::vec3 front = glm::vec3( 0.0, 0.0, 0.0 );

    sceneLights->createComponent< ECS::CDirectionalLight >( );
    sceneLights->getComponent< ECS::CDirectionalLight >( )->diffuse = glm::vec3( 1.0, 1.0f, 0.99f );
    sceneLights->getComponent< ECS::CDirectionalLight >( )->direction = front - pos;
    sceneLights->getComponent< ECS::CDirectionalLight >( )->specular = glm::vec3( 1.0, 1.0f, 0.99f );
    sceneLights->getComponent< ECS::CDirectionalLight >( )->power = 0.15f;

    cameraComponent = std::make_shared< ECS::DynamicGameEntity >( );
    cameraComponent->createComponent< ECS::CCamera >( );
    cameraComponent->getComponent< ECS::CCamera >( )->position = glm::vec3( -0.6f, 1.0f, 5.4f );
    camera = std::make_shared< FpsCamera >( cameraComponent->getComponent< ECS::CCamera >( ) );

    floor = std::make_shared< SampleFloor >( );
    sky = std::make_shared< SampleCubeMap >( );

    animDummy = std::make_shared< SampleAnimatedFox >( world );

    car1 = std::make_shared< SampleCar1 >( world );
    car2 = std::make_shared< SampleCar2 >( world );

    initialScene = std::make_shared< Scene::Scene >( );
    initialScene->addEntity( sceneLights );
    initialScene->addEntity( cameraComponent );
    initialScene->addEntity( floor );
    initialScene->addEntity( sky );
//    initialScene->addEntity( car1 );
//    initialScene->addEntity( car2 );
    initialScene->addEntity( animDummy );
    world->setScene( initialScene );

    Input::GlobalEventHandler::Instance( ).subscribeToEvent( Input::EventType::WindowResized, [ & ]( const Input::EventType &type, const Input::pEventParameters &parameters )
    {
        auto windowParams = Input::GlobalEventHandler::ToWindowResizedParameters( parameters );
        if ( windowParams->width > 0 && windowParams->height > 0 )
        {
            camera->updateAspectRatio( windowParams->width, windowParams->height );
        }
    } );

    Input::GlobalEventHandler::Instance( ).subscribeToEvent( Input::EventType::Tick, [ & ]( const Input::EventType &type, const Input::pEventParameters &parameters )
    {
        auto tickParams = Input::GlobalEventHandler::ToTickParameters( parameters );

        camera->processKeyboardEvents( tickParams->window );
        camera->processMouseEvents( tickParams->window );
    } );
}

void SampleGame_Small::update( )
{
    camera->processKeyboardEvents( world->getGLFWwindow( ) );
    camera->processMouseEvents( world->getGLFWwindow( ) );
}

void SampleGame_Small::dispose( )
{

}

}