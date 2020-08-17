#include "dungeon.h"
#include "game.h"
#include "node.h"
#include "room.h"
#include "wall.h"
#include "collisioncomponent.h"
#include "spritecomponent.h" // unneeded
#include "player.h" // unneeded
#include <vector>
#include <set>
#include <iostream>
#include <string>
#include <map>

Dungeon::Dungeon(Game* game)
	:mGame(game)
	,mRoomCount(10)
	,mDungeonSize(3200, 3200)
	,mCsvSize(mGame->GetCsvSize())
{
}

void Dungeon::GenerateLevel()
{
	PlaceNodes();
	PickRooms();
	PlaceRooms();
	ConnectRooms();
	FindPaths();
}

void Dungeon::PlaceNodes()
{
	// set up node grid with as many nodes as can fit in the dungeon size
	// nodes are each 3 csv tiles by 3 csv tiles (96 world units)
	std::cout << "Placing Nodes..." << std::endl;

	// offset start by a little bit, and go up by three wall tiles
	for (int j = int(mCsvSize.y * 1.5f); j < mDungeonSize.y; j += int(3.0f * mCsvSize.y))
	{
		std::vector<Node*> row;

		for (float i = int(mCsvSize.x * 1.5f); i < mDungeonSize.x; i += int(3.0f * mCsvSize.x))
		{
			Node* node = new Node(mGame);
			node->SetPosition(Vector2(i, j));
			row.push_back(node);
			// initialize the node map
			mNodesUsed.emplace(node, false);
		}

		mNodeGrid.push_back(row);
	}

	// get node neighbors
	for (int j = 0; j < mNodeGrid.size(); j++)
	{
		for (int i = 0; i < mNodeGrid[0].size(); i++)
		{
			if (mNodeGrid[j][i] != nullptr) {
				if (i > 0)
				{
					if (mNodeGrid[j][i - 1] != nullptr)
					{
						mNodeGrid[j][i]->AddNeighbor(mNodeGrid[j][i - 1]);
					}
				}
				if (i < mNodeGrid[0].size() - 1)
				{
					if (mNodeGrid[j][i + 1] != nullptr)
					{
						mNodeGrid[j][i]->AddNeighbor(mNodeGrid[j][i + 1]);
					}
				}
				if (j > 0)
				{
					if (mNodeGrid[j - 1][i] != nullptr)
					{
						mNodeGrid[j][i]->AddNeighbor(mNodeGrid[j - 1][i]);
					}
				}
				if (j < mNodeGrid.size() - 1)
				{
					if (mNodeGrid[j + 1][i] != nullptr)
					{
						mNodeGrid[j][i]->AddNeighbor(mNodeGrid[j + 1][i]);
					}
				}
			}
		}
	}

	std::cout << "Done!" << std::endl;
}

void Dungeon::PickRooms()
{
	// create mRoomCount number of rooms, and make each one a clone of a random room type initialized in mgame
	std::cout << "Picking Rooms..." << std::endl;

	std::vector<Room*> mRoomTypes = mGame->GetRoomTypes();

	// entrance
	// exit
	// boss
	// shop
	// normal

	for (int i = 0; i < mRoomCount; i++)
	{
		int random = rand() % mRoomTypes.size();
		mInitialRooms.push_back(Clone(mRoomTypes[random]));
	}

	std::cout << "Done!" << std::endl;
}

Room* Dungeon::Clone(Room* room)
{
	// create a new room that is nearly identical to room

	Room* newRoom = new Room(mGame, room->GetSize(), nullptr);

	// clone walls also
	for (Wall* w : room->GetWalls())
	{
		Wall* newWall = new Wall(mGame, w->GetRelativePos(), room);
		newRoom->AddWall(newWall);
	}

	return newRoom;
}

