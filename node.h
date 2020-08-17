#pragma once
#include "actor.h"

class Node : public Actor
{
public:
	Node(class Game* game);

	// getters and setters
	class Room* GetRoom() { return mParentRoom; }
	void SetRoom(class Room* parent) { mParentRoom = parent; }

	std::vector<class Node*> GetNeighbors() { return mNeighbors; }
	void AddNeighbor(class Node* neighbor) { mNeighbors.push_back(neighbor); }

	class Node* GetPreviousNode() { return mPreviousNode; }
	void SetPreviousNode(class Node* node) { mPreviousNode = node; }

	int GetFScore() { return mFScore; }
	void SetFScore(float score) { mFScore = score; }

	int GetGScore() { return mGScore; }
	void SetGScore(float score) { mGScore = score; }

	int GetHScore() { return mHScore; }
	void SetHScore(float score) { mHScore = score; }

protected:
	class CollisionComponent* mCollisionComponent;
	class SpriteComponent* mSpriteComponent;

	bool mPlaced;
	class Room* mParentRoom; // the room that this node is underneath

	std::vector<class Node*> mNeighbors;
	class Node* mPreviousNode;
	float mFScore; // total cost
	float mGScore; // dist to get to this node
	float mHScore; // heuristic
};