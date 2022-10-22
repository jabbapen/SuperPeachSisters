#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "GameConstants.h"
#include "StudentWorld.h"

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

// Forward declarations needed because of cyclic dependency between this an StudentWorld;
class StudentWorld;

// Classes defined in this file
class Actor;

class GameActionActor;
class PeachActor;
class FlagPlayerTargetActor;
class MarioPlayerTargetActor;

class ObstacleActor;			
class PipeActor;
class BlockActor;
class GoodieBlockActor;
class StarGoodieBlockActor;
class FlowerGoodieBlockActor;
class MushroomGoodieBlockActor;

class GoodieActor;
class StarGoodieActor;
class FlowerGoodieActor;
class MushroomGoodieActor;

class EnemyActor;				
class GoombaEnemyActor;
class KoopaEnemyActor;
class PiranhaEnemyActor;

class TemporaryActor;
class PirahnaFireballActor;
class PeachFireballActor;
class ShellActor;

// Constants
//
const int DIRECTION_RIGHT = 0;
const int DIRECTION_LEFT = 180;
const int DIRECTION_RANDOM = -1;
const int DEPTH_BOTTOM = 0;
const int DEPTH_TOP = 3;
const double DEFAULT_SIZE = 1.0;
const bool BLOCKING = true;
const bool ALIVE = true;
const bool DAMAGABLE = true;
const bool PLAYER = true;
const bool PLAYER_TARGET = true;

// Actor abstract class with common functions
//
class Actor : public GraphObject
{
public:
	Actor(StudentWorld* world, int iid, int x, int y, int dir, int depth, double size, bool blocking, bool damagable,
		bool player = !PLAYER, bool playerTarget = !PLAYER_TARGET)
		: GraphObject(iid, x, y, DIRECTION_RIGHT, depth, size),
		m_world(world), m_blocking(blocking), m_damagable(damagable), m_player(player), m_playerTarget(playerTarget),
		m_alive(ALIVE)
	{ 
		setDirection(dir);
	}

	virtual void doSomething() = 0;
	virtual bool bonk(Actor* actor) { return false; }
	virtual bool bonkedBy(Actor* actor) { return false; }
	virtual bool damage(Actor* actor) { return false; }
	virtual bool damagedBy(Actor* actor) { return false; }

	bool isOverlappingSpace(double x, double y, double width, double height) const;
	bool isOverlapping(Actor* actor) const { return isOverlappingSpace(actor->getX(), actor->getY(), SPRITE_WIDTH, SPRITE_HEIGHT); };

	bool isDamagable() const { return m_damagable; }
	bool isBlocking() const { return m_blocking; }
	bool isAlive() const { return m_alive; }

	bool isPlayer() { return m_player; }
	bool isPlayerTarget() { return m_playerTarget; }
	virtual void targetAction(Actor* actor) { }

protected:
	StudentWorld* getWorld() { return m_world; }
	void reverseDirection();
	void setDirection(int direction);
	void setAlive(bool alive) { m_alive = alive; }

	bool move(int steps);
	bool jump(int distance);
	bool doJumping(int steps);
	bool doFalling(int steps, int moveSteps = 0, bool mustMove = false);

private:
	StudentWorld* m_world;
	bool m_player;
	bool m_playerTarget;

	int m_remainingJumpDistance = 0;
	bool m_blocking;
	bool m_damagable;
	bool m_alive;
};

// PlayerActor abstract class
//
class PlayerActor : public Actor {
public:
	PlayerActor(StudentWorld* world, int iid, int x, int y, int direction, int depth, double size, bool blocking, bool damagable)
		: Actor(world, iid, x, y, direction, depth, size, blocking, damagable) { }
	virtual void doSomething() = 0;
	void giveScore(int points);
	virtual bool doUserInput() = 0;
};

// PEACH Actor
//
class PeachActor : public PlayerActor {
public:
	PeachActor(StudentWorld* world, int x, int y) 
		: PlayerActor(world, IID_PEACH, x, y, DIRECTION_RIGHT, DEPTH_BOTTOM, DEFAULT_SIZE, !BLOCKING, DAMAGABLE) { }
	void doSomething();
	bool bonk(Actor* actor);
	bool bonkedBy(Actor* actor);
	bool damagedBy(Actor* actor) { return bonkedBy(actor); }

	void giveStarPower(int ticks) { m_starPowerTicks += ticks; }
	void giveShootPower() { m_shootPower = true; }
	void giveJumpPower() { m_jumpPower = true; }
	void giveHitPoints(int points) { m_hitPoints = points; }

	bool hasStarPower() { return m_starPowerTicks > 0; }
	bool hasJumpPower() { return m_jumpPower; }
	bool hasShootPower() { return m_shootPower; }
	bool hasInvincibility() { return m_tempInvincibilityTicks > 0; }

private:
	bool doUserInput();
	const int MOVE_STEPS = 4;
	const int JUMP_REGULAR_DISTANCE = 8;
	const int JUMP_POWER_DISTANCE = 12;
	const int JUMP_STEPS = 4;
	const int FALL_STEPS = 4;