void Dungeon::PlaceRooms()
{
	// place each room at a random position that lines up with node grid
	// avoid overlapping rooms and delete rooms that can't be fit in
	// associate each room with the nodes that are in it
	std::cout << "Placing Rooms..." << std::endl;

	std::vector<Room*> positionedRooms;

	bool dungeonFull = false;
	// for each room
	for (Room* r : mInitialRooms)
	{
		// delete the extra rooms
		if (dungeonFull)
		{
			// destroy room and its walls
			DestroyRoom(r);
			continue;
		}

		bool intersect = true;
		int placeAttempts = 0;
		while (intersect)
		{
			int nodeGridWidth = mNodeGrid[0].size();
			int nodeGridHeight = mNodeGrid.size();
			// pick random position
			int xIndex = rand() % (int)(nodeGridWidth - (r->GetSize().x / 3.0f)); // row length
			int yIndex = rand() % (int)(nodeGridHeight - (r->GetSize().y / 3.0f)); // number of columns

			Vector2 roomPos = mNodeGrid[yIndex][xIndex]->GetPosition();
			// set corner to node position
			roomPos += (Vector2((r->GetSize().x * 0.5f - 1.5f) * mCsvSize.x, (r->GetSize().y * 0.5f - 1.5f) * mCsvSize.y));
			r->SetPosition(roomPos);

			// check overlap
			intersect = false;
			for (Room* other : positionedRooms)
			{
				CollisionComponent* thisCC = r->GetComponent<CollisionComponent>();
				CollisionComponent* otherCC = other->GetComponent<CollisionComponent>();

				intersect = thisCC->Intersect(otherCC);
				if (intersect)
					break;
			}

			// check if dungeon is full
			if (intersect)
				placeAttempts++;

			if (placeAttempts > 500)
			{
				std::cerr << "Error: over 500 room placement attempts" << std::endl;
				std::cout << "Stopping at " + std::to_string(positionedRooms.size()) + " rooms" << std::endl;
				// destroy this room and its walls
				DestroyRoom(r);
				// continue to all the next rooms and destroy them
				dungeonFull = true;
				intersect = false; // just to break out of the while loop
			}
		}
		// on the first loop that dungeonfull, make sure to skip adding room to vector
		if (dungeonFull)
			continue;

		// associate rooms and nodes
		FindNodes(r);

		positionedRooms.push_back(r);
		mRooms.push_back(r);

		// position walls (for debugging)
		for (Wall* w : r->GetWalls())
			w->SetPosition(w->GetRelativePos() + r->GetPosition());
	}

	mInitialRooms.clear();
	std::cout << "Done!" << std::endl;
}

void Dungeon::DestroyRoom(Room* room)
{
	// destroy room's walls
	for (Wall* w : room->GetWalls())
		w->SetState(ActorState::Destroy);

	 // destroy room
	room->SetState(ActorState::Destroy);
}

void Dungeon::FindNodes(Room* room)
{
	// finds all the nodes within a room's bounds
	// adds nodes to a vector in room
	// adds room to variable in nodes

	// x coord of left edge
	int leftEdge = room->GetPosition().x - room->GetSize().x * 0.5f * mCsvSize.x;
	// x coord in node lengths (three csv sizes)
	// the left edge is in between nodes so we always want to round up
	int minNodeX = ceil(leftEdge / (3.0f * mCsvSize.x));

	// y coord of top edge
	int topEdge = room->GetPosition().y - room->GetSize().y * 0.5f * mCsvSize.y;
	// y coord in node lengths (three csv sizes)
	// the top edge is in between nodes so we always want to round up
	int minNodeY = ceil(topEdge / (3.0f * mCsvSize.y));

	// set max nodes
	int maxNodeX = minNodeX + floor(room->GetSize().x / 3.0f); // divide room size because it is in csv units and nodes are 3 csv units long
	if (maxNodeX > mNodeGrid[0].size())
		maxNodeX = mNodeGrid[0].size();

	int maxNodeY = minNodeY + floor(room->GetSize().y / 3.0f);
	if (maxNodeY > mNodeGrid.size())
		maxNodeY = mNodeGrid.size();

	// set nodes
	for (int j = minNodeY; j < maxNodeY; j++)
	{
		for (int i = minNodeX; i < maxNodeX; i++)
		{
			Node* node = mNodeGrid[j][i];
			node->SetRoom(room);
			node->GetComponent<SpriteComponent>()->SetTexture(mGame->GetTexture("assets/debug/blue.png"));
			room->AddNode(node);
		}
	}
}

