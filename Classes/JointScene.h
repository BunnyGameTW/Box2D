#ifndef __JOINTSCENE_H__
#define __JOINTSCENE_H__

//#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "Common/CButton.h"
#ifdef BOX2D_DEBUG
#include "Common/GLES-Render.h"
#include "Common/GB2ShapeCache-x.h"
#endif

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f
#include <list>
using namespace std;
#define NUMBER_BRUSH 3000
class CContactListener2 : public b2ContactListener
{
public:
	cocos2d::Sprite *_targetSprite; // 用於判斷是否
	bool _bCreateSpark;		//產生火花
	bool _bApplyImpulse;	// 產生瞬間的衝力
	b2Vec2 _createLoc;
	int  _NumOfSparks;
	CContactListener2();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite &targetSprite);
};
class JointScene : public cocos2d::Layer
{
public:

	~JointScene();
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();
	Node *_csbRoot;

	// for Box2D
	b2World* _b2World;
	cocos2d::Label *_titleLabel;
	cocos2d::Size _visibleSize;


	// for MouseJoint
	b2Body *_bottomBody; // 底部的 edgeShape
	b2MouseJoint* _MouseJoint;
	bool _bTouchOn;

	// Box2D Examples
	void readBlocksCSBFile(const char *);
	void readSceneFile(const char *);
	void createStaticBoundary();

	void setupMouseJoint();
	void setupDistanceJoint();
	void setupPrismaticJoint();
	void setupPulleyJoint();
	void setupGearJoint();
	void setupWeldJoint();
	void setupRopeJoint();
	b2Body* wheelbodyB;
	b2Body* dynamicBody[4];//tooth
	b2Body* dynamicBodyTooth[16];//tooth
	bool _bSmoking,_bToothL,_bToothR;
	void setupCar();
	void setupBall();
	CContactListener2 _contactListener;
	b2Body* SensorBody[3], *bodyCar;
	b2Body* bodyBall;
	cocos2d::Sprite *_collisionSprite;
	CButton *_ballBtn,*_triBtn;
	void setupSensor();
	void setupBallbtn();
	void setupTribtn();
	//door
	void setupDoor();
	cocos2d::Sprite *doorSprite[2];
	b2Body* doorStaticBody[2];
	int ballNum,ballNum2;
	b2Body *ballBody[2];
	bool ball1, ball2;
	void setupTri();//triangle
	//brush
	list<cocos2d::Point>plataformPoints;     //記錄每一個位置的list
	cocos2d::Sprite *_brush[NUMBER_BRUSH];     //宣告有3000個圖片的陣列

											   //三個list分別記錄，可使用、正在用、以建立剛體
	list<Sprite*>_FreeList;
	list<Sprite*>_InUsedList;
	list<Sprite*>_BoxList;
	float _iFree;
	float _iInUsed;
	int _boxuse;

	b2BodyDef brushBodyDef;
	b2FixtureDef brushfixtureDef;
	b2Body* newBody;
	b2Body* brushBody[2];
	int brush_i;
	bool _bdraw,thirdPart;
#ifdef BOX2D_DEBUG
	//DebugDraw
	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
#endif

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
	void doStep(float dt);

	cocos2d::EventListenerTouchOneByOne *_listener1;
	bool onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰結束事件 

    // implement the "static create()" method manually
    CREATE_FUNC(JointScene);
};

#endif // __JointScene_SCENE_H__
