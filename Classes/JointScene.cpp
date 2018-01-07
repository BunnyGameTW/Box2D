#include "JointScene.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

#define MAX_2(X,Y) (X)>(Y) ? (X) : (Y)
#ifndef MAX_CIRCLE_OBJECTS
#define MAX_CIRCLE_OBJECTS  11
#endif
extern char g_CircleObject[MAX_CIRCLE_OBJECTS][20];

#define StaticAndDynamicBodyExample 1
using namespace cocostudio::timeline;

JointScene::~JointScene()
{

#ifdef BOX2D_DEBUG
	if (_DebugDraw != NULL) delete _DebugDraw;
#endif

	if (_b2World != nullptr) delete _b2World;
//  for releasing Plist&Texture
//	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	Director::getInstance()->getTextureCache()->removeUnusedTextures();

}

Scene* JointScene::createScene()
{
    auto scene = Scene::create();
    auto layer = JointScene::create();
    scene->addChild(layer);
    return scene;
}

// on "init" you need to initialize your instance
bool JointScene::init()
{   
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }

//  For Loading Plist+Texture
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");

	_visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
	//���D : ��ܥثe BOX2D �Ҥ��Ъ��\��
	_titleLabel = Label::createWithTTF("Joint Scene", "fonts/Marker Felt.ttf", 32);
	_titleLabel->setPosition(_titleLabel->getContentSize().width*0.5f + 25.f, _visibleSize.height - _titleLabel->getContentSize().height*0.5f - 5.f);
	this->addChild(_titleLabel, 2);

	// �إ� Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//���O��V
	bool AllowSleep = true;			//���\�ε�
	_b2World = new b2World(Gravity);	//�Ыإ@��
	_b2World->SetAllowSleeping(AllowSleep);	//�]�w���󤹳\�ε�

											// Create Scene with csb file
	_csbRoot = CSLoader::createNode("JointScene.csb");
#ifndef BOX2D_DEBUG
	// �]�w��ܭI���ϥ�
	auto bgSprite = _csbRoot->getChildByName("bg64_1");
	bgSprite->setVisible(true);

#endif
	addChild(_csbRoot, 1);

	createStaticBoundary();
	//setupMouseJoint();
	/*setupDistanceJoint();
	setupPrismaticJoint();
	setupPulleyJoint();*/
	setupGearJoint();
	setupWeldJoint();
	setupRopeJoint();
	setupCar();//car
	setupBall();
	setupSensor();
	setupBallbtn();
	setupTribtn();
	ballNum =ballNum2= 0;
	setupDoor();
	setupTri();
	thirdPart = false;
	//brush
	brush_i = 0;
	_iFree = 0;
	_iInUsed = 0;
	_boxuse = 0;
	for (int i = 0; i < NUMBER_BRUSH; i++) {
		_brush[i] = (Sprite*)Sprite::createWithSpriteFrameName("orange02.png");; // �]�w�ò��� CParticle ���������e
		_brush[i]->setScale(0.3f);
		_brush[i]->setVisible(false);
		_brush[i]->setPosition(_visibleSize.width / 2, _visibleSize.height / 2);
		_csbRoot->addChild(_brush[i]);
		_FreeList.push_front(_brush[i]); // �ǤJ���ȡA�]�N�O�� i �Ӥ��l������
		_iFree++;
	}
	
#ifdef BOX2D_DEBUG
	//DebugDrawInit
	_DebugDraw = nullptr;
	_DebugDraw = new GLESDebugDraw(PTM_RATIO);
	//�]�wDebugDraw
	_b2World->SetDebugDraw(_DebugDraw);
	//���ø�s���O
	uint32 flags = 0;
	flags += GLESDebugDraw::e_shapeBit;						//ø�s�Ϊ�
	flags += GLESDebugDraw::e_pairBit;						
	flags += GLESDebugDraw::e_jointBit;
	flags += GLESDebugDraw::e_centerOfMassBit;
	flags += GLESDebugDraw::e_aabbBit;
	//�]�wø�s����
	_DebugDraw->SetFlags(flags);
#endif

	_listener1 = EventListenerTouchOneByOne::create();	//�Ыؤ@�Ӥ@��@���ƥ��ť��
	_listener1->onTouchBegan = CC_CALLBACK_2(JointScene::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	_listener1->onTouchMoved = CC_CALLBACK_2(JointScene::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	_listener1->onTouchEnded = CC_CALLBACK_2(JointScene::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(JointScene::doStep));

    return true;
}

void JointScene::setupMouseJoint()
{
	// ���o�ó]�w frame01 �e�عϥܬ��ʺA����
	auto frameSprite = _csbRoot->getChildByName("frame01");
	Point loc = frameSprite->getPosition();

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	bodyDef.userData = frameSprite;
	b2Body *body = _b2World->CreateBody(&bodyDef);

	// Define poly shape for our dynamic body.
	b2PolygonShape rectShape;
	Size frameSize = frameSprite->getContentSize();
	rectShape.SetAsBox((frameSize.width - 4)*0.5f / PTM_RATIO, (frameSize.height - 4)*0.5f / PTM_RATIO);
	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &rectShape;
	fixtureDef.restitution = 0.1f;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.1f;
	body->CreateFixture(&fixtureDef);
	_bTouchOn = false;
}


void JointScene::setupDistanceJoint()
{
	// ���o�ó]�w frame01_dist �e�عϥܬ��i�R�A����j
	auto frameSprite = _csbRoot->getChildByName("frame01_dist");
	Point loc = frameSprite->getPosition();
	Size size = frameSprite->getContentSize();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = frameSprite;
	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width*0.5f / PTM_RATIO, size.height*0.5f / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	staticBody->CreateFixture(&fixtureDef);

	// ���o�ó]�w circle01 ���i�ʺA����j
	auto circleSprite = _csbRoot->getChildByName("circle01_dist");
	loc = circleSprite->getPosition();
	size = circleSprite->getContentSize();
	b2CircleShape circleShape;
	circleShape.m_radius = size.width*0.5f / PTM_RATIO;

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData = circleSprite;
	b2Body* dynamicBody = _b2World->CreateBody(&dynamicBodyDef);
	fixtureDef.shape = &circleShape;
	dynamicBody->CreateFixture(&fixtureDef);

	//���ͶZ�����`
	b2DistanceJointDef JointDef;
	JointDef.Initialize(staticBody, dynamicBody, staticBody->GetPosition(), dynamicBody->GetPosition());
	_b2World->CreateJoint(&JointDef);
}

void JointScene::setupPrismaticJoint()
{
	// ���o�ó]�w frame01_pri �e�عϥܬ��i�R�A����j
	auto frameSprite = _csbRoot->getChildByName("frame01_pri");
	Point loc = frameSprite->getPosition();
	Size size = frameSprite->getContentSize();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = frameSprite;
	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);
	b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width*0.5f / PTM_RATIO, size.height*0.5f / PTM_RATIO);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	staticBody->CreateFixture(&fixtureDef);

	// ���o�ó]�w circle01_pri ���i�ʺA����j
	auto circleSprite = _csbRoot->getChildByName("circle01_pri");
	loc = circleSprite->getPosition();
	size = circleSprite->getContentSize();
	float scale = circleSprite->getScale();
	b2CircleShape circleShape;
	circleShape.m_radius = size.width*0.5f*scale / PTM_RATIO;

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData = circleSprite;
	b2Body* dynamicBody = _b2World->CreateBody(&dynamicBodyDef);
	fixtureDef.shape = &circleShape;
	dynamicBody->CreateFixture(&fixtureDef);

	//���ͫ��� Prismatic Joint
	b2PrismaticJointDef JointDef;
	JointDef.Initialize(staticBody, dynamicBody, staticBody->GetPosition(), b2Vec2(1.0f / PTM_RATIO, 0));
	_b2World->CreateJoint(&JointDef);

	// ���o�ó]�w circle02_pri ���i�ʺA����j
	circleSprite = _csbRoot->getChildByName("circle02_pri");
	loc = circleSprite->getPosition();
	size = circleSprite->getContentSize();
	scale = circleSprite->getScale();
	circleShape.m_radius = size.width*0.5f*scale / PTM_RATIO;

	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData = circleSprite;
	dynamicBody = _b2World->CreateBody(&dynamicBodyDef);
	fixtureDef.shape = &circleShape;
	dynamicBody->CreateFixture(&fixtureDef);

	//���ͤ��� Prismatic Joint
	JointDef;
	JointDef.Initialize(staticBody, dynamicBody, staticBody->GetPosition(), b2Vec2(0, 1.0f / PTM_RATIO));
	_b2World->CreateJoint(&JointDef);
}

