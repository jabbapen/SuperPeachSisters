#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Level.h"
#include "Actor.h"
#include <string>
#include <list>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp
class Actor;
class PeachActor;

class StudentWorld;

const bool GAME_ACTION = true;

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetPath) : GameWorld(assetPath) { }
	~StudentWorld() 
	{ 
		cleanUp(); 
	}

	virtual int init();
	virtual int move();
	virtual void cleanUp();

	void addActor(Actor* actor);
	void removeDeadActors();

	bool anyOtherBlockingActorsAt(Actor* actor, double x = 0, double y = 0);
	bool isSpaceUnderActorAt(Actor* actor, double x, double y, double width = SPRITE_WIDTH, int steps = 1);

	bool moveActor(Actor* actor, double x, double y);
	bool damageActorsTouching(Actor* actor);

	void setLevelCompleted() { m_levelCompleted = true; }
	void setPlayerDied() { m_playerDied = true; }
	void setPlayerWon() { m_playerWon = true; }
	void startLevel();

	void updateGameStats();

	PeachActor* getPlayer() const { return m_player; }

private:
	PeachActor* m_player = 0;
	bool m_levelCompleted = false;
	bool m_playerDied = false;
	bool m_playerWon = false;

	std::string getLevelFileName(int level);
	std::list<Actor*> m_actors;
};

#define MINIMUM_LEVEL 1
#define MAXIMUM_LEVEL 99

#endif // STUDENTWORLD_H_
