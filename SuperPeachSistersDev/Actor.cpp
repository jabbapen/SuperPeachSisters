#include "Actor.h"

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp

// Actor abstract class common functions
//
bool Actor::isOverlappingSpace(double x, double y, double width, double height) const
{
	double x1 = getX();
	if ((x1 >= (x + width)) || ((x1 + SPRITE_WIDTH) <= x))
	{
		return false;
	}

	double y1 = getY();
	if ((y1 >= (y + height)) || ((y1 + SPRITE_HEIGHT) <= y))
	{
		return false;
	}

	return true;
}

void Actor::reverseDirection() 
{
	int direction = (getDirection() == DIRECTION_LEFT) ? DIRECTION_RIGHT : DIRECTION_LEFT;
	setDirection(direction);
}

void Actor::setDirection(int direction) 
{
	int graphDirection;
	if (direction == DIRECTION_RANDOM) 
	{
		graphDirection = ((rand() % 2) == 0) ? DIRECTION_LEFT : DIRECTION_RIGHT;
	}
	else 
	{
		graphDirection = direction;
	}
	GraphObject::setDirection(graphDirection);
}

bool Actor::move(int steps)
{
	double x = getX();
	double y = getY();
	switch (getDirection())
	{
	case DIRECTION_LEFT:
		x -= steps;
		break;
	case DIRECTION_RIGHT:
		x += steps;
		break;
	}
	return getWorld()->moveActor(this, x, y);
}

bool Actor::jump(int distance)
{
	double x = getX();
	double y = getY();
	if (!getWorld()->anyOtherBlockingActorsAt(this, x, y - 1))
	{
		return false;
	}
	m_remainingJumpDistance = distance;
	getWorld()->playSound(SOUND_PLAYER_JUMP);
	return true;
}

bool Actor::doJumping(int steps)
{
	if (m_remainingJumpDistance <= 0) return false;
	double x = getX();
	double y = getY() + steps;
	if (!getWorld()->moveActor(this, x, y)) 
	{
		m_remainingJumpDistance = 0;
		getWorld()->playSound(SOUND_PLAYER_BONK);
		return false;
	}
	--m_remainingJumpDistance;
	return true;
}

bool Actor::doFalling(int steps, int moveSteps, bool mustMove)
{
	double x = getX();
	double y = getY();
	if (getWorld()->isSpaceUnderActorAt(this, x, y, SPRITE_WIDTH, steps))
	{
		if (moveSteps != 0) 
		{
			moveSteps = abs(moveSteps);
			if (getDirection() == DIRECTION_LEFT)
			{
				moveSteps = -moveSteps;
			}
			if (getWorld()->anyOtherBlockingActorsAt(this, x + moveSteps, y - steps))
			{
				if (mustMove) 
				{
					return false;
				}
				moveSteps = 0;
			}
		}
		return getWorld()->moveActor(this, x + moveSteps, y - steps);
	}
	return false;
}

// PLAYER ACTOR abstract class

void PlayerActor::giveScore(int points) { getWorld()->increaseScore(points); }

// PEACH Actor
//
void PeachActor::doSomething()
{
	if (!isAlive())
	{
		return;
	}
	doInvincibility();
	doShootRecharging();
	if (!doJumping(JUMP_STEPS))
	{
		doFalling(FALL_STEPS);
	}
	doUserInput();
}

void PeachActor::doInvincibility()
{
	if (hasStarPower())
	{
		m_starPowerTicks--;
	}
	if (hasInvincibility())
	{
		m_tempInvincibilityTicks--;
	}
}

void PeachActor::doShootRecharging() 
{
	if (m_shootRechargingTicks > 0) 
	{
		m_shootRechargingTicks--;
	}
}

bool PeachActor::bonk(Actor* actor)
{
	if (!actor->isAlive())
	{
		return false;
	}
	return actor->bonkedBy(this);
}