void Dungeon::ConnectRooms()
{
	// connect all rooms with minimum spanning tree
	std::cout << "Connecting Rooms..." << std::endl;

	// construct initial graph
	std::set<Room*> graphVertices; // set of rooms
	std::map <std::set<Room*>, float> graphEdges; // map of edges as keys, and cost as entry

	for (Room* r : mRooms)
	{
		// add all rooms to vertex set
		graphVertices.emplace(r);
		
		// create edges
		for (Room* other : mRooms)
		{
			// ignore if rooms are the same
			if (other == r)
				continue;

			// create edge
			std::set<Room*> edge;
			edge.emplace(r);
			edge.emplace(other);
			// cost is distance
			float cost = (other->GetPosition() - r->GetPosition()).Length();
			// add to edges map
			graphEdges.emplace(edge, cost);
		}
	}

	// prims algorithm finds min spanning tree
	std::set<std::set<Room*>> tree; // set of edges that will be the final tree
	std::set<Room*> treeVerts; // set of vertices in the tree
	
	// start with a vertex
	Room* initVertex = *graphVertices.begin();
	treeVerts.emplace(initVertex);

	while (tree.size() < graphVertices.size() - 1)
	{
		std::set<std::set<Room*>> potentialEdges;

		// for every vertex in the tree
		for (Room* v : treeVerts)
		{
			// for every edge in the graph
			for (auto it = graphEdges.begin(); it != graphEdges.end(); it++)
			{
				std::set<Room*> graphEdge = it->first;
				if (graphEdge.find(v) != graphEdge.end()) // if edge contains init vertex, add edge to potential edges
					potentialEdges.emplace(graphEdge);
			}
		}

		// weed out the edges that are already in the tree
		std::set<std::set<Room*>> tempEdges = potentialEdges;
		potentialEdges.clear();
		// for all potential edges
		for (std::set<Room*> edge : tempEdges)
		{
			bool inTree = (tree.find(edge) != tree.end());
			
			int vertsInTree = 0;
			for (Room* v : edge)
			{
				// find edge in tree verts
				if (treeVerts.find(v) != treeVerts.end())
					vertsInTree++;
			}

			// if the edge is not in the tree and only one vertex is part of the tree, keep it
			if (!inTree && vertsInTree == 1)
				potentialEdges.emplace(edge);
		}

		// pick cheapest potential edge
		int minCost = INT32_MAX;
		std::set<Room*> cheapestEdge;
		for (std::set<Room*> edge : potentialEdges) // iterate over potential edges to find the cheapest one
		{
			if (graphEdges[edge] < minCost)
			{
				cheapestEdge = edge;
				minCost = graphEdges[edge];
			}
		}
		if (!cheapestEdge.empty())
		{
			// add edge to tree and add verts to treeVerts
			tree.emplace(cheapestEdge);
			for (Room* v : cheapestEdge)
				treeVerts.emplace(v);
		}
		else
		{
			std::cerr << "Could not find cheapest edge" << std::endl;
			return;
		}
	}

	// add back in some edges
	int addedEdgeCount = 0.1f * graphEdges.size();
	for (int i = 0; i < addedEdgeCount; i++)
	{
		int random = rand() % graphEdges.size();

		auto it = graphEdges.begin();
		std::advance(it, random);

		std::set<Room*> edge = it->first;

		// only add it if it isn't already in tree
		if (tree.find(edge) == tree.end())
			tree.emplace(edge);
	}
	mMinSpanningTree = tree;

	std::cout << "Done!" << std::endl;
}

void Dungeon::FindPaths()
{
	std::cout << "Pathfinding..." << std::endl;

	for (std::set<Room*> edge : mMinSpanningTree)
	{
		std::vector<Node*> path;

		auto it = edge.begin();
		Room* a = *it;
		it++;
		Room* b = *it;
		
		std::pair<Node*, Node*> nodes = GetStartAndEndNodes(a, b);
		if (nodes.first == nullptr || nodes.second == nullptr)
			continue;
		path = Pathfind(nodes.first, nodes.second);

		for (Node* n : path)
		{
			n->GetComponent<SpriteComponent>()->SetTexture(mGame->GetTexture("assets/debug/green.png"));
		}
	}
	std::cout << "Done!" << std::endl;
}

