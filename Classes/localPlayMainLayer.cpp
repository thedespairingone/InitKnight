#include "localPlayMainLayer.h"

USING_NS_CC;

// on "init" you need to initialize your instance
bool localPlay::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
		return false;

	statusLayer = nullptr;

	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	_tileMap = TMXTiledMap::create("testMap/bigLocalMap.tmx");
	addChild(_tileMap, 0, 100);

	TMXObjectGroup* group = _tileMap->getObjectGroup("player");
	ValueMap spawnPoint = group->getObject("startPoint");

	auto x = spawnPoint["x"].asFloat();
	auto y = spawnPoint["y"].asFloat();

	hero = Hero::create();
	hero->initHeroSprite();
	hero->setPosition(Vec2(x, y));
	addChild(hero, 2, 200);

	setViewpointCenter(hero->getPosition());
	//��ȡ��ͼ�Ĳ�ͬ��
	_collidable = _tileMap->getLayer("barriers");
	_collidable->setVisible(false);

	_heart = _tileMap->getLayer("heart");
	_heart->setVisible(true);
	
	_npc = _tileMap->getLayer("npc");
	_npc->setVisible(true);
	this->scheduleUpdate();

	return true;
}

void localPlay::getStatusLayer(localStatus* tLayer)
{
	if (tLayer == nullptr)
		cocos2d::log("fuck status layer!!");
	else
		cocos2d::log("status layer tag %d", tLayer->getTag());
	statusLayer = tLayer;
}
cocos2d::EventKeyboard::KeyCode localPlay::whichPressed()
{
	if (keyStatus[EventKeyboard::KeyCode::KEY_W])
		return EventKeyboard::KeyCode::KEY_W;
	if (keyStatus[EventKeyboard::KeyCode::KEY_A])
		return EventKeyboard::KeyCode::KEY_A;
	if (keyStatus[EventKeyboard::KeyCode::KEY_S])
		return EventKeyboard::KeyCode::KEY_S;
	if (keyStatus[EventKeyboard::KeyCode::KEY_D])
		return EventKeyboard::KeyCode::KEY_D;
	if (keyStatus[EventKeyboard::KeyCode::KEY_Q])
		return EventKeyboard::KeyCode::KEY_Q;
	if (keyStatus[EventKeyboard::KeyCode::KEY_J])
		return EventKeyboard::KeyCode::KEY_J;
	if (keyStatus[EventKeyboard::KeyCode::KEY_SPACE])
		return EventKeyboard::KeyCode::KEY_SPACE;
	return EventKeyboard::KeyCode::KEY_NONE;
}

void localPlay::update(float delta)
{
	Node::update(delta);
	EventKeyboard::KeyCode pressedKey = whichPressed();
	if (pressedKey != EventKeyboard::KeyCode::KEY_NONE)
	{
		onPress(pressedKey);
	}

}

void localPlay::onPress(EventKeyboard::KeyCode keyCode)
{
	Vec2 playerPos = hero->getPosition();
	const int movSpeed = hero->getMoveSpeed();
	switch (keyCode)
	{
	case EventKeyboard::KeyCode::KEY_W:
		hero->setRunAnimation();
		playerPos.y += movSpeed;
		break;
	case EventKeyboard::KeyCode::KEY_A:
		hero->setRunAnimation(1);
		playerPos.x -= movSpeed;
		break;
	case EventKeyboard::KeyCode::KEY_S:
		hero->setRunAnimation();
		playerPos.y -= movSpeed;
		break;
	case EventKeyboard::KeyCode::KEY_D:
		hero->setRunAnimation(0);
		playerPos.x += movSpeed;
		break;
	case EventKeyboard::KeyCode::KEY_J:
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sword.wav");
		hero->setAttackAnimation();
		break;
	default:
		break;
	}
	this->setPlayerPosition(playerPos);
}

