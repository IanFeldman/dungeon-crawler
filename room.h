#pragma once
#include "actor.h"

class Room : public Actor
{
public:
	Room(class Game* game, Vector2 size, const char* fileName);
	Vector2 GetSize() { return mSize; }
	const char* GetFileName() { return mFileName; }

	void AddWall(class Wall* wall) { mWalls.push_back(wall); }
	std::vector<class Wall*> GetWalls() { return mWalls; }

	void AddNode(class Node* node) { mNodes.push_back(node); }
	std::vector<class Node*> GetNodes() { return mNodes; }

private:
	class SpriteComponent* mSpriteComponent;
	class CollisionComponent* mCollisionComponent;
	Vector2 mSize;
	const char* mFileName;
	std::vector<class Wall*> mWalls;
	std::vector<class Node*> mNodes;
};