	void doInvincibility();
	const int TEMP_INVINCIBILITY_TICKS = 10;
	int m_starPowerTicks = 0;
	int m_tempInvincibilityTicks = 0;

	void doShootRecharging();
	bool m_shootPower = false;
	const int SHOOT_RECHARGE_TICKS = 8;
	int m_shootRechargingTicks = 0;
	const int FIREBALL_START_DISTANCE = 4;

	bool m_jumpPower = false;

	const int BONKED_DAMAGE_POINTS = 1;
	int m_hitPoints = 1;
};

// PlayerTargetActor abstract class
//
class PlayerTargetActor : public Actor {
public:
	PlayerTargetActor(StudentWorld* world, int iid, int x, int y, int direction, int depth, double size, bool blocking, bool damagable)
		: Actor(world, iid, x, y, direction, depth, size, blocking, damagable) { }
	void doSomething();
private:
	virtual void doPlayerTargetAction(PlayerActor* player) = 0;
};

// FLAG Actor
//
class FlagPlayerTargetActor : public PlayerTargetActor {
public:
	FlagPlayerTargetActor(StudentWorld* world, int x, int y) 
		: PlayerTargetActor(world, IID_FLAG, x, y, DIRECTION_RIGHT, DEPTH_BOTTOM + 1, DEFAULT_SIZE, !BLOCKING, !DAMAGABLE) { }
private:
	void doPlayerTargetAction(PlayerActor* player);
	const int SCORE = 1000;
};

// MARIO Actor
//
class MarioPlayerTargetActor : public PlayerTargetActor {
public:
	MarioPlayerTargetActor(StudentWorld* world, int x, int y) 
		: PlayerTargetActor(world, IID_MARIO, x, y, DIRECTION_RIGHT, DEPTH_BOTTOM + 1, DEFAULT_SIZE, !BLOCKING, !DAMAGABLE) { }
private:
	void doPlayerTargetAction(PlayerActor* player);
	const int SCORE = 1000;
};

// OBSTACLE Actor abstract class
//
class ObstacleActor : public Actor {
public:
	ObstacleActor(StudentWorld* world, int iid, int x, int y) 
		: Actor(world, iid, x, y, DIRECTION_RIGHT, DEPTH_BOTTOM + 2, DEFAULT_SIZE, BLOCKING, !DAMAGABLE) { }
	void doSomething() { }
	bool bonkedBy(Actor* actor);
};

// PIPE Actor
//
class PipeActor : public ObstacleActor {
public:
	PipeActor(StudentWorld* world, int x, int y) : ObstacleActor(world, IID_PIPE, x, y) { }
};

// BLOCK Actor
//
class BlockActor : public ObstacleActor {
public:
	BlockActor(StudentWorld* world, int x, int y) : ObstacleActor(world, IID_BLOCK, x, y) { }
};

// GOODIE BLOCK Actor abstract class
//
class GoodieBlockActor : public BlockActor
{
public:
	GoodieBlockActor(StudentWorld* world, int x, int y, int items = 1) : BlockActor(world, x, y), m_items(items) { }
	bool bonkedBy(Actor* actor);
private:
	virtual Actor* createGoodie(StudentWorld* world, int x, int y) = 0;
	bool hasItem() const { return m_items > 0; }
	void useItem() { m_items--; }
	int m_items;
};

// STAR BLOCK Goodie Actor
//
class StarGoodieBlockActor : public GoodieBlockActor
{
public:
	StarGoodieBlockActor(StudentWorld* world, int x, int y) : GoodieBlockActor(world, x, y) { }
private:
	Actor* createGoodie(StudentWorld* world, int x, int y);
};

// FLOWER BLOCK Goodie Actor
//
class FlowerGoodieBlockActor : public GoodieBlockActor
{
public:
	FlowerGoodieBlockActor(StudentWorld* world, int x, int y) : GoodieBlockActor(world, x, y) { }
private:
	Actor* createGoodie(StudentWorld* world, int x, int y);
};

// MUSHROOM BLOCK Goodie Actor
//
class MushroomGoodieBlockActor : public GoodieBlockActor
{
public:
	MushroomGoodieBlockActor(StudentWorld* world, int x, int y) : GoodieBlockActor(world, x, y) { }
private:
	Actor* createGoodie(StudentWorld* world, int x, int y);
};

// GOODIE ACTOR abstract class
//
class GoodieActor : public Actor
{
public:
	GoodieActor(StudentWorld* world, int iid, int x, int y) 
		: Actor(world, iid, x, y, DIRECTION_RIGHT, DEPTH_BOTTOM + 1, DEFAULT_SIZE, !BLOCKING, !DAMAGABLE) { }
	void doSomething();
private:
	virtual void giveGoodiesTo(PeachActor* peach) = 0;
	bool doMove();
	void finishPowerUp();
	const int MOVE_STEPS = 2;
	const int FALL_STEPS = 2;
};