bool localPlay::detectPlayerPosition(Vec2 position)
{
	//�����ص�����ת��Ϊ��Ƭ����
	Vec2 tileCoord = this->tileCoordFromPosition(position);
	//�����Ƭ��GID
	int tileGid = _collidable->getTileGIDAt(tileCoord);

	if (tileGid > 0)
	{
		Value prop = _tileMap->getPropertiesForGID(tileGid);
		ValueMap propValueMap = prop.asValueMap();

		std::string collision = propValueMap["isCollidable"].asString();

		if (collision == "true") { //��ײ���ɹ�
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/empty.wav");
			return false;
		}
	}
	tileGid = _heart->getTileGIDAt(tileCoord);
	if (tileGid > 0)
	{
		Value prop = _tileMap->getPropertiesForGID(tileGid);
		ValueMap propValueMap = prop.asValueMap();

		std::string collect = propValueMap["isCollectable"].asString();

		if (collect == "true") { //��ȡ���ɹ�
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/getHeart.mp3");
			_heart->removeTileAt(tileCoord);
			hero->setUpAnimation();
			statusLayer->addHeroBlood(15.0f);
			statusLayer->addPoint(30);
			return false;
		}
	}

	tileGid = _npc->getTileGIDAt(tileCoord);
	if (tileGid > 0)
	{
		Value prop = _tileMap->getPropertiesForGID(tileGid);
		ValueMap propValueMap = prop.asValueMap();

		std::string talk = propValueMap["isTalktoOld"].asString();

		if (talk == "true") { //����npc�Ի����ɹ�
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/getHeart.mp3");
			_npc->removeTileAt(tileCoord);
			statusLayer->showOldNPCDialog();
			auto keyboardListenerTalkOld = EventListenerKeyboard::create();
			//ʹ��Lambda����ʽ����npc�Ի������¼�
			keyboardListenerTalkOld->onKeyPressed = [this, keyboardListenerTalkOld](EventKeyboard::KeyCode keyCode, Event* event)
			{
				switch (keyCode)
				{
				case EventKeyboard::KeyCode::KEY_Y:
					this->statusLayer->addHeroBlood(30);
					//���̼���
					Director::getInstance()->getEventDispatcher()->removeEventListener(keyboardListenerTalkOld);
					this->statusLayer->removeOldNPCDialog();
					break;
				case EventKeyboard::KeyCode::KEY_X:
					hero->setMoveSpeed(hero->getMoveSpeed() * 2);
					Director::getInstance()->getEventDispatcher()->removeEventListener(keyboardListenerTalkOld);
					this->statusLayer->removeOldNPCDialog();
					break;
				default:
					break;
				}
			};
			EventDispatcher *eventDispatcher = Director::getInstance()->getEventDispatcher();
			eventDispatcher->addEventListenerWithFixedPriority(keyboardListenerTalkOld, 1);
			return false;
		}

		talk = propValueMap["isTalktoPao"].asString();

		if (talk == "true") { //��������npc�Ի����ɹ�
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/getHeart.mp3");
			_npc->removeTileAt(tileCoord);
			statusLayer->showPaojieDialog();
			auto keyboardListenerTalkOld = EventListenerKeyboard::create();
			//ʹ��Lambda����ʽ����npc�Ի������¼�
			keyboardListenerTalkOld->onKeyPressed = [this, keyboardListenerTalkOld](EventKeyboard::KeyCode keyCode, Event* event)
			{
				switch (keyCode)
				{
				case EventKeyboard::KeyCode::KEY_Y:
					//��ȡ����
					//���̼���
					Director::getInstance()->getEventDispatcher()->removeEventListener(keyboardListenerTalkOld);
					this->statusLayer->removePaoNPCDialog();
					break;
				case EventKeyboard::KeyCode::KEY_X:
					this->statusLayer->cutHeroBlood(40);
					Director::getInstance()->getEventDispatcher()->removeEventListener(keyboardListenerTalkOld);
					this->statusLayer->removePaoNPCDialog();
					break;
				default:
					break;
				}
			};
			EventDispatcher *eventDispatcher = Director::getInstance()->getEventDispatcher();
			eventDispatcher->addEventListenerWithFixedPriority(keyboardListenerTalkOld, 1);
			return false;
		}
	}
	return true;
}
void localPlay::setPlayerPosition(Vec2 position)
{
	if (detectPlayerPosition(position))
	{
		//�ƶ�����
		hero->setPosition(position);
		//������ͼ
		this->setViewpointCenter(hero->getPosition());
	}
}

Vec2 localPlay::tileCoordFromPosition(Vec2 pos)
{
	int x = pos.x / _tileMap->getTileSize().width;
	int y = ((_tileMap->getMapSize().height * _tileMap->getTileSize().height) - pos.y) 
		/ _tileMap->getTileSize().height;
	return Vec2(x, y);
}

void localPlay::setViewpointCenter(Vec2 position)
{

	Size visibleSize = Director::getInstance()->getVisibleSize();
	//���Է�ֹ����ͼ��߳�����Ļ֮�⡣
	int x = MAX(position.x, visibleSize.width / 2);
	int y = MAX(position.y, visibleSize.height / 2);
	//���Է�ֹ����ͼ�ұ߳�����Ļ֮�⡣
	x = MIN(x, (_tileMap->getMapSize().width * _tileMap->getTileSize().width)
		- visibleSize.width / 2);
	y = MIN(y, (_tileMap->getMapSize().height * _tileMap->getTileSize().height)
		- visibleSize.height / 2);

	//��Ļ���ĵ�
	Vec2 pointA = Vec2(visibleSize.width / 2, visibleSize.height / 2);
	//ʹ���鴦����Ļ���ģ��ƶ���ͼĿ��λ��
	Vec2 pointB = Vec2(x, y);

	//��ͼ�ƶ�ƫ����
	Vec2 offset = pointA - pointB;
	this->setPosition(offset);
}

void localPlay::onEnter()
{
	Layer::onEnter();
	auto keyboardListener = EventListenerKeyboard::create();
	//ʹ��Lambda����ʽ���������¼�
	//����this��ʹ�ó�Ա����
	keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event)
	{
		this->keyStatus[keyCode] = true;
		log("%d pressed", keyCode);
	};

	//�����ͷ�
	keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, Event *event)
	{
		this->keyStatus[keyCode] = false;
		hero->stopAllAnimation();
		log("%d released", keyCode);
	};

	EventDispatcher *eventDispatcher = Director::getInstance()->getEventDispatcher();
	eventDispatcher->addEventListenerWithFixedPriority(keyboardListener, 2);

}