// �ƽ����`
void JointScene::setupPulleyJoint()
{
	// ���o�ó]�w circle01_pul ���i�ʺA����A�j
	auto circleSprite = _csbRoot->getChildByName("circle01_pul");
	Point locA = circleSprite->getPosition();
	Size size = circleSprite->getContentSize();
	float scale = circleSprite->getScale();
	b2CircleShape circleShape;
	circleShape.m_radius = size.width*0.5f*scale / PTM_RATIO;

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(locA.x / PTM_RATIO, locA.y / PTM_RATIO);
	bodyDef.userData = circleSprite;
	b2Body* bodyA = _b2World->CreateBody(&bodyDef);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 10.0f;
	fixtureDef.friction = 0.2f;
	bodyA->CreateFixture(&fixtureDef);

	// ���o�ó]�w circle02_pul ���i�ʺA����B�j
	circleSprite = _csbRoot->getChildByName("circle02_pul");
	Point locB = circleSprite->getPosition();
	size = circleSprite->getContentSize();
	scale = circleSprite->getScale();
	circleShape.m_radius = size.width*0.5f*scale / PTM_RATIO;

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(locB.x / PTM_RATIO, locB.y / PTM_RATIO);
	bodyDef.userData = circleSprite;
	b2Body* bodyB = _b2World->CreateBody(&bodyDef);
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 10.0f;
	fixtureDef.friction = 0.2f;
	bodyB->CreateFixture(&fixtureDef);

	//���ͷƽ����`
	b2PulleyJointDef JointDef;
	JointDef.Initialize(bodyA, bodyB,
		b2Vec2( locA.x / PTM_RATIO, (locA.y +150) / PTM_RATIO),
		b2Vec2( locB.x / PTM_RATIO, (locB.y +150) / PTM_RATIO),
		bodyA->GetWorldCenter(),
		bodyB->GetWorldCenter(),
		1);
	_b2World->CreateJoint(&JointDef);
}

void JointScene::setupGearJoint()
{
	// ��������2����ܪ��i��Ρj�R�A����A�H�T�w�i�H��ʪ�2�ӰʺA����
	char tmp[20] = "";
	Sprite *gearSprite[4];
	Point loc[4];
	Size  size[4];
	float scale[4];
	b2Body* staticBody[4];
	//b2Body* dynamicBody[2];
	b2RevoluteJoint*  RevJoint[3];
	b2PrismaticJoint* PriJoint;//�
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.userData = NULL;

	b2CircleShape staticShape;
	staticShape.m_radius = 5 / PTM_RATIO;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &staticShape;
	float rectRot;
	for (int i = 0; i <  4; i++)
	{
		sprintf(tmp, "gear01_%02d", i+1);
		gearSprite[i] = (Sprite *)_csbRoot->getChildByName(tmp);
		loc[i] = gearSprite[i]->getPosition();
		size[i] = gearSprite[i]->getContentSize();
		scale[i] = gearSprite[i]->getScale();
		//
		staticBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		staticBody[i] = _b2World->CreateBody(&staticBodyDef);
		staticBody[i]->CreateFixture(&fixtureDef);
	}
	
	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	b2CircleShape circleShape;	
	b2PolygonShape polyShape;//�����
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.25f;
	
	for (int i = 0; i < 4; i++)
	{
		if (i < 3) circleShape.m_radius = (size[i].width - 4) * 0.5f * scale[i] / PTM_RATIO;
		else {
			float sx = gearSprite[i]->getScaleX();
			float sy = gearSprite[i]->getScaleY();
			fixtureDef.shape = &polyShape;
			polyShape.SetAsBox((size[i].width - 4) *0.5f *sx / PTM_RATIO, (size[i].height - 4) *0.5f *sy / PTM_RATIO);
		}
		dynamicBodyDef.userData = gearSprite[i];
		dynamicBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		if (i == 3)dynamicBodyDef.angle =49.5;
		dynamicBody[i] = _b2World->CreateBody(&dynamicBodyDef);
		dynamicBody[i]->CreateFixture(&fixtureDef);
	}
	b2PrismaticJointDef PrJoint; // �������`
	b2RevoluteJointDef RJoint;	// �������`
	for (int i = 0; i < 4; i++)
	{
			if (i < 3) {
				RJoint.Initialize(staticBody[i], dynamicBody[i], dynamicBody[i]->GetWorldCenter());
				RevJoint[i] = (b2RevoluteJoint*)_b2World->CreateJoint(&RJoint);
			}
			else {
				PrJoint.Initialize(staticBody[i], dynamicBody[i], dynamicBody[i]->GetWorldCenter(), b2Vec2(1.0f, 1.0f));
				PriJoint = (b2PrismaticJoint*)_b2World->CreateJoint(&PrJoint);
			}
	}
	b2GearJointDef GrecJoint;
	GrecJoint.bodyA = dynamicBody[2];
	GrecJoint.bodyB = dynamicBody[3];
	GrecJoint.joint1 = RevJoint[2];
	GrecJoint.joint2 = PriJoint;
	GrecJoint.ratio = -1;
	_b2World->CreateJoint(&GrecJoint);
	//tooth
	//���o�ó]�wtooth ���i�ʺA����j
	Sprite *toothSprite[16];
	Point toothLoc[16];
	Size  toothSize[16];
	float toothScale[16];
	float toothRot[16];
	b2BodyDef dynamicBodyDefTooth;
	//b2Body* dynamicBodyTooth[16];
	dynamicBodyDefTooth.type = b2_dynamicBody;
	for (int i = 0; i < 16; i++)
	{
		sprintf(tmp, "tooth_%02d", i + 1);
		toothSprite[i] = (Sprite *)_csbRoot->getChildByName(tmp);

		toothLoc[i] = toothSprite[i]->getPosition();
		toothSize[i] = toothSprite[i]->getContentSize();
		toothRot[i]=-MATH_DEG_TO_RAD( toothSprite[i] -> getRotation());
		toothScale[i] = toothSprite[i]->getScale();
		dynamicBodyDefTooth.position.Set(toothLoc[i].x / PTM_RATIO, toothLoc[i].y / PTM_RATIO);
		//dynamicBodyDefTooth.angle = toothRot[i];
		dynamicBodyDefTooth.angle= toothRot[i];
		dynamicBodyDefTooth.userData = toothSprite[i];
		dynamicBodyTooth[i] = _b2World->CreateBody(&dynamicBodyDefTooth);
	}
	b2PolygonShape polyShape1;
	b2RevoluteJointDef RJointTooth;	// �������`
	b2RevoluteJoint* RevJointTooth;
	for (int i = 0; i < 16; i++)
	{
		float sx = toothSprite[i]->getScaleX();
		float sy = toothSprite[i]->getScaleY();
		fixtureDef.shape = &polyShape1;
		polyShape1.SetAsBox((toothSize[i].width ) *0.5f *sx / PTM_RATIO, (toothSize[i].height) *0.5f *sy / PTM_RATIO);
		
		fixtureDef.density = 1.0f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 1.0f;
		dynamicBodyTooth[i]->CreateFixture(&fixtureDef);
		if(i<8)	RJointTooth.Initialize(dynamicBody[0], dynamicBodyTooth[i], dynamicBody[0]->GetWorldCenter());
		else RJointTooth.Initialize(dynamicBody[1], dynamicBodyTooth[i], dynamicBody[1]->GetWorldCenter());
		RevJointTooth = (b2RevoluteJoint*)_b2World->CreateJoint(&RJointTooth);
	}	
	b2RevoluteJointDef revJoint;
	revJoint.bodyA = dynamicBody[0];
	revJoint.localAnchorA.Set(0,1);
	revJoint.bodyB = dynamicBodyTooth[0];
	revJoint.localAnchorB.Set(0, 0);
	_b2World->CreateJoint(&revJoint);

	//revJoint.bodyA = dynamicBody[0];
	revJoint.localAnchorA.Set(0.707, 0.707);
	revJoint.bodyB = dynamicBodyTooth[1];
	//revJoint.localAnchorB.Set(0, 0);
	_b2World->CreateJoint(&revJoint);

	//revJoint.bodyA = dynamicBody[3];
	revJoint.localAnchorA.Set(1, 0);
	revJoint.bodyB = dynamicBodyTooth[2];
	//revJoint.localAnchorB.Set(0, 0);
	_b2World->CreateJoint(&revJoint);

	//revJoint.bodyA = dynamicBody[3];
	revJoint.localAnchorA.Set(0.707, -0.707);
	revJoint.bodyB = dynamicBodyTooth[3];
	//revJoint.localAnchorB.Set(0, 0);
	_b2World->CreateJoint(&revJoint);

	//revJoint.bodyA = dynamicBody[3];
	revJoint.localAnchorA.Set(0,-1);
	revJoint.bodyB = dynamicBodyTooth[4];
	//revJoint.localAnchorB.Set(0, 0);
	_b2World->CreateJoint(&revJoint);

	//revJoint.bodyA = dynamicBody[3];
	revJoint.localAnchorA.Set(-0.707, -0.707);
	revJoint.bodyB = dynamicBodyTooth[5];
	//revJoint.localAnchorB.Set(0, 0);
	_b2World->CreateJoint(&revJoint);

	//revJoint.bodyA = dynamicBody[3];
	revJoint.localAnchorA.Set(-1,0);
	revJoint.bodyB = dynamicBodyTooth[6];
	//revJoint.localAnchorB.Set(0, 0);
	_b2World->CreateJoint(&revJoint);

	//revJoint.bodyA = dynamicBody[3];
	revJoint.localAnchorA.Set(-0.707, 0.707);
	revJoint.bodyB = dynamicBodyTooth[7];
	//revJoint.localAnchorB.Set(0, 0);
	_b2World->CreateJoint(&revJoint);

	revJoint.bodyA = dynamicBody[1];
	revJoint.localAnchorA.Set(0.38f, 0.92f);
	revJoint.bodyB = dynamicBodyTooth[8];
	_b2World->CreateJoint(&revJoint);

	revJoint.bodyA = dynamicBody[1];
	revJoint.localAnchorA.Set(0.92f, 0.38f);
	revJoint.bodyB = dynamicBodyTooth[9];
	_b2World->CreateJoint(&revJoint);

	revJoint.bodyA = dynamicBody[1];
	revJoint.localAnchorA.Set(0.92f, -0.38f);
	revJoint.bodyB = dynamicBodyTooth[10];
	_b2World->CreateJoint(&revJoint);

	revJoint.bodyA = dynamicBody[1];
	revJoint.localAnchorA.Set(0.38f, -0.92f);
	revJoint.bodyB = dynamicBodyTooth[11];
	_b2World->CreateJoint(&revJoint);

	revJoint.bodyA = dynamicBody[1];
	revJoint.localAnchorA.Set(-0.38f, -0.92f);
	revJoint.bodyB = dynamicBodyTooth[12];
	_b2World->CreateJoint(&revJoint);
	revJoint.bodyA = dynamicBody[1];
	revJoint.localAnchorA.Set(-0.92f, -0.38f);
	revJoint.bodyB = dynamicBodyTooth[13];
	_b2World->CreateJoint(&revJoint);
	revJoint.bodyA = dynamicBody[1];
	revJoint.localAnchorA.Set(-0.92f, 0.38f);
	revJoint.bodyB = dynamicBodyTooth[14];
	_b2World->CreateJoint(&revJoint);
	revJoint.bodyA = dynamicBody[1];
	revJoint.localAnchorA.Set(-0.38f, 0.92f);
	revJoint.bodyB = dynamicBodyTooth[15];
	_b2World->CreateJoint(&revJoint);
	//
	//���;������`(�P�B�ۤ�)
	b2GearJointDef GJoint;
	GJoint.bodyA = dynamicBody[0];
	GJoint.bodyB = dynamicBody[1];
	GJoint.joint1 = RevJoint[0];
	GJoint.joint2 = RevJoint[1];
	GJoint.ratio = 1;
	_b2World->CreateJoint(&GJoint);
	
}