std::pair<Node*, Node*> Dungeon::GetStartAndEndNodes(class Room* start, class Room* end)
{
	std::pair<Node*, Node*> nodes = std::make_pair(nullptr, nullptr);

	// get potential starting nodes
	std::vector<Node*> potentialStartNodes;
	for (Node* n : start->GetNodes())
	{
		// the node has to be unused and have a usable neighbor
		bool nodeUsed = true;
		bool usableNeighbors = false;
		nodeUsed = mNodesUsed[n];
		for (Node* neighbor : n->GetNeighbors())
		{
			// at least one of the neighbors is not part of the room and isn't used
			if (neighbor->GetRoom() == nullptr && !mNodesUsed[neighbor])
			{
				usableNeighbors = true;
				break;
			}
		}

		if (!nodeUsed && usableNeighbors)
			potentialStartNodes.push_back(n);
	}
	if (potentialStartNodes.empty())
	{
		std::cerr << "No available starting nodes" << std::endl;
		return nodes;
	}

	// same process for end nodes
	std::vector<Node*> potentialEndNodes;
	for (Node* n : end->GetNodes())
	{
		// the node has to be unused and on the edge
		bool nodeUsed = true;
		bool usableNeighbors = false;
		nodeUsed = mNodesUsed[n];
		for (Node* neighbor : n->GetNeighbors())
		{
			// at least one of the neighbors is not part of the room and isn't used
			if (neighbor->GetRoom() == nullptr && !mNodesUsed[neighbor])
				usableNeighbors = true;
		}

		if (!nodeUsed && usableNeighbors)
			potentialEndNodes.push_back(n);
	}
	if (potentialEndNodes.empty())
	{
		std::cerr << "No available starting nodes" << std::endl;
		return nodes;
	}

	// get closest combination of start and end node
	float minDist = INT32_MAX;
	Node* startNode = nullptr;
	Node* endNode = nullptr;
	for (Node* start : potentialStartNodes)
	{
		for (Node* end : potentialEndNodes)
		{
			float dist = (end->GetPosition() - start->GetPosition()).Length();

			if (dist < minDist)
			{
				minDist = dist;
				startNode = start;
				endNode = end;
			}
		}
	}
	if (startNode == nullptr || endNode == nullptr)
	{
		std::cerr << "Start and/or end node are null" << std::endl;
		return nodes;
	}
	startNode->GetComponent<SpriteComponent>()->SetTexture(mGame->GetTexture("assets/debug/yellow.png"));
	endNode->GetComponent<SpriteComponent>()->SetTexture(mGame->GetTexture("assets/debug/yellow.png"));

	nodes = std::make_pair(startNode, endNode);
	return nodes;
}

// a star
std::vector<Node*> Dungeon::Pathfind(Node* startNode, Node* endNode)
{
	// to push back if there is an error
	std::vector<Node*> emptyPath;

	// open set
	std::set<Node*> openSet;
	openSet.emplace(startNode);

	std::set<Node*> closedSet;

	// initialize starting node
	startNode->SetFScore(0.0f);
	
	while (!openSet.empty())
	{
		// current node is the one with the lowest total cost
		// find node in open set with lowest score
		Node* currentNode = nullptr;
		int minScore = INT32_MAX;
		for (Node* n : openSet)
		{
			if (n->GetFScore() < minScore) {
				currentNode = n;
				minScore = n->GetFScore();
			}
		}
		// remove current node from open set
		if (openSet.find(currentNode) != openSet.end())
			openSet.erase(currentNode);
		// add current to closed set
		closedSet.emplace(currentNode);
		// check if end
		if (currentNode == endNode)
		{
			// the end
			// reconstruct path
			std::vector<Node*> path;
			path.push_back(currentNode);

			Node* tempNode = currentNode;
			while (tempNode->GetPreviousNode() != nullptr)
			{
				tempNode = tempNode->GetPreviousNode();
				path.push_back(tempNode);
			}

			for (Node* n : path)
			{
				mNodesUsed[n] = true;
			}
			return path;
		}

		// look through neighbors
		for (Node* neighbor : currentNode->GetNeighbors())
		{
			// if neighbor is in closed set, skip
			if (closedSet.find(neighbor) != closedSet.end())
				continue;
			// if neighbor is associated with a room and that neighbor is not the final node, then skip this neighbor
			if (neighbor->GetRoom() != nullptr && neighbor != endNode)
			{
				closedSet.emplace(neighbor);
				continue;
			}
			// if the neighbor is already used, skip to next neighbor
			if (mNodesUsed[neighbor])
			{
				closedSet.emplace(neighbor);
				continue;
			}

			int gScore = currentNode->GetGScore() + mCsvSize.x * 3.0f;
			int hScore = abs(endNode->GetPosition().x - neighbor->GetPosition().x) + abs(endNode->GetPosition().y - neighbor->GetPosition().y);
			neighbor->SetGScore(gScore);
			neighbor->SetHScore(hScore);
			neighbor->SetFScore(gScore + hScore);

			neighbor->SetPreviousNode(currentNode);

			// if neighbor is not in open set, add him
			if (openSet.find(neighbor) == openSet.end())
			{
				openSet.emplace(neighbor);
			}
		}
	}

	std::cerr << "No path found" << std::endl;
	return emptyPath;
}