bool PeachActor::bonkedBy(Actor* actor)
{
	if ((m_starPowerTicks > 0) || (m_tempInvincibilityTicks > 0))
	{
		return false;
	}
    m_hitPoints -= BONKED_DAMAGE_POINTS;
	m_tempInvincibilityTicks = TEMP_INVINCIBILITY_TICKS;
	m_shootPower = false;
	m_jumpPower = false;
	if (m_hitPoints > 0)
	{
		getWorld()->playSound(SOUND_PLAYER_HURT);
	}
	else
	{
		setAlive(false);
		getWorld()->setPlayerDied();
	}
	return true;
}

bool PeachActor::doUserInput() 
{
	bool didSomething = false;
	// Check if user pressed a key
	int key;
	if (getWorld()->getKey(key))
	{
		switch (key)
		{
		case KEY_PRESS_LEFT:
			setDirection(DIRECTION_LEFT);
			move(MOVE_STEPS);
			didSomething = true;
			break;
		case KEY_PRESS_RIGHT:
			setDirection(DIRECTION_RIGHT);
			move(MOVE_STEPS);
			didSomething = true;
			break;
		case KEY_PRESS_UP: 
			{
				int distance = m_jumpPower ? JUMP_POWER_DISTANCE : JUMP_REGULAR_DISTANCE;
				jump(distance);
			}
			didSomething = true;
			break;
		case KEY_PRESS_SPACE:
			if (m_shootPower && (m_shootRechargingTicks < 1))
			{
				getWorld()->playSound(SOUND_PLAYER_FIRE);
				m_shootRechargingTicks = SHOOT_RECHARGE_TICKS;
				int x = (int)getX();
				if (getDirection() == DIRECTION_LEFT)
				{
					x -= (FIREBALL_START_DISTANCE);
				}
				else
				{
					x += (FIREBALL_START_DISTANCE);
				}
				getWorld()->addActor(new PeachFireballActor(getWorld(), x, (int)getY(), getDirection()));
			}
			didSomething = true;
			break;
		}
	}
	return didSomething;
}

// PLAYER TARGET abstract actor
//
void PlayerTargetActor::doSomething() 
{
	if (!isAlive())
	{
		return;
	}
	PlayerActor* player = getWorld()->getPlayer();
	if (isOverlapping(player))
	{
		doPlayerTargetAction(player);
	}
}

// FLAG PLAYER TARGET ACTOR
//
void FlagPlayerTargetActor::doPlayerTargetAction(PlayerActor* player)
{
	player->giveScore(SCORE);
	setAlive(!ALIVE);
	getWorld()->setLevelCompleted();
}

// MARIO PLAYER TARGET ACTOR
//
void MarioPlayerTargetActor::doPlayerTargetAction(PlayerActor* player) 
{
	player->giveScore(SCORE);
	setAlive(!ALIVE);
	getWorld()->setPlayerWon();
}

// OBSTACLE ACTOR
//
bool ObstacleActor::bonkedBy(Actor* actor)
{
	if (actor != getWorld()->getPlayer())
	{
		return false;
	}
	getWorld()->playSound(SOUND_PLAYER_BONK);
	return true;
}

// GOODIE BLOCK abstract actor
//
bool GoodieBlockActor::bonkedBy(Actor* actor)
{
	if (actor != getWorld()->getPlayer()) 
	{
		return false;
	}
	if (hasItem())
	{
		useItem();
		StudentWorld* world = getWorld();
		Actor* goodie = createGoodie(world, (int)getX(), (int)(getY() + SPRITE_HEIGHT));
		world->addActor(goodie);
		world->playSound(SOUND_POWERUP_APPEARS);
	}
	else
	{
		getWorld()->playSound(SOUND_PLAYER_BONK);
	}
	return true;
}

// STAR GOODIE BLOCK Actor
//
Actor* StarGoodieBlockActor::createGoodie(StudentWorld* world, int x, int y) { return new StarGoodieActor(world, x, y); }

