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

	_centerOfScreen = origin + Vec2(visibleSize.width / 2, -visibleSize.height / 2);

	_screenOrigin = Vec2(visibleSize.width, visibleSize.height);



	/////////////////////////////
	// 3. add your codes below...

	// add a label shows "Hello World"
	// create and initialize a label

	_camera = Node::create();
	_tileMap = TMXTiledMap::create("graphics/DawnLike_3/Examples/test3.tmx");
	cout << "Tilemap size: " << _tileMap->getMapSize().width << endl;
	_tileMap->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	//_tileMap->setAnchorPoint(Vec2(0, _tileMap->getMapSize().height * 16.0));
	// _tileMap = TMXTiledMap::createWithXML("test.json", "graphics/DawnLike_3/Examples/");
	//addChild(_tileMap, 1);
	//_camera->addChild(_tileMap, 1);
	//_tileMap->setPosition(Vec2(0 * 16, -50 * 16));
	_tileMap->setPosition(0, 0);
	addChild(_tileMap, 1);
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

	addChild(_cat, 2);



	for (size_t i = 0; i < 1000; ++i) {
		auto sprite = Sprite::create("graphics/DawnLike_3/Characters/Cat0.png", Rect((i % 4) * 16, 0, 16, 16));
		sprite->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);

		sprite->setPosition(Vec2(50*16, 50*16));
		_entities.push_back(sprite);
		_positions.push_back(Vec2(50*16, 50*16));
		//_camera->addChild(sprite, 2);
		addChild(sprite, 2);
	}

	addChild(_camera, 2);

	auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);

	// position the label on the center of the screen
	label->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height - label->getContentSize().height));

	// add the label as a child to this layer
	//this->addChild(label, 3);

	// Network stuff
	_client.connect();
	this->scheduleUpdate();
	return true;
}

void HelloWorld::update(float dt) {
	using namespace std;
	// Translate camera according to cat! CATPOWER!
	//const Vec2 position = _cat->getPosition();



	//_camera->setPosition(-_catWorld + _centerOfScreen);



	// Transform cat
	Vec2 catWorldPosition = Vec2(_catWorld.x, -_catWorld.y);// Transform cat into "camera" origin (0,0), then place that origin at TOP of screen.

	_cat->setPosition(Vec2(_centerOfScreen.x, -_centerOfScreen.y));


	_tileMap->setPosition(-catWorldPosition + _screenOrigin + Vec2(-_centerOfScreen.x, _centerOfScreen.y));

	//cout << "screenPosition: " << _tileMap->getPosition().y << endl;

	Vec2 worldToCamera = -catWorldPosition + _screenOrigin + Vec2(-_centerOfScreen.x, _centerOfScreen.y);
	_client.updatePositions(_entities.size(), _positions);

	for (size_t i = 0; i < _entities.size(); ++i) {
		auto position = _positions[i];
		position = Vec2(position.x, -position.y);
		_entities[i]->setPosition(position + worldToCamera);
	}

	//_tileMap->setPosition(-_catWorld);

	//_cat->setPosition(_centerOfScreen);



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
