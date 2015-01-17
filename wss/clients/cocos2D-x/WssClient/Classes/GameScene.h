#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include <vector>

#include "cocos2d.h"
#include "Network/Client.h"

class HelloWorld : public cocos2d::Layer
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);

    void update(float dt);


private:
    cocos2d::TMXTiledMap *_tileMap;
    cocos2d::TMXLayer *_background;

    cocos2d::Sprite* _cat;
    cocos2d::Vec2 _catWorld;
    cocos2d::Node* _originNode;
    cocos2d::Node* _camera;
    size_t _camOnIndex = 0;


    std::vector<cocos2d::Sprite*> _entities;

    cocos2d::Vec2 _centerOfScreen;

    cocos2d::Vec2 _screenOrigin;

    WssClient::NetworkClient _client;
};

#endif // __HELLOWORLD_SCENE_H__