void JointScene::setupWeldJoint()
{
	// ���o�ó]�w frame01_weld ���i�R�A����j
	auto frameSprite = _csbRoot->getChildByName("weldCircle_01");
	Point loc = frameSprite->getPosition();
	Size size = frameSprite->getContentSize();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = frameSprite;
	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);
	
	b2CircleShape circleShape;
	circleShape.m_radius = (size.width - 4)*0.5f / PTM_RATIO;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circleShape;
	staticBody->CreateFixture(&fixtureDef);


	//���o�ó]�w circle01_weld ���i�ʺA����j
	auto circleSprite = _csbRoot->getChildByName("weld_01");
	loc = circleSprite->getPosition();
	size = circleSprite->getContentSize();
	float sx = circleSprite->getScaleX();
	float sy = circleSprite->getScaleY();
	

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData = circleSprite;
	b2Body* dynamicBody1 = _b2World->CreateBody(&dynamicBodyDef);
	b2PolygonShape boxShape;	
	boxShape.SetAsBox((size.width - 4) *0.5f *sx / PTM_RATIO, (size.height - 4) *0.5f *sy / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 1.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	dynamicBody1->CreateFixture(&fixtureDef);

	//���Ͳk�����`(�i���)
	b2WeldJointDef JointDef;
	JointDef.Initialize(staticBody, dynamicBody1, staticBody->GetPosition());
	JointDef.frequencyHz = 1.0f;
	JointDef.dampingRatio = 0.125f;
	_b2World->CreateJoint(&JointDef);
}

