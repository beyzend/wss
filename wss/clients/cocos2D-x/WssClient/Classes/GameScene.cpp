#include "GameScene.h"

#include <iostream>

USING_NS_CC;

Scene* HelloWorld::createScene() {
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
bool HelloWorld::init() {
	using namespace cocos2d;
	using namespace std;
	//////////////////////////////
	// 1. super init first
	if (!Layer::init()) {
		return false;
	}

	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	/////////////////////////////
	// 2. add a menu item with "X" image, which is clicked to quit the program
	//    you may modify it.

	// add a "close" icon to exit the progress. it's an autorelease object
	auto closeItem = MenuItemImage::create("CloseNormal.png", "CloseSelected.png", CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width / 2, origin.y + closeItem->getContentSize().height / 2));

	// create menu, it's an autorelease object
	auto menu = Menu::create(closeItem, NULL);
	menu->setPosition(Vec2::ZERO);
	this->addChild(menu, 1);

	_centerOfScreen = origin + Vec2(visibleSize.width / 2, visibleSize.height / 2);

	_screenOrigin = Vec2(visibleSize.width, visibleSize.height);



	/////////////////////////////
	// 3. add your codes below...

	// add a label shows "Hello World"
	// create and initialize a label

	_camera = Node::create();
	_originNode = Node::create();

	_tileMap = TMXTiledMap::create("graphics/DawnLike_3/Examples/test3.tmx");
	_tileMap->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	_tileMap->setPosition(0, 0);
	_originNode->addChild(_tileMap, 1);


	// Keyboard stuff
	auto listener = EventListenerKeyboard::create();
	listener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event) {
		Vec2 offset;
		if (keyCode == EventKeyboard::KeyCode::KEY_A) {
			offset = Vec2(-16.0, 0.0);
		}
		else if(keyCode == EventKeyboard::KeyCode::KEY_D) {
			offset = Vec2(16.0, 0.0);
		}
		else if(keyCode == EventKeyboard::KeyCode::KEY_W) {
			offset = Vec2(0.0, -16.0);
		}
		else if(keyCode == EventKeyboard::KeyCode::KEY_S) {
			 offset = Vec2(0.0, 16.0);
		}
		else if(keyCode == EventKeyboard::KeyCode::KEY_COMMA) {
			_camOnIndex = --_camOnIndex % 1000;
		}
		else if(keyCode == EventKeyboard::KeyCode::KEY_PERIOD) {
			_camOnIndex = ++_camOnIndex % 1000;
		}
		Vec2 origin = Director::getInstance()->getVisibleOrigin();
		Size visibleSize = Director::getInstance()->getVisibleSize();
		Vec2 centerScreen = origin + Vec2(visibleSize.width / 2, visibleSize.height / 2);
		_catWorld = _catWorld + offset;
		//_cat->setPosition(_cat->getPosition() + offset);
	};

	listener->onKeyReleased = [&](EventKeyboard::KeyCode keyCode, Event* event) {
		if (keyCode == EventKeyboard::KeyCode::KEY_A || keyCode == EventKeyboard::KeyCode::KEY_D) {
		}
	};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	// Create some character sprites
	_cat = Sprite::create("graphics/DawnLike_3/Characters/Cat0.png", Rect(2 * 16, 0, 16, 16));
	_cat->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	_catWorld = Vec2(0, 25 * 16);
	addChild(_cat, 2);

	for (size_t i = 0; i < 1000; ++i) {
		auto sprite = Sprite::create("graphics/DawnLike_3/Characters/Cat0.png", Rect((i % 4) * 16, 0, 16, 16));
		sprite->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);

		sprite->setPosition(Vec2(50*16, -50*16));
		_entities.push_back(sprite);
		_originNode->addChild(sprite, 2);
	}

	_camOnIndex = 50;

	_camera->addChild(_originNode, 1);
	addChild(_camera, 1);
	_originNode->setPosition(_screenOrigin);

	Vec2 catWorldPosition = Vec2(_catWorld.x, -_catWorld.y);

	_camera->setPosition(-catWorldPosition - _centerOfScreen);

	auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);

	// position the label on the center of the screen
	label->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height - label->getContentSize().height));

	// Network stuff
	_client.connect();
	this->scheduleUpdate();
	return true;
}

void HelloWorld::update(float dt) {
	using namespace std;
	_client.updatePositions(_entities);

	Vec2 catWorldPosition = Vec2(_catWorld.x, -_catWorld.y);

	_cat->setPosition(_centerOfScreen);

	// Transform camera based on cat
	_camera->setPosition(-catWorldPosition - _centerOfScreen);

	return;
}

void HelloWorld::menuCloseCallback(Ref* pSender) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
	return;
#endif

	Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	exit(0);
#endif
}
