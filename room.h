#pragma once
#include "actor.h"
#include <SDL.h>
#include "spritecomponent.h"

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

	void AddRoom(class Room* room) { mConnectedRooms.push_back(room); }
	std::vector<class Room*> GetConnectedRooms() { return mConnectedRooms; }

	void SetSprite(SDL_Texture* texture) { mSprite = texture; mSpriteComponent->SetTexture(texture); }
	SDL_Texture* GetSprite() { return mSprite; }

private:
	class SpriteComponent* mSpriteComponent;
	class CollisionComponent* mCollisionComponent;
	SDL_Texture* mSprite;
	Vector2 mSize;
	const char* mFileName;
	std::vector<class Wall*> mWalls;
	std::vector<class Node*> mNodes;
	std::vector<class Room*> mConnectedRooms;
};