void JointScene::setupRopeJoint()
{
	// ���o�ó]�w frame01_rope �e�عϥܬ��i�R�A����j
	auto frameSprite = _csbRoot->getChildByName("frame01_rope");
	Point locHead = frameSprite->getPosition();
	Size sizeHead = frameSprite->getContentSize();

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(locHead.x / PTM_RATIO, locHead.y / PTM_RATIO);
	bodyDef.userData = frameSprite;
	b2Body* ropeHeadBody = _b2World->CreateBody(&bodyDef);
	b2FixtureDef  fixtureDef;
	fixtureDef.density = 1.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	b2PolygonShape boxShape;
	boxShape.SetAsBox(sizeHead.width*0.5f / PTM_RATIO, sizeHead.height*0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	ropeHeadBody->CreateFixture(&fixtureDef);

	//���o�ó]�w circle01_rope ���i�ʺA����j
	auto circleSprite = _csbRoot->getChildByName("circle01_rope");
	Point locTail = circleSprite->getPosition();
	Size sizeTail = circleSprite->getContentSize();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(locTail.x / PTM_RATIO, locTail.y / PTM_RATIO);
	bodyDef.userData = circleSprite;
	b2Body* donutsBody = _b2World->CreateBody(&bodyDef);
	b2CircleShape circleShape;
	circleShape.m_radius = (sizeTail.width - 4)*0.5f / PTM_RATIO;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 10.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 1.0f;
	donutsBody->CreateFixture(&fixtureDef);
	

	//����÷�l���`
	b2RopeJointDef JointDef;
	JointDef.bodyA = ropeHeadBody;
	JointDef.bodyB = donutsBody;
	JointDef.localAnchorA = b2Vec2(0, 0);
	JointDef.localAnchorB = b2Vec2(0, 30.0f / PTM_RATIO);
	JointDef.maxLength = (locHead.y - locTail.y- 30)/PTM_RATIO;
	JointDef.collideConnected = true;
	b2RopeJoint* J = (b2RopeJoint*)_b2World->CreateJoint(&JointDef);

	// �����A�H�u�q�۳s�A
	char tmp[20] = "";
	Sprite *ropeSprite[14];
	Point loc[14];
	Size  size[15];
	b2Body* ropeBody[14];

	bodyDef.type = b2_dynamicBody;
	// �]���O÷�l�ҥH���q���n�ӭ�
	fixtureDef.density = 0.01f;  fixtureDef.friction = 1.0f; fixtureDef.restitution =0.0f;
	fixtureDef.shape = &boxShape;
	// ���ͤ@�t�C��÷�l�q�� rope01_01 ~ rope01_15�A�P�ɱ��_��
	for (int i = 0; i < 14; i++)
	{
		sprintf(tmp, "rope01_%02d", i + 1);
		ropeSprite[i] = (Sprite *)_csbRoot->getChildByName(tmp);
		loc[i] = ropeSprite[i]->getPosition();
		size[i] = ropeSprite[i]->getContentSize();

		bodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		bodyDef.userData = ropeSprite[i];
		ropeBody[i] = _b2World->CreateBody(&bodyDef);
		boxShape.SetAsBox((size[i].width-4)*0.5f / PTM_RATIO, (size[i].height-4)*0.5f / PTM_RATIO);
		ropeBody[i]->CreateFixture(&fixtureDef);
	}
	// �Q�� RevoluteJoint �N�u�q�����s���b�@�_
	// ���s�� ropeHeadBody �P  ropeBody[0]

	float locAnchor = 0.5f*(size[0].height - 5) / PTM_RATIO;
	b2RevoluteJointDef revJoint;
	revJoint.bodyA = ropeHeadBody;
	revJoint.localAnchorA.Set(0, -0.9f);
	revJoint.bodyB = ropeBody[0];
	revJoint.localAnchorB.Set(0, locAnchor);
	_b2World->CreateJoint(&revJoint);
	for (int i = 0; i < 13; i++) {
		revJoint.bodyA = ropeBody[i];
		revJoint.localAnchorA.Set(0, -locAnchor);
		revJoint.bodyB = ropeBody[i+1];
		revJoint.localAnchorB.Set(0, locAnchor);
		_b2World->CreateJoint(&revJoint);
	}
	revJoint.bodyA = ropeBody[13];
	revJoint.localAnchorA.Set(0, -locAnchor);
	revJoint.bodyB = donutsBody;
	revJoint.localAnchorB.Set(0, 0.85f);
	_b2World->CreateJoint(&revJoint);

	//���o�ó]�w circle02_rope ���i�ʺA����j
	auto circleSprite2 = _csbRoot->getChildByName("circle02_rope");
	Point locTail2 = circleSprite2->getPosition();
	Size sizeTail2 = circleSprite2->getContentSize();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(locTail2.x / PTM_RATIO, locTail2.y / PTM_RATIO);
	bodyDef.userData = circleSprite2;
	b2Body* donutsBody2 = _b2World->CreateBody(&bodyDef);
	b2CircleShape circleShape2;
	circleShape2.m_radius = (sizeTail2.width - 4)*0.5f / PTM_RATIO;
	fixtureDef.shape = &circleShape2;
	fixtureDef.density = 10.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	donutsBody2->CreateFixture(&fixtureDef);

	//����÷�l���`
	b2RopeJointDef JointDef2;
	JointDef2.bodyA = ropeHeadBody;
	JointDef2.bodyB = donutsBody2;
	JointDef2.localAnchorA = b2Vec2(0, 0);
	JointDef2.localAnchorB = b2Vec2(0, 30.0f / PTM_RATIO);
	JointDef2.maxLength = (locHead.y - locTail2.y - 30) / PTM_RATIO;
	JointDef2.collideConnected = true;
	b2RopeJoint* J2 = (b2RopeJoint*)_b2World->CreateJoint(&JointDef2);
	// �����A�H�u�q�۳s�A
	char tmp2[20] = "";
	Sprite *ropeSprite2[14];
	Point loc2[14];
	Size  size2[15];
	b2Body* ropeBody2[14];
	bodyDef.type = b2_dynamicBody;
	// �]���O÷�l�ҥH���q���n�ӭ�
	fixtureDef.density = 0.01f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;
	// ���ͤ@�t�C��÷�l�q�� rope01_01 ~ rope01_15�A�P�ɱ��_��
	for (int i = 0; i < 14; i++)
	{
		sprintf(tmp2, "rope02_%02d", i + 1);
		ropeSprite2[i] = (Sprite *)_csbRoot->getChildByName(tmp2);
		loc2[i] = ropeSprite2[i]->getPosition();
		size2[i] = ropeSprite2[i]->getContentSize();

		bodyDef.position.Set(loc2[i].x / PTM_RATIO, loc2[i].y / PTM_RATIO);
		bodyDef.userData = ropeSprite2[i];
		ropeBody2[i] = _b2World->CreateBody(&bodyDef);
		boxShape.SetAsBox((size2[i].width - 4)*0.5f / PTM_RATIO, (size2[i].height - 4)*0.5f / PTM_RATIO);
		ropeBody2[i]->CreateFixture(&fixtureDef);
	}
	// �Q�� RevoluteJoint �N�u�q�����s���b�@�_
	// ���s�� ropeHeadBody �P  ropeBody[0]

	float locAnchor2 = 0.5f*(size2[0].height - 5) / PTM_RATIO;
	b2RevoluteJointDef revJoint2;
	revJoint2.bodyA = ropeHeadBody;
	revJoint2.localAnchorA.Set(0, -0.9f);
	revJoint2.bodyB = ropeBody2[0];
	revJoint2.localAnchorB.Set(0, locAnchor2);
	_b2World->CreateJoint(&revJoint2);
	for (int i = 0; i < 13; i++) {
		revJoint2.bodyA = ropeBody2[i];
		revJoint2.localAnchorA.Set(0, -locAnchor2);
		revJoint2.bodyB = ropeBody2[i + 1];
		revJoint2.localAnchorB.Set(0, locAnchor2);
		_b2World->CreateJoint(&revJoint2);
	}
	revJoint2.bodyA = ropeBody2[13];
	revJoint2.localAnchorA.Set(0, -locAnchor2);
	revJoint2.bodyB = donutsBody2;
	revJoint2.localAnchorB.Set(0, 0.85f);
	_b2World->CreateJoint(&revJoint2);
	//
	//���o�ó]�w circle0_rope ���i�ʺA����j
	auto circleSprite3 = _csbRoot->getChildByName("circle03_rope");
	Point locTail3 = circleSprite3->getPosition();
	Size sizeTail3 = circleSprite3->getContentSize();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(locTail3.x / PTM_RATIO, locTail3.y / PTM_RATIO);
	bodyDef.userData = circleSprite3;
	b2Body* donutsBody3 = _b2World->CreateBody(&bodyDef);
	b2CircleShape circleShape3;
	circleShape3.m_radius = (sizeTail3.width - 4)*0.5f / PTM_RATIO;
	fixtureDef.shape = &circleShape3;
	fixtureDef.density = 10.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	donutsBody3->CreateFixture(&fixtureDef);

	//����÷�l���`
	b2RopeJointDef JointDef3;
	JointDef3.bodyA = ropeHeadBody;
	JointDef3.bodyB = donutsBody3;
	JointDef3.localAnchorA = b2Vec2(0, 0);
	JointDef3.localAnchorB = b2Vec2(0, 30.0f / PTM_RATIO);
	JointDef3.maxLength = (locHead.y - locTail3.y - 30) / PTM_RATIO;
	JointDef3.collideConnected = true;
	b2RopeJoint* J3 = (b2RopeJoint*)_b2World->CreateJoint(&JointDef3);
	// �����A�H�u�q�۳s�A
	char tmp3[20] = "";
	Sprite *ropeSprite3[14];
	Point loc3[14];
	Size  size3[15];
	b2Body* ropeBody3[14];
	bodyDef.type = b2_dynamicBody;
	// �]���O÷�l�ҥH���q���n�ӭ�
	fixtureDef.density = 0.01f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;
	// ���ͤ@�t�C��÷�l�q�� rope01_01 ~ rope01_15�A�P�ɱ��_��
	for (int i = 0; i < 14; i++)
	{
		sprintf(tmp3, "rope03_%02d", i + 1);
		ropeSprite3[i] = (Sprite *)_csbRoot->getChildByName(tmp3);
		loc3[i] = ropeSprite3[i]->getPosition();
		size3[i] = ropeSprite3[i]->getContentSize();

		bodyDef.position.Set(loc3[i].x / PTM_RATIO, loc3[i].y / PTM_RATIO);
		bodyDef.userData = ropeSprite3[i];
		ropeBody3[i] = _b2World->CreateBody(&bodyDef);
		boxShape.SetAsBox((size3[i].width - 4)*0.5f / PTM_RATIO, (size3[i].height - 4)*0.5f / PTM_RATIO);
		ropeBody3[i]->CreateFixture(&fixtureDef);
	}
	// �Q�� RevoluteJoint �N�u�q�����s���b�@�_
	// ���s�� ropeHeadBody �P  ropeBody[0]

	float locAnchor3 = 0.5f*(size2[0].height - 5) / PTM_RATIO;
	b2RevoluteJointDef revJoint3;
	revJoint3.bodyA = ropeHeadBody;
	revJoint3.localAnchorA.Set(0, -0.9f);
	revJoint3.bodyB = ropeBody3[0];
	revJoint3.localAnchorB.Set(0, locAnchor3);
	_b2World->CreateJoint(&revJoint3);
	for (int i = 0; i < 13; i++) {
		revJoint3.bodyA = ropeBody3[i];
		revJoint3.localAnchorA.Set(0, -locAnchor3);
		revJoint3.bodyB = ropeBody3[i + 1];
		revJoint3.localAnchorB.Set(0, locAnchor3);
		_b2World->CreateJoint(&revJoint3);
	}
	revJoint3.bodyA = ropeBody3[13];
	revJoint3.localAnchorA.Set(0, -locAnchor3);
	revJoint3.bodyB = donutsBody3;
	revJoint3.localAnchorB.Set(0, 0.85f);
	_b2World->CreateJoint(&revJoint3);


}

void JointScene::readBlocksCSBFile(const char *csbfilename)
{
	auto csbRoot = CSLoader::createNode(csbfilename);
	csbRoot->setPosition(_visibleSize.width / 2.0f, _visibleSize.height / 2.0f);
	addChild(csbRoot, 1);
	char tmp[20] = "";
	for (size_t i = 1; i <= 3; i++)
	{
		// ���ͩһݭn�� Sprite file name int plist 
		sprintf(tmp, "block1_%02d", i); 
	}
}

void JointScene::readSceneFile(const char *csbfilename)
{
	auto csbRoot = CSLoader::createNode(csbfilename);
	csbRoot->setPosition(_visibleSize.width / 2.0f, _visibleSize.height / 2.0f);
	addChild(csbRoot, 1);
	char tmp[20] = "";
	for (size_t i = 1; i <= 3; i++)
	{
		// ���ͩһݭn�� Sprite file name int plist 
		sprintf(tmp, "XXX_%02d", i);
	}
}

void JointScene::doStep(float dt)
{
	int velocityIterations = 8;	// �t�׭��N����
	int positionIterations = 1; // ��m���N���� ���N���Ƥ@��]�w��8~10 �V���V�u����Ĳv�V�t
	// Instruct the world to perform a single step of simulation.
	// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);

	// ���o _b2World ���Ҧ��� body �i��B�z
	// �̥D�n�O�ھڥثe�B�⪺���G�A��s���ݦb body �� sprite ����m
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		// �H�U�O�H Body ���]�t Sprite ��ܬ���
		if (body->GetUserData() != NULL)
		{
			Sprite *ballData = (Sprite*)body->GetUserData();
			ballData->setPosition(body->GetPosition().x*PTM_RATIO, body->GetPosition().y*PTM_RATIO);
			ballData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}
	}
	//create car smoke
	float lengthV = wheelbodyB->GetLinearVelocity().x;
	float lengthY = wheelbodyB->GetLinearVelocity().y;
	if (lengthV > 4.5f || lengthV < -4.5f) {
		_bSmoking = true;
		if (_bSmoking) { //�i�H�Q�o�A��{�o�����Q�o
			_bSmoking = false; // �}�l�p��
			for (int i = 0; i < 2; i++) {
				// �إ� Spark Sprite �ûP�ثe�����鵲�X
				auto smokeSprite = Sprite::createWithSpriteFrameName("cloud.png");
				smokeSprite->setColor(Color3B(255, 255, 255));
				smokeSprite->setBlendFunc(BlendFunc::ADDITIVE);
				this->addChild(smokeSprite, 5);
				//���ͤp������
				b2BodyDef RectBodyDef;
				RectBodyDef.position.Set(wheelbodyB->GetPosition().x, wheelbodyB->GetPosition().y);
				RectBodyDef.type = b2_dynamicBody;
				RectBodyDef.userData = smokeSprite;
				b2PolygonShape RectShape;
				RectShape.SetAsBox(5 / PTM_RATIO, 5 / PTM_RATIO);
				b2Body* RectBody = _b2World->CreateBody(&RectBodyDef);
				b2FixtureDef RectFixtureDef;
				RectFixtureDef.shape = &RectShape;
				RectFixtureDef.density = 1.0f;
				RectFixtureDef.isSensor = true;
				b2Fixture*RectFixture = RectBody->CreateFixture(&RectFixtureDef);

				//���O�q
				RectBody->ApplyForce(b2Vec2(-lengthV * (int)(rand() % 20) + 25, -lengthY*10), wheelbodyB->GetPosition(), true);
			}
		}
	}
	//create tooth flare
	int max;
	for (int i = 0; i < 15; i++) {		
		float lengthT = dynamicBodyTooth[i]->GetAngularVelocity();
		float length = dynamicBodyTooth[i+1]->GetAngularVelocity();
		max = lengthT > length ? i : i+1;
	}
	float lengthT = dynamicBodyTooth[max]->GetAngularVelocity();
	if (lengthT > 2.5f) {
		_bToothL = true;
		_bToothR = true;
		if (_bToothL) { //�i�H�Q�o�A��{�o�����Q�o
			_bToothL = false; // �}�l�p��
			for (int i = 0; i < 1; i++) {
				// �إ� Spark Sprite �ûP�ثe�����鵲�X
				auto smokeSprite = Sprite::createWithSpriteFrameName("flare.png");
				smokeSprite->setColor(Color3B(rand() % 256, rand() % 256, rand() % 156));
				smokeSprite->setBlendFunc(BlendFunc::ADDITIVE);
				this->addChild(smokeSprite, 5);
				//���ͤp������
				b2BodyDef RectBodyDef;
				RectBodyDef.position.Set(dynamicBody[0]->GetPosition().x+2.0f, dynamicBody[0]->GetPosition().y);
				RectBodyDef.type = b2_dynamicBody;
				RectBodyDef.userData = smokeSprite;
				b2PolygonShape RectShape;
				RectShape.SetAsBox(5 / PTM_RATIO, 5 / PTM_RATIO);
				b2Body* RectBody = _b2World->CreateBody(&RectBodyDef);
				b2FixtureDef RectFixtureDef;
				RectFixtureDef.shape = &RectShape;
				RectFixtureDef.density = 1.0f;
				RectFixtureDef.isSensor = true;
				b2Fixture*RectFixture = RectBody->CreateFixture(&RectFixtureDef);
				b2Vec2 toothPos(dynamicBody[0]->GetPosition().x + 140.0f, dynamicBody[0]->GetPosition().y);
				//���O�q
				RectBody->ApplyForce(b2Vec2(lengthT * (int)(rand() % 20) + 25, lengthT * (int)(rand() % 20) + 25), toothPos, true);
			}
		}
		if (_bToothR) { //�i�H�Q�o�A��{�o�����Q�o
			_bToothR = false; // �}�l�p��
			for (int i = 0; i < 1; i++) {
				// �إ� Spark Sprite �ûP�ثe�����鵲�X
				auto smokeSprite = Sprite::createWithSpriteFrameName("spark.png");
				smokeSprite->setColor(Color3B(rand() % 256, rand() % 256, rand() % 156));
				smokeSprite->setBlendFunc(BlendFunc::ADDITIVE);
				this->addChild(smokeSprite, 5);
				//���ͤp������
				b2BodyDef RectBodyDef;
				RectBodyDef.position.Set(dynamicBody[1]->GetPosition().x - 2.0f, dynamicBody[1]->GetPosition().y);
				RectBodyDef.type = b2_dynamicBody;
				RectBodyDef.userData = smokeSprite;
				b2PolygonShape RectShape;
				RectShape.SetAsBox(5 / PTM_RATIO, 5 / PTM_RATIO);
				b2Body* RectBody = _b2World->CreateBody(&RectBodyDef);
				b2FixtureDef RectFixtureDef;
				RectFixtureDef.shape = &RectShape;
				RectFixtureDef.density = 1.0f;
				RectFixtureDef.isSensor = true;
				b2Fixture*RectFixture = RectBody->CreateFixture(&RectFixtureDef);
				b2Vec2 toothPos(dynamicBody[1]->GetPosition().x + 140.0f, dynamicBody[1]->GetPosition().y);
				//���O�q
				RectBody->ApplyForce(b2Vec2(-lengthT * (int)(rand() % 20) + 25, lengthT * (int)(rand() % 20) + 25), toothPos, true);
			}
		}
	}
	//car impulse
	float posC = bodyCar->GetPosition().x;
	float posS = SensorBody[0]->GetPosition().x;
	if (posC- posS<1.0f && posC - posS>0.0f) {
		bodyCar->ApplyLinearImpulse(b2Vec2(50 + rand() % 1010,0), SensorBody[0]->GetWorldCenter(), true);
	}
	posC = bodyBall->GetPosition().x;
	posS = SensorBody[1]->GetPosition().x;
	if (posC - posS<1.0f && posC - posS>-1.0f) {
		bodyBall->ApplyLinearImpulse(b2Vec2(0,50 + rand() % 1010), SensorBody[1]->GetWorldCenter(), true);
	}
	//ball status (door
	for(int i=0;i<2;i++){
		if (ballBody[i] != NULL ) {
			if (ballBody[i]->GetPosition().x < 116.7f / PTM_RATIO && ballBody[0]->GetPosition().y < 145.24f / PTM_RATIO)ball1 = true;
			else ball1 = false;
		}
	}
	if (ball1) {
			doorStaticBody[0]->SetActive(false);
			doorSprite[0]->setVisible(false);
	}
	else {
		doorStaticBody[0]->SetActive(true);
		doorSprite[0]->setVisible(true);
	}
	//door2
	if (ballNum2 > 30) {
		doorStaticBody[1]->SetActive(false);
		doorSprite[1]->setVisible(false);
	}
	//thridPart
	if (bodyCar->GetPosition().x - SensorBody[2]->GetPosition().x > -1.0f && bodyCar->GetPosition().x - SensorBody[2]->GetPosition().x < 1.0f) {
		thirdPart = true;
	}
}

