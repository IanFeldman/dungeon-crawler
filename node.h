#pragma once
#include "actor.h"

class Node : public Actor
{
public:
	Node(class Game* game);

	// getters and setters
	void SetRoom(class Room* parent) { mParentRoom = parent; }

	std::vector<class Node*> GetNeighbors() { return mNeighbors; }
	void AddNeighbor(class Node* neighbor) { mNeighbors.push_back(neighbor); }

	class Node* GetPreviousNode() { return mPreviousNode; }
	void SetPreviousNode(class Node* node) { mPreviousNode = node; }

	int GetFScore() { return mFScore; }
	void SetFScore(int score) { mFScore = score; }

	int GetGScore() { return mGScore; }
	void SetGScore(int score) { mGScore = score; }

protected:
	class CollisionComponent* mCollisionComponent;
	class SpriteComponent* mSpriteComponent;

	bool mPlaced;
	class Room* mParentRoom; // the room that this node is underneath

	std::vector<class Node*> mNeighbors;
	class Node* mPreviousNode;
	int mFScore; // total cost
	int mGScore; // dist to get to this node
};