// FLOWER GOODIE BLOCK Actor
//
Actor* FlowerGoodieBlockActor::createGoodie(StudentWorld* world, int x, int y) { return new FlowerGoodieActor(world, x, y); }

// MUSHROOM GOODIE BLOCK Actor
//
Actor* MushroomGoodieBlockActor::createGoodie(StudentWorld* world, int x, int y) { return new MushroomGoodieActor(world, x, y); }

// GOODIE Actor abstract class
//
bool GoodieActor::doMove() 
{
	doFalling(FALL_STEPS);
	double x = getX();
	double y = getY();
	if (getDirection() == DIRECTION_LEFT)
	{
		x -= MOVE_STEPS;
	}
	else
	{
		x += MOVE_STEPS;
	}
	if (getWorld()->anyOtherBlockingActorsAt(this, x, y))
	{
		reverseDirection();
		return false;
	}
	return getWorld()->moveActor(this, x, y);
}

void GoodieActor::finishPowerUp() 
{
	setAlive(false);
	getWorld()->playSound(SOUND_PLAYER_POWERUP);
}

void GoodieActor::doSomething() 
{
	if (!isAlive())
	{
		return;
	}
	PeachActor* peach = getWorld()->getPlayer();
	if (isOverlapping(peach))
	{
		giveGoodiesTo(peach);
		finishPowerUp();
	}
	doMove();
}

// FLOWER Actor
//
void FlowerGoodieActor::giveGoodiesTo(PeachActor* peach) 
{
	peach->giveScore(SCORE);
	peach->giveShootPower();
	peach->giveHitPoints(HIT_POINTS);
}

// MUSHROOM Actor
//
void MushroomGoodieActor::giveGoodiesTo(PeachActor* peach)
{
	peach->giveScore(SCORE);
	peach->giveJumpPower();
	peach->giveHitPoints(HIT_POINTS);
}

// STAR Actor
//
void StarGoodieActor::giveGoodiesTo(PeachActor* peach)
{
	peach->giveScore(SCORE);
	peach->giveStarPower(STAR_POWER);
}

// ENEMY ACTOR abstract class
//
bool EnemyActor::bonk(Actor* actor) 
{
	if (!actor->isAlive())
	{
		return false;
	}
	return actor->bonkedBy(this);
}

bool EnemyActor::bonkedBy(Actor* actor) 
{
	PeachActor* peach = getWorld()->getPlayer();
	if (actor != peach)
	{
		return false;
	}
	if (!peach->hasStarPower()) 
	{
		return false;
	}
	getWorld()->playSound(SOUND_PLAYER_KICK);
	return damagedBy(this);
}

bool EnemyActor::damagedBy(Actor* actor) 
{
	getWorld()->getPlayer()->giveScore(SCORE);
	setAlive(!ALIVE);
	return true;
}

bool EnemyActor::doBonkIfOverlapping(Actor* actor)
{
	if (isOverlapping(actor))
	{
		return bonk(actor);
	}
	return false;
}

bool EnemyActor::doMove() 
{
	double x = getX();
	double y = getY();
	double dx;
	if (getDirection() == DIRECTION_LEFT)
	{
		x -= MOVE_STEPS;
		dx = x;
	}
	else
	{
		x += MOVE_STEPS;
		dx = x + SPRITE_WIDTH;
	}
	if (getWorld()->anyOtherBlockingActorsAt(this, x, y)) 
	{
		reverseDirection();
	}
	else if (getWorld()->isSpaceUnderActorAt(this, dx, y, 1, 1)) 
	{
		reverseDirection();
	}
	return getWorld()->moveActor(this, x, y);
}

// GOOMBA Actor
//
void GoombaEnemyActor::doSomething()
{
	if (!isAlive())
	{
		return;
	}
	if (doBonkIfOverlapping(getWorld()->getPlayer()))
	{
		return;
	}
	doMove();
}