bool JointScene::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//Ĳ�I�}�l�ƥ�
{
	Point touchLoc = pTouch->getLocation();
	_bdraw = true;
	// For Mouse Joiint 
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		if (body->GetUserData() == NULL ) continue; // �R�A���餣�B�z
		if (body->GetFixtureList()->GetRestitution() == 0.75f)continue;
		//ballBody->CreateFixture(&fixtureDef);	// �b Body �W���ͳo�ӭ��骺�]�w
		// �P�_�I����m�O�_���b�ʺA����@�w���d��
		Sprite *spriteObj = (Sprite*)body->GetUserData();
		Size objSize = spriteObj->getContentSize();
		float fdist = MAX_2(objSize.width, objSize.height)/2.0f;
		float x = body->GetPosition().x*PTM_RATIO- touchLoc.x;
		float y = body->GetPosition().y*PTM_RATIO - touchLoc.y;
		float tpdist = x*x + y*y;
		if ( tpdist < fdist*fdist)
		{
			_bTouchOn = true;
			_bdraw = false;//�b����W����e�e
			b2MouseJointDef mouseJointDef;
			mouseJointDef.bodyA = _bottomBody;
			mouseJointDef.bodyB = body;
			mouseJointDef.target = b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO);
			mouseJointDef.collideConnected = true;
			mouseJointDef.maxForce = 1000.0f * body->GetMass();
			_MouseJoint = (b2MouseJoint*)_b2World->CreateJoint(&mouseJointDef); // �s�W Mouse Joint
			body->SetAwake(true);
			break;
		}
	}
	_ballBtn->touchesBegin(touchLoc);
	_triBtn->touchesBegin(touchLoc);
	return true;
}

void  JointScene::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I���ʨƥ�
{
	Point touchLoc = pTouch->getLocation();
	if (_bTouchOn)
	{
		_MouseJoint->SetTarget(b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO));
	}
	_ballBtn->touchesBegin(touchLoc);
	_triBtn->touchesBegin(touchLoc);
	//brush
	Sprite* get;
	//Point touchLoc = pTouch->getLocation();
	Point start = pTouch->getLocationInView();
	start = CCDirector::sharedDirector()->convertToGL(start);
	Point end = pTouch->getPreviousLocationInView();
	end = CCDirector::sharedDirector()->convertToGL(end);

	float distance = ccpDistance(start, end);
	if (thirdPart) {
		if (_bdraw != false)//�p�G�O�b�i�H�e�e���a��
		{
			//�ھګe�@�I�M�{�b�I����m�P�_�����n�X�ӹϧΡA�ó�����m�A����ilist��
			for (int i = 0; i < distance; i += 50)
			{
				float difx = end.x - start.x;
				float dify = end.y - start.y;
				float delta = (float)i / distance;
				get = _FreeList.front();
				get->setVisible(true);
				Point loc = ccp(start.x + (difx * delta), start.y + (dify * delta));
				get->setPosition(loc);
				_FreeList.pop_front();
				_InUsedList.push_back(get);
				_iFree--; _iInUsed++;
				plataformPoints.push_back(loc);
			}
		}
	}
}

