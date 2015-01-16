#include "GameScene.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
	using namespace cocos2d;
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);



   _tileMap = TMXTiledMap::create("graphics/DawnLike_3/Examples/test3.tmx");
   // _tileMap = TMXTiledMap::createWithXML("test.json", "graphics/DawnLike_3/Examples/");
    addChild(_tileMap, 1);

    _tileMap->setPosition(Vec2(0*16,-50*16));
    // Keyboard stuff
    auto listener = EventListenerKeyboard::create();
    listener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event) {
    	if (keyCode == EventKeyboard::KeyCode::KEY_A) {
    		Vec2 offset = Vec2(20.0, 0.0);
    		_tileMap->setPosition(_tileMap->getPosition() + offset);
    	}
    	else if(keyCode == EventKeyboard::KeyCode::KEY_D) {
    		Vec2 offset = Vec2(-20.0, 0.0);
    		_tileMap->setPosition(_tileMap->getPosition() + offset);
    	}
    	else if(keyCode == EventKeyboard::KeyCode::KEY_W) {
    		Vec2 offset = Vec2(0.0, -20.0);
    		_tileMap->setPosition(_tileMap->getPosition() + offset);
    	}
    	else if(keyCode == EventKeyboard::KeyCode::KEY_S) {
    		Vec2 offset = Vec2(0.0, 20.0);
    		_tileMap->setPosition(_tileMap->getPosition() + offset);
    	}
    };

    listener->onKeyReleased = [&](EventKeyboard::KeyCode keyCode, Event* event) {
    	if (keyCode == EventKeyboard::KeyCode::KEY_A || keyCode == EventKeyboard::KeyCode::KEY_D) {
    		offset = Vec2(0.0, 0.0);
    	}
    };


    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);



    // add "HelloWorld" splash screen"
    //auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    //sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
    //this->addChild(sprite, 0);
    
    return true;
}

void HelloWorld::update(float dt) {

	const Vec2 position = _tileMap->getPosition();
	_tileMap->setPosition(position + offset * dt);

	return;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