// FLOWER Goodie Actor
//
class FlowerGoodieActor : public GoodieActor {
public:
	FlowerGoodieActor(StudentWorld* world, int x, int y) : GoodieActor(world, IID_FLOWER, x, y) { }
private:
	void giveGoodiesTo(PeachActor* peach);
	const int SCORE = 50;
	const bool SHOOT_POWER = true;
	const int HIT_POINTS = 2;
};

// MUSHROOM Goodie Actor
//
class MushroomGoodieActor : public GoodieActor {
public:
	MushroomGoodieActor(StudentWorld* world, int x, int y) : GoodieActor(world, IID_MUSHROOM, x, y) { }
private:
	void giveGoodiesTo(PeachActor* peach);
	const int SCORE = 75;
	const bool JUMP_POWER = true;
	const int HIT_POINTS = 2;
};

// STAR Goodie Actor
//
class StarGoodieActor : public GoodieActor {
public:
	StarGoodieActor(StudentWorld* world, int x, int y) : GoodieActor(world, IID_STAR, x, y) { }
private:
	void giveGoodiesTo(PeachActor* peach);
	const int SCORE = 100;
	const int STAR_POWER = 150;
};


// ENEMY ACTOR abstract case
//
class EnemyActor : public Actor {
public:
	EnemyActor(StudentWorld* world, int iid, int x, int y) 
		: Actor(world, iid, x, y, DIRECTION_RANDOM, DEPTH_BOTTOM, DEFAULT_SIZE, !BLOCKING, DAMAGABLE) { }
	virtual void doSomething() = 0;
	bool bonk(Actor* actor);
	bool bonkedBy(Actor* actor);
	bool damagedBy(Actor* actor);
protected:
	bool doBonkIfOverlapping(Actor* actor);
	bool doMove();
	const int SCORE = 100;
	const int MOVE_STEPS = 1;
};

// GOOMBA Enemy Actor
//
class GoombaEnemyActor : public EnemyActor {
public:
	GoombaEnemyActor(StudentWorld* world, int x, int y) : EnemyActor(world, IID_GOOMBA, x, y) { }
	void doSomething();
private:
};

// KOOPA Enemy Actor
//
class KoopaEnemyActor : public EnemyActor {
public:
	KoopaEnemyActor(StudentWorld* world, int x, int y) : EnemyActor(world, IID_KOOPA, x, y) { }
	void doSomething();
	bool bonkedBy(Actor* actor);
	bool damagedBy(Actor* actor);
private:
	const int MOVE_STEPS = 1;
};

// PIRANHA Enemy Actor
//
class PiranhaEnemyActor : public EnemyActor {
public:
	PiranhaEnemyActor(StudentWorld* world, int x, int y) : EnemyActor(world, IID_PIRANHA, x, y), m_firingDelay(0) { }
	void doSomething();
private:
	bool doTurnTowards(Actor* actor);
	bool doFireAt(Actor* actor);
	const double DETECTION_HEIGHT = 1.5 * SPRITE_HEIGHT;
	const int FIRING_RANGE = 8 * SPRITE_WIDTH;
	const int FIRING_DELAY = 40;
	int m_firingDelay;
};

// TEMPORARY DAMAGING Actor abstract class
//
class TemporaryActor : public Actor {
public:
	TemporaryActor(StudentWorld* world, int iid, int x, int y, int direction)
		: Actor(world, iid, x, y, direction, DEPTH_BOTTOM + 1, DEFAULT_SIZE, !BLOCKING, !DAMAGABLE) { }
    virtual void doSomething() = 0;
protected:
	bool doMove();
	const int MOVE_STEPS = 2;
	const int FALL_STEPS = 2;
};

// SHELL Actor
//
class ShellActor : public TemporaryActor {
public:
	ShellActor(StudentWorld* world, int x, int y, int direction) 
		: TemporaryActor(world, IID_SHELL, x, y, direction) { }
	void doSomething();
	bool damage(Actor* actor);
};

// PEACH Fireball Actor
//
class PeachFireballActor : public TemporaryActor {
public:
	PeachFireballActor(StudentWorld* world, int x, int y, int direction) 
		: TemporaryActor(world, IID_PEACH_FIRE, x, y, direction) { }
	void doSomething();
	bool damage(Actor* actor);
};

// PIRAHNA Fireball Actor
//
class PiranhaFireballActor : public TemporaryActor {
public:
	PiranhaFireballActor(StudentWorld* world, int x, int y, int direction) 
		: TemporaryActor(world, IID_PIRANHA_FIRE, x, y, direction) { }
	void doSomething();
	bool damage(Actor* actor);
};

#endif // ACTOR_H_