// KOOPA Actor
//
void KoopaEnemyActor::doSomething()
{
	if (!isAlive())
	{
		return;
	}
	doBonkIfOverlapping(getWorld()->getPlayer());
	doMove();
}

bool KoopaEnemyActor::bonkedBy(Actor* actor) 
{
	if (!EnemyActor::bonkedBy(actor)) 
	{
		return false;
	}
	getWorld()->addActor(new ShellActor(getWorld(), (int)getX(), (int)getY(), getDirection()));
	return true;
}

bool KoopaEnemyActor::damagedBy(Actor* actor)
{
	if (!EnemyActor::damagedBy(actor))
	{
		return false;
	}
	getWorld()->addActor(new ShellActor(getWorld(), (int)getX(), (int)getY(), getDirection()));
	return true;
}

// PIRANHA Actor//
void PiranhaEnemyActor::doSomething()
{
	if (!isAlive())
	{
		return;
	}
	increaseAnimationNumber();
	PeachActor* peach = getWorld()->getPlayer();
	if (doBonkIfOverlapping(peach))
	{
		return;
	}
	if (!doTurnTowards(peach)) 
	{
		return;
	}
	doFireAt(peach);
}

bool PiranhaEnemyActor::doFireAt(Actor* actor)
{
	if (m_firingDelay > 0)
	{
		m_firingDelay--;
		return false;
	}
	else
	{
		double distance = abs(getX() - actor->getX());
		if (distance >= FIRING_RANGE)
		{
			return false;
		}
		m_firingDelay = FIRING_DELAY;
		getWorld()->playSound(SOUND_PIRANHA_FIRE);
		getWorld()->addActor(new PiranhaFireballActor(getWorld(), (int)getX(), (int)getY(), getDirection()));
	}
	return true;
}

bool PiranhaEnemyActor::doTurnTowards(Actor* actor)
{
	if (actor->getY() < (getY() - DETECTION_HEIGHT))
	{
		return false;
	}
	if (actor->getY() >= (getY() + DETECTION_HEIGHT))
	{
		return false;
	}
	int actorDirection = (actor->getX() < getX()) ? DIRECTION_LEFT : DIRECTION_RIGHT;
	setDirection(actorDirection);
	return true;
}

// TEMPORARY Actor common functions
//
bool TemporaryActor::doMove()
{
	doFalling(FALL_STEPS);
	double x = getX();
	double y = getY();
	if (getDirection() == DIRECTION_LEFT)
	{
		x -= MOVE_STEPS;
	}
	else
	{
		x += MOVE_STEPS;
	}
	if (getWorld()->anyOtherBlockingActorsAt(this, x, y))
	{
		setAlive(!ALIVE);
		return false;
	}
	return getWorld()->moveActor(this, x, y);
}

void PiranhaFireballActor::doSomething()
{
	PeachActor* peach = getWorld()->getPlayer();
	if (isOverlapping(peach))
	{
		if (damage(peach))
		{
			setAlive(!ALIVE);
			return;
		}
	}
	doMove();
}

bool PiranhaFireballActor::damage(Actor* actor)
{
	if (!actor->isAlive()) 
	{
		return false;
	}
	return actor->damagedBy(this);
}

void PeachFireballActor::doSomething()
{
	if (getWorld()->damageActorsTouching(this))
	{
		setAlive(!ALIVE);
		return;
	}
	doMove();
}

bool PeachFireballActor::damage(Actor* actor) 
{
	if (getWorld()->getPlayer() == actor) 
	{
		return false;
	}
	return actor->damagedBy(this);
}

void ShellActor::doSomething()
{
	if (getWorld()->damageActorsTouching(this))
	{
		setAlive(!ALIVE);
		return;
	}
	doMove();
}

bool ShellActor::damage(Actor* actor)
{
	if (!actor->isAlive())
	{
		return false;
	}
	if (getWorld()->getPlayer() == actor)
	{
		return false;
	}
	return actor->damagedBy(this);
}
