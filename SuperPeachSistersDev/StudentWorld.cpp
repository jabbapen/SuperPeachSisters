#include "StudentWorld.h"
#include "GameConstants.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <list>
#include <sstream>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h, and Actor.cpp

int StudentWorld::init()
{
    // load the current level
    int levelNumber = this->getLevel();
    string fileName = getLevelFileName(levelNumber);
    Level* level = new Level(assetPath());
    int result = level->loadLevel(fileName);
    if (result != Level::load_success) 
    {
        // as long as the first level can be loaded the game is won when the next level file is not found
        if (result == Level::load_fail_file_not_found) cerr << "ERROR: Could not find level " << levelNumber << " file '" << fileName << "'.";
        if (result == Level::load_fail_bad_format) cerr << "ERROR: Bad format error in level " << levelNumber << " file '" << fileName << "'.";
        return GWSTATUS_LEVEL_ERROR;
    }

    // load the actors in the level
    m_player = 0;
    for (int gy = 0; gy < GRID_HEIGHT; gy++)
    {
        for (int gx = 0; gx < GRID_WIDTH; gx++)
        {
            Level::GridEntry entry = level->getContentsOf(gx, gy);
            if (entry == Level::GridEntry::empty) continue;
            switch (entry)
            {
            case Level::GridEntry::empty:
                break;
            case Level::GridEntry::block:
                addActor(new BlockActor(this, gx * SPRITE_WIDTH, gy * SPRITE_HEIGHT));
                break;
            case Level::GridEntry::star_goodie_block:
                addActor(new StarGoodieBlockActor(this, gx * SPRITE_WIDTH, gy * SPRITE_HEIGHT));
                break;
            case Level::GridEntry::flower_goodie_block:
                addActor(new FlowerGoodieBlockActor(this, gx * SPRITE_WIDTH, gy * SPRITE_HEIGHT));
                break;
            case Level::GridEntry::mushroom_goodie_block:
                addActor(new MushroomGoodieBlockActor(this, gx * SPRITE_WIDTH, gy * SPRITE_HEIGHT));
                break;
            case Level::GridEntry::pipe:
                addActor(new PipeActor(this, gx * SPRITE_WIDTH, gy * SPRITE_HEIGHT));
                break;
            case Level::GridEntry::koopa:
                addActor(new KoopaEnemyActor(this, gx * SPRITE_WIDTH, gy * SPRITE_HEIGHT));
                break;
            case Level::GridEntry::goomba:
                addActor(new GoombaEnemyActor(this, gx * SPRITE_WIDTH, gy * SPRITE_HEIGHT));
                break;
            case Level::GridEntry::piranha:
                addActor(new PiranhaEnemyActor(this, gx * SPRITE_WIDTH, gy * SPRITE_HEIGHT));
                break;
            case Level::GridEntry::peach:
                m_player = new PeachActor(this, gx * SPRITE_WIDTH, gy * SPRITE_HEIGHT);
                addActor(m_player);
                break;
            case Level::GridEntry::flag:
                addActor(new FlagPlayerTargetActor(this, gx * SPRITE_WIDTH, gy * SPRITE_HEIGHT));
                break;
            case Level::GridEntry::mario:
                addActor(new MarioPlayerTargetActor(this, gx * SPRITE_WIDTH, gy * SPRITE_HEIGHT));
                break;
            }
        }
    }
    startLevel();
    updateGameStats();
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    for (auto actorIterator = m_actors.begin(); actorIterator != m_actors.end(); ++actorIterator)
    {
        Actor* actor = *actorIterator;
        
        if (actor->isAlive())
        {
            actor->doSomething();
        }

        if (m_playerDied) 
        {
            decLives();
            playSound(SOUND_PLAYER_DIE);
            return GWSTATUS_PLAYER_DIED;
        }
        
        if (m_levelCompleted) 
        {
            playSound(SOUND_FINISHED_LEVEL);
            return GWSTATUS_FINISHED_LEVEL;
        }
        
        if(m_playerWon)
        {
            playSound(SOUND_GAME_OVER);
            return GWSTATUS_PLAYER_WON;
        }
    }

    removeDeadActors();

    updateGameStats();

    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::startLevel() 
{
    m_levelCompleted = false;
    m_playerDied = false;
    m_playerWon = false;
}

void StudentWorld::cleanUp()
{
    for (auto actorIterator = m_actors.begin(); actorIterator != m_actors.end(); ++actorIterator)
        delete *actorIterator;
    m_actors.clear();
}

void StudentWorld::addActor(Actor* actor)
{
    m_actors.push_back(actor);
}

void StudentWorld::removeDeadActors()
{
    list<Actor*>::const_iterator actorIterator = m_actors.cbegin();
    while (actorIterator != m_actors.cend())
    {
        if (!((*actorIterator)->isAlive()))
        {
            delete* actorIterator;
            actorIterator = m_actors.erase(actorIterator);
        }
        else {
            ++actorIterator;
        }
    }
}

bool StudentWorld::damageActorsTouching(Actor* actor)
{
    bool damageDone = false;
    for (auto actorIterator = m_actors.begin(); actorIterator != m_actors.end(); ++actorIterator)
    {
        if (*actorIterator != actor)
        {
            if ((*actorIterator)->isOverlapping(actor))
            {
                if (actor->damage(*actorIterator))
                {
                    damageDone = true;
                }
            }
        }
    }
    return damageDone;
}

bool StudentWorld::anyOtherBlockingActorsAt(Actor* thisActor, double x, double y) 
{
    for (auto actorIterator = m_actors.begin(); actorIterator != m_actors.end(); ++actorIterator)
    {
        if (*actorIterator != thisActor)
        {
            if ((*actorIterator)->isBlocking()) 
            {
                if ((*actorIterator)->isOverlappingSpace(x, y, SPRITE_WIDTH, SPRITE_HEIGHT))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool StudentWorld::isSpaceUnderActorAt(Actor* actor, double x, double y, double width, int steps)
{
    for (auto actorIterator = m_actors.begin(); actorIterator != m_actors.end(); ++actorIterator)
    {
        if ((*actorIterator != actor) && (*actorIterator)->isBlocking())
        {
            if ((*actorIterator)->isOverlappingSpace(x, y - steps, width, 1))
            {
                return false;
            }
        }
    }
    return true;
}

bool StudentWorld::moveActor(Actor* actor, double x, double y)
{
    bool canMove = true;
    for (auto actorIterator = m_actors.begin(); actorIterator != m_actors.end(); ++actorIterator)
    {
        if (*actorIterator != actor)
        {
            if ((*actorIterator)->isOverlappingSpace(x, y, SPRITE_WIDTH, SPRITE_HEIGHT))
            {
                actor->bonk(*actorIterator);
                if ((*actorIterator)->isBlocking())
                {
                    canMove = false;
                }
            }
        }
    }
    if (canMove) 
    {
        actor->moveTo(x, y);
    }
    return canMove;
}

void StudentWorld::updateGameStats()
{
    std::stringstream stream;
    stream << "Lives: " << getLives();
    stream << "  Level: " << std::setw(2) << std::setfill('0') << getLevel();
    stream << "  Points: " << std::setw(6) << std::setfill('0') << getScore();
    if (m_player->hasStarPower()) stream << " StarPower!";
    if (m_player->hasShootPower()) stream << " ShootPower!";
    if (m_player->hasJumpPower()) stream << " JumpPower!";
    setGameStatText(stream.str());
}

std::string StudentWorld::getLevelFileName(int level)
{
    if (level < MINIMUM_LEVEL || level > MAXIMUM_LEVEL) return std::string();
    std::stringstream stream;
    stream << "level" << std::setw(2) << std::setfill('0') << level << ".txt";
    return std::string(stream.str());
}