void  JointScene::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I�����ƥ� 
{
	Point touchLoc = pTouch->getLocation();
	Size ball_size;
	float scale_x;
	brushBodyDef.type = b2_dynamicBody; //this will be a dynamic body
	brushBodyDef.position.Set(_visibleSize.width / 2, _visibleSize.height / 2); //set the starting position

	Sprite* get;
	list <cocos2d::Point>::iterator it;

	if (_bTouchOn)
	{
		_bTouchOn = false;
		if (_MouseJoint != NULL )
		{
			_b2World->DestroyJoint(_MouseJoint);
			_MouseJoint = NULL;
		}
	}
	//
	if (_ballBtn->touchesEnded(touchLoc)) {
		ballNum++;
		if (ballNum <= 2) {
			// �H����ܤ@���y�A�q  �B����ۥѸ��U �A�j�p�Y��50%
			auto ballSprite = Sprite::createWithSpriteFrameName(g_CircleObject[rand() % MAX_CIRCLE_OBJECTS]);
			ballSprite->setScale(0.5f);
			//	ballSprite->setPosition(touchLoc);
			this->addChild(ballSprite, 2);

			// �إߤ@��²�檺�ʺA�y��
			b2BodyDef bodyDef;	// ���H���c b2BodyDef �ŧi�@�� Body ���ܼ�
			bodyDef.type = b2_dynamicBody; // �]�w���ʺA����
			bodyDef.userData = ballSprite;	// �]�w Sprite ���ʺA���骺��ܹϥ�
			bodyDef.position.Set((rand() % 271 + 30.0f) / PTM_RATIO, 690.0f / PTM_RATIO);
			// �H bodyDef �b b2World  ���إ߹���öǦ^�ӹ��骺����
			ballBody[ballNum-1] = _b2World->CreateBody(&bodyDef);

			// �]�w�Ӫ��骺�~��
			b2CircleShape ballShape;	//  �ŧi���骺�~�������ܼơA���B�O��Ϊ���
			Size ballsize = ballSprite->getContentSize();	// �ھ� Sprite �ϧΪ��j�p�ӳ]�w��Ϊ��b�|
			ballShape.m_radius = 0.5f*(ballsize.width - 4) *0.5f / PTM_RATIO;
			// �H b2FixtureDef  ���c�ŧi���鵲�c�ܼơA�ó]�w���骺�������z�Y��
			b2FixtureDef fixtureDef;
			fixtureDef.shape = &ballShape;			// ���w���骺�~�������
			fixtureDef.restitution = 0.75f;			// �]�w�u�ʫY��
			fixtureDef.density = 1.0f;				// �]�w�K��
			fixtureDef.friction = 0.15f;			// �]�w�����Y��
			ballBody[ballNum - 1]->CreateFixture(&fixtureDef);	// �b Body �W���ͳo�ӭ��骺�]�w
		}
	}
	if (_triBtn->touchesEnded(touchLoc) && ball1) {
		ballNum2++;
			// �H����ܤ@���y�A�q  �B����ۥѸ��U �A�j�p�Y��50%
			auto ballSprite = Sprite::createWithSpriteFrameName(g_CircleObject[rand() % MAX_CIRCLE_OBJECTS]);
			ballSprite->setScale(0.2f);
			//	ballSprite->setPosition(touchLoc);
			this->addChild(ballSprite, 2);

			// �إߤ@��²�檺�ʺA�y��
			b2BodyDef bodyDef;	// ���H���c b2BodyDef �ŧi�@�� Body ���ܼ�
			bodyDef.type = b2_dynamicBody; // �]�w���ʺA����
			bodyDef.userData = ballSprite;	// �]�w Sprite ���ʺA���骺��ܹϥ�
			bodyDef.position.Set(605.0f / PTM_RATIO, 440.0f / PTM_RATIO);
			// �H bodyDef �b b2World  ���إ߹���öǦ^�ӹ��骺����
			b2Body *ballBody = _b2World->CreateBody(&bodyDef);

			// �]�w�Ӫ��骺�~��
			b2CircleShape ballShape;	//  �ŧi���骺�~�������ܼơA���B�O��Ϊ���
			Size ballsize = ballSprite->getContentSize()*0.4f;	// �ھ� Sprite �ϧΪ��j�p�ӳ]�w��Ϊ��b�|
			ballShape.m_radius = 0.5f*(ballsize.width - 4) *0.5f / PTM_RATIO;
			// �H b2FixtureDef  ���c�ŧi���鵲�c�ܼơA�ó]�w���骺�������z�Y��
			b2FixtureDef fixtureDef;
			fixtureDef.shape = &ballShape;			// ���w���骺�~�������
			fixtureDef.restitution = 0.75f;			// �]�w�u�ʫY��
			fixtureDef.density = 1.0f;				// �]�w�K��
			fixtureDef.friction = 0.15f;			// �]�w�����Y��
			ballBody->CreateFixture(&fixtureDef);	// �b Body �W���ͳo�ӭ��骺�]�w
		
	}
	//brush
	if (thirdPart) {
		if (plataformPoints.size() == 1) {
			get = _InUsedList.front();
			cocos2d::Point start = plataformPoints.front();
			brushBodyDef.position.Set(start.x / PTM_RATIO, start.y / PTM_RATIO);
			brushBodyDef.userData = get;
			newBody = _b2World->CreateBody(&brushBodyDef);
			ball_size = get->getContentSize();
			scale_x = get->getScaleX();
			b2CircleShape ballShape;//  �ŧi���骺�~�������ܼơA���B�O��Ϊ���
									// �ھ� Sprite �ϧΪ��j�p�ӳ]�w��Ϊ��b�|
			ballShape.m_radius = (ball_size.width - 4)*scale_x *0.5f / PTM_RATIO;
			// �H b2FixtureDef  ���c�ŧi���鵲�c�ܼơA�ó]�w���骺�������z�Y��
			brushfixtureDef.shape = &ballShape;// ���w���骺�~�������
			brushfixtureDef.restitution = 0.0f;// �]�w�u�ʫY��
			brushfixtureDef.density = 5.0f;// �]�w�K��
			brushfixtureDef.friction = 1.0f;// �]�w�����Y��
			brushfixtureDef.filter.maskBits = 1 << (_boxuse % 10 + 1) | 1;
			brushfixtureDef.filter.categoryBits = 1 << (_boxuse % 10 + 1);
			newBody->CreateFixture(&brushfixtureDef);
			_InUsedList.pop_front();
			_BoxList.push_back(get);
			_iInUsed--;
			_boxuse++;
			plataformPoints.pop_front();
		}
		//move���Ƽƫإ߭���
		else if (plataformPoints.size() > 1) {
			//�M����mlist�@�M�A���L�̭���
			for (it = plataformPoints.begin(); it != plataformPoints.end();)
			{
				get = _InUsedList.front();
				cocos2d::Point start = (*it);
				brushBodyDef.position.Set(start.x / PTM_RATIO, start.y / PTM_RATIO);
				brushBodyDef.userData = get;
				newBody = _b2World->CreateBody(&brushBodyDef);
				ball_size = get->getContentSize();
				scale_x = get->getScaleX();
				b2CircleShape ballShape;//  �ŧi���骺�~�������ܼơA���B�O��Ϊ���
										// �ھ� Sprite �ϧΪ��j�p�ӳ]�w��Ϊ��b�|
				ballShape.m_radius = (ball_size.width - 4)*scale_x *0.5f / PTM_RATIO;
				// �H b2FixtureDef  ���c�ŧi���鵲�c�ܼơA�ó]�w���骺�������z�Y��
				brushfixtureDef.shape = &ballShape;// ���w���骺�~�������
				brushfixtureDef.restitution = 0.0f;// �]�w�u�ʫY��
				brushfixtureDef.density = 0.5f;// �]�w�K��
				brushfixtureDef.friction = 1.0f;// �]�w�����Y��
				brushfixtureDef.filter.maskBits = 1 << (_boxuse % 10 + 1) | 1;
				brushfixtureDef.filter.categoryBits = 1 << (_boxuse % 10 + 1);
				newBody->CreateFixture(&brushfixtureDef);
				_InUsedList.pop_front();
				_BoxList.push_back(get);
				_iInUsed--;
				_boxuse++;
				it++;
				plataformPoints.pop_front();
				brushBody[brush_i % 2] = newBody;
				//��e�@�өM�{�b���βk��joint���_��
				if (brush_i > 0) {
					b2PulleyJointDef JointDef;
					b2Body* body1 = brushBody[(brush_i - 1) % 2];
					b2Body* body2 = brushBody[brush_i % 2];
					//		JointDef.Initialize(body1, body2, b2Vec2((body1->GetPosition().x + body2->GetPosition().x) / 2.0f, (body1->GetPosition().y + body2->GetPosition().y) / 2.0f));
					JointDef.Initialize(body1, body2,
						b2Vec2(body1->GetPosition().x / PTM_RATIO, body1->GetPosition().y),
						b2Vec2(body2->GetPosition().x / PTM_RATIO, body2->GetPosition().y),
						b2Vec2(body1->GetPosition().x, body1->GetPosition().y / PTM_RATIO),
						b2Vec2(body2->GetPosition().x, body2->GetPosition().y / PTM_RATIO),
						1);
					_b2World->CreateJoint(&JointDef); // �ϥιw�]�Ȳk��

					b2WeldJointDef WeldJointDef;
					WeldJointDef.Initialize(body1, body2, b2Vec2((body1->GetPosition().x + body2->GetPosition().x) / 2.0f, (body1->GetPosition().y + body2->GetPosition().y) / 2.0f));
					_b2World->CreateJoint(&WeldJointDef); // �ϥιw�]�Ȳk��
				}
				brush_i++;
			}
			brush_i = 0;
		}
	}
	if (_bdraw)
	{
		_bdraw = false;
	}
}
void JointScene::setupBallbtn() {
	//
	auto btnSprite = _csbRoot->getChildByName("ball_btn");
	_ballBtn = CButton::create();
	_ballBtn->setButtonInfo("dnarrow.png", "dnarrowon.png", btnSprite->getPosition());
	_ballBtn->setScale(btnSprite->getScale());
	this->addChild(_ballBtn, 5);
	btnSprite->setVisible(false);
}
void JointScene::setupTribtn() {
	//
	auto btnSprite = _csbRoot->getChildByName("tri_btn");
	_triBtn = CButton::create();
	_triBtn->setButtonInfo("dnarrow.png", "dnarrowon.png", btnSprite->getPosition());
	_triBtn->setScale(btnSprite->getScale());
	this->addChild(_triBtn, 5);
	btnSprite->setVisible(false);
}
void JointScene::createStaticBoundary()
{
	// ������ Body, �]�w�������Ѽ�

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2Body *body = _b2World->CreateBody(&bodyDef);

	_bottomBody = body;
	// �����R�A��ɩһݭn�� EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef edgeFixtureDef; // ���� Fixture
	edgeFixtureDef.shape = &edgeShape;
	// bottom edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// left edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// right edge
	edgeShape.Set(b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// top edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// Ū���Ҧ� wall1_ �}�Y���ϥ� ���O�R�A����
	// �{���X�Ӧ� StaticDynamicScene.cpp
	char tmp[20] = "";

	// ���� EdgeShape �� body
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	body = _b2World->CreateBody(&bodyDef);

	// �����R�A��ɩһݭn�� EdgeShape
	b2FixtureDef fixtureDef; // ���� Fixture
	fixtureDef.shape = &edgeShape;

	for (size_t i = 1; i <= 19; i++)
	{
		// ���ͩһݭn�� Sprite file name int plist 
		// ���B���o�����O�۹�� csbRoot �Ҧb��m���۹�y��
		// �b�p�� edgeShape ���۹����y�ЮɡA�����i���ഫ
		sprintf(tmp, "wall1_%02d", i);
		auto edgeSprite = (Sprite *)_csbRoot->getChildByName(tmp);
		Size ts = edgeSprite->getContentSize();
		Point loc = edgeSprite->getPosition();
		float angle = edgeSprite->getRotation();
		float scale = edgeSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j

		Point lep1, lep2, wep1, wep2; // EdgeShape ����Ӻ��I
		lep1.y = 0; lep1.x = -(ts.width - 4) / 2.0f;
		lep2.y = 0; lep2.x = (ts.width - 4) / 2.0f;

		// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
		// �ھ��Y��B���ಣ�ͩһݭn���x�}
		// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
		// �M��i�����A
		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scale;  // ���]�w X �b���Y��
		cocos2d::Mat4::createRotationZ(angle*M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = loc.x; //�]�w Translation�A�ۤv���[�W���˪�
		modelMatrix.m[7] = loc.y; //�]�w Translation�A�ۤv���[�W���˪�

											 // ���ͨ�Ӻ��I
		wep1.x = lep1.x * modelMatrix.m[0] + lep1.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep1.y = lep1.x * modelMatrix.m[4] + lep1.y * modelMatrix.m[5] + modelMatrix.m[7];
		wep2.x = lep2.x * modelMatrix.m[0] + lep2.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep2.y = lep2.x * modelMatrix.m[4] + lep2.y * modelMatrix.m[5] + modelMatrix.m[7];

		edgeShape.Set(b2Vec2(wep1.x / PTM_RATIO, wep1.y / PTM_RATIO), b2Vec2(wep2.x / PTM_RATIO, wep2.y / PTM_RATIO));
		body->CreateFixture(&fixtureDef);
	}

}
void JointScene::setupDoor() {
	
	char tmp[20] = "";
	for (size_t i = 1; i <= 2; i++)
	{
		// ���ͩһݭn�� Sprite file name int plist 
		// ���B���o�����O�۹�� csbRoot �Ҧb��m���۹�y��
		// �b�p�� edgeShape ���۹����y�ЮɡA�����i���ഫ
		sprintf(tmp, "door_%d", i);
		doorSprite[i-1] = (Sprite *)_csbRoot->getChildByName(tmp);
		Size ts = doorSprite[i-1]->getContentSize();
		Point loc = doorSprite[i-1]->getPosition();
		float angle = doorSprite[i-1]->getRotation();
		float sx = doorSprite[i-1]->getScaleX();
		float sy = doorSprite[i-1]->getScaleY();

		b2BodyDef staticBodyDef;
		staticBodyDef.type = b2_staticBody;
		staticBodyDef.userData = NULL;
		staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
		staticBodyDef.angle = MATH_RAD_TO_DEG( angle );
		doorStaticBody[i-1] = _b2World->CreateBody(&staticBodyDef);	
		b2FixtureDef fixtureDef;
		b2PolygonShape boxShape;
		boxShape.SetAsBox((ts.width - 4) *0.5f *sx / PTM_RATIO, (ts.height - 4) *0.5f *sy / PTM_RATIO);
		fixtureDef.shape = &boxShape;
		doorStaticBody[i - 1]->CreateFixture(&fixtureDef);

	}
	// _b2World->DestroyBody(doorStaticBody[0]);
	 //doorSprite[0]->setVisible(false);
	
}
//���l
void JointScene::setupCar() {

	// ���o�ó]�w wheel_1 ���i�ʺA����A�j
	auto circleSprite = _csbRoot->getChildByName("wheel_1");
	Point locA = circleSprite->getPosition();
	Size size = circleSprite->getContentSize();
	float scale = circleSprite->getScale();
	b2CircleShape circleShape;
	circleShape.m_radius = size.width*0.5f*scale / PTM_RATIO;

	b2BodyDef bodyDef;	//���H���c b2BodyDef �ŧi�@�� Body ���ܼ�
	bodyDef.type = b2_dynamicBody;		// �]�w���ʺA����
	bodyDef.position.Set(locA.x / PTM_RATIO, locA.y / PTM_RATIO);
	bodyDef.userData = circleSprite;	// �]�w�ϥ�
	b2Body* bodyA = _b2World->CreateBody(&bodyDef);
	b2FixtureDef fixtureDef;			// �]�w���z�ʽ�
	fixtureDef.shape = &circleShape;	// �ŧi���骺�~�������ܼƬO��Ϊ���
	fixtureDef.density = 100.0f;
	fixtureDef.friction = 0.2f;			// �]�w�����Y��
	bodyA->CreateFixture(&fixtureDef);

	// ���o�ó]�w wheel_2 ���i�ʺA����B�j
	circleSprite = _csbRoot->getChildByName("wheel_2");
	Point locB = circleSprite->getPosition();
	size = circleSprite->getContentSize();
	scale = circleSprite->getScale();
	circleShape.m_radius = size.width*0.5f*scale / PTM_RATIO;

	bodyDef.type = b2_dynamicBody;		// �]�w���ʺA����
	bodyDef.position.Set(locB.x / PTM_RATIO, locB.y / PTM_RATIO);
	bodyDef.userData = circleSprite;	// �]�w�ϥ�
	wheelbodyB = _b2World->CreateBody(&bodyDef);
	wheelbodyB->CreateFixture(&fixtureDef);	// �P�W��bodyA�ۦP

	

	//setcar
	auto carSprite = _csbRoot->getChildByName("carBody");
	Point locCar = carSprite->getPosition();
	Size carsize = carSprite->getContentSize();
	float carscale = carSprite->getScale();

	b2BodyDef carDef;
	carDef.type = b2_dynamicBody;
	carDef.position.Set(locCar.x / PTM_RATIO, locCar.y / PTM_RATIO);
	carDef.userData = carSprite;
	bodyCar = _b2World->CreateBody(&carDef);

	b2PolygonShape carShape;
	carShape.SetAsBox(carsize.width *carscale / 2.5 / PTM_RATIO,
		carsize.height * carscale / 2.5 / PTM_RATIO);

	b2FixtureDef carFixture;
	carFixture.shape = &carShape;
	carFixture.density = 100.0f;
	carFixture.friction = 0.5f;

	bodyCar->CreateFixture(&carFixture);

	b2RevoluteJointDef revJoint, revJoint2;
	revJoint.bodyA = bodyA;
	revJoint.localAnchorA.Set(0, 0);
	revJoint.bodyB = bodyCar;
	revJoint.localAnchorB.Set(0.7, -0.7);
	_b2World->CreateJoint(&revJoint);

	revJoint2.bodyA = wheelbodyB;
	revJoint2.localAnchorA.Set(0, 0);
	revJoint2.bodyB = bodyCar;
	revJoint2.localAnchorB.Set(-0.9, -0.7);
	_b2World->CreateJoint(&revJoint2);

}
void JointScene::setupBall() {
	// ���o�ó]�w wheel_1 ���i�ʺA����A�j
	auto circleSprite = _csbRoot->getChildByName("ball_1");
	Point locA = circleSprite->getPosition();
	Size size = circleSprite->getContentSize();
	float scale = circleSprite->getScale();
	b2CircleShape circleShape;
	circleShape.m_radius = size.width*0.5f*scale / PTM_RATIO;

	b2BodyDef bodyDef;	//���H���c b2BodyDef �ŧi�@�� Body ���ܼ�
	bodyDef.type = b2_dynamicBody;		// �]�w���ʺA����
	bodyDef.position.Set(locA.x / PTM_RATIO, locA.y / PTM_RATIO);
	bodyDef.userData = circleSprite;	// �]�w�ϥ�
	bodyBall = _b2World->CreateBody(&bodyDef);
	b2FixtureDef fixtureDef;			// �]�w���z�ʽ�
	fixtureDef.shape = &circleShape;	// �ŧi���骺�~�������ܼƬO��Ϊ���
	fixtureDef.density = 1000.0f;
	fixtureDef.friction = 0.2f;			// �]�w�����Y��
	bodyBall->CreateFixture(&fixtureDef);
}
void JointScene::setupSensor() {
	char tmp[20] = "";
	for (int i = 1; i <=3; i++)
	{
		sprintf(tmp, "sensor_%02d", i);
		auto sensorSprite = (Sprite *)_csbRoot->getChildByName(tmp);
		Point loc = sensorSprite->getPosition();
		Size  size = sensorSprite->getContentSize();
		float scale = sensorSprite->getScale();
		sensorSprite->setVisible(false);
		b2BodyDef sensorBodyDef;
		sensorBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
		sensorBodyDef.type = b2_staticBody;

		SensorBody[i-1] = _b2World->CreateBody(&sensorBodyDef);
		b2PolygonShape sensorShape;
		sensorShape.SetAsBox(size.width *0.5f * scale / PTM_RATIO, size.height*0.5f*scale / PTM_RATIO);

		b2FixtureDef SensorFixtureDef;
		SensorFixtureDef.shape = &sensorShape;
		SensorFixtureDef.isSensor = true;	// �]�w�� Sensor
		SensorFixtureDef.density = 9999 + i; // �G�N�]�w���o�ӭȡA��K�IĲ�ɭԪ��P�_
		SensorBody[i-1]->CreateFixture(&SensorFixtureDef);

	}

}
void JointScene::setupTri() {
	b2BodyDef bodyDef;
	char tmp[20] = "";
	Sprite *triSprite[3];
	Point loc[3];
	Size  size[3];
	float scale[3];
	float rot[3];
	b2Body* staticBody[3];
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.userData = NULL;

	b2CircleShape staticShape;
	staticShape.m_radius = 5 / PTM_RATIO;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &staticShape;
	for (int i = 0; i < 3; i++)
	{
		sprintf(tmp, "triangle_%02d", i + 1);
		triSprite[i] = (Sprite *)_csbRoot->getChildByName(tmp);
		loc[i] = triSprite[i]->getPosition();
		size[i] = triSprite[i]->getContentSize();
		scale[i] = triSprite[i]->getScale();
		rot[i]=-MATH_DEG_TO_RAD( triSprite[i]->getRotation());
		//
		staticBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		staticBody[i] = _b2World->CreateBody(&staticBodyDef);
		staticBody[i]->CreateFixture(&fixtureDef);
	}
	b2Body *triDynamicbody[3];
	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	b2PolygonShape polyShape;//triangle
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.25f;
	for (int i = 0; i < 3; i++)
	{
		float sx = triSprite[i]->getScaleX();
		float sy = triSprite[i]->getScaleY();
		//
		Point lep[3], wep[3];	// triShape ���T�ӳ��I, 0 ���I�B 1 ���U�B 2 �k�U
		lep[0].x = 0;  lep[0].y = (size[i].height - 2) / 2.0f;
		lep[1].x = -(size[i].width - 2) / 2.0f; lep[1].y = -(size[i].height - 2) / 2.0f;
		lep[2].x = (size[i].width - 2) / 2.0f; lep[2].y = -(size[i].height - 2) / 2.0f;

		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = sx;  // ���]�w X �b���Y��
		modelMatrix.m[5] = sy;  // ���]�w Y �b���Y��
		for (size_t j = 0; j < 3; j++)
		{   // �ǤJ�Y��P���઺ local space ���y�Эp��
			wep[j].x = lep[j].x * modelMatrix.m[0] + lep[j].y * modelMatrix.m[1];
			wep[j].y = lep[j].x * modelMatrix.m[4] + lep[j].y * modelMatrix.m[5];
		}
		b2Vec2 vecs[] = {
			b2Vec2(wep[0].x / PTM_RATIO, wep[0].y / PTM_RATIO),
			b2Vec2(wep[1].x / PTM_RATIO, wep[1].y / PTM_RATIO),
			b2Vec2(wep[2].x / PTM_RATIO, wep[2].y / PTM_RATIO)
		};
		
		//
		fixtureDef.shape = &polyShape;
		polyShape.SetAsBox((size[i].width - 4) *0.5f *sx / PTM_RATIO, (size[i].height - 4) *0.5f *sy / PTM_RATIO);
		polyShape.Set(vecs, 3);//
		dynamicBodyDef.userData = triSprite[i];
		dynamicBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		dynamicBodyDef.angle = rot[i];
		triDynamicbody[i] = _b2World->CreateBody(&dynamicBodyDef);
		triDynamicbody[i]->CreateFixture(&fixtureDef);
	}
	b2RevoluteJointDef RJoint;	// �������`
	b2RevoluteJoint *RevJoint[3];
	for (int i = 0; i < 3; i++)
	{
			RJoint.Initialize(staticBody[i], triDynamicbody[i], triDynamicbody[i]->GetWorldCenter());
			RevJoint[i] = (b2RevoluteJoint*)_b2World->CreateJoint(&RJoint);	
	}

	//
	// ���o�ó]�w frame01_weld ���i�R�A����j
		auto frameSprite = _csbRoot->getChildByName("frame_01");
	Point loc2 = frameSprite->getPosition();
	Size size2 = frameSprite->getContentSize();
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc2.x / PTM_RATIO, loc2.y / PTM_RATIO);
	staticBodyDef.userData = frameSprite;
	b2Body* staticBody2 = _b2World->CreateBody(&staticBodyDef);
	float sx = frameSprite->getScaleX();
	float sy = frameSprite->getScaleY();
	
	b2PolygonShape boxShape;
	boxShape.SetAsBox((size2.width - 4) *0.5f *sx / PTM_RATIO, (size2.height - 4) *0.5f *sy / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	staticBody2->CreateFixture(&fixtureDef);

	// ���o�ó]�w frame01_weld ���i�R�A����j
	auto frameSprite2 = _csbRoot->getChildByName("frame_02");
	loc2 = frameSprite2->getPosition();
	size2 = frameSprite2->getContentSize();
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc2.x / PTM_RATIO, loc2.y / PTM_RATIO);
	staticBodyDef.userData = frameSprite;
	staticBody2 = _b2World->CreateBody(&staticBodyDef);
	sx = frameSprite2->getScaleX();
	sy = frameSprite2->getScaleY();

	//b2PolygonShape boxShape;
	boxShape.SetAsBox((size2.width - 4) *0.5f *sx / PTM_RATIO, (size2.height - 4) *0.5f *sy / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	staticBody2->CreateFixture(&fixtureDef);
}
CContactListener2::CContactListener2()
{
	_bApplyImpulse = false;
	_bCreateSpark = false;
	_NumOfSparks = 5;
}
void CContactListener2::setCollisionTarget(cocos2d::Sprite &targetSprite)
{
	_targetSprite = &targetSprite;
}

//
// �u�n�O��� body �� fixtures �I���A�N�|�I�s�o�Ө禡
//
void CContactListener2::BeginContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	// check �O�_�����U���y�g�L sensor1 �A�u�n�g�L�N�ߨ����L�u�X�h
	if (BodyA->GetFixtureList()->GetDensity() == 10000.0f) { // �N�� sensor1
		BodyB->ApplyLinearImpulse(b2Vec2(0, 50 + rand() % 101), BodyB->GetWorldCenter(), true);
		_bApplyImpulse = true;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10000.0f) {// �N�� sensor1
		BodyA->ApplyLinearImpulse(b2Vec2(0, 50 + rand() % 101), BodyB->GetWorldCenter(), true);
		_bApplyImpulse = true;
	}

}

//�I������
void CContactListener2::EndContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	if (BodyA->GetFixtureList()->GetDensity() == 10001.0f && _bApplyImpulse) { // �N�� sensor2
		BodyA->GetFixtureList()->SetDensity(10002);
		_bApplyImpulse = false;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10001.0f && _bApplyImpulse) {	// �N�� sensor2
		BodyB->GetFixtureList()->SetDensity(10002);
		_bApplyImpulse = false;
	}
}
#ifdef BOX2D_DEBUG
//��gø�s��k
void JointScene::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif