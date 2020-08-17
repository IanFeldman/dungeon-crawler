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

//void Dungeon::ConnectRooms()
//{
//	// connect all rooms with minimum spanning tree
//	std::cout << "Connecting Rooms..." << std::endl;
//
//	// construct initial graph
//	std::set<Room*> graphVertices; // set of rooms
//	std::map <std::set<Room*>, float> graphEdges; // map of edges as keys, and cost as entry
//
//	for (Room* r : mRooms)
//	{
//		// add all rooms to vertex set
//		graphVertices.emplace(r);
//		
//		// create edges
//		for (Room* other : mRooms)
//		{
//			// ignore if rooms are the same
//			if (other == r)
//				continue;
//
//			// create edge
//			std::set<Room*> edge;
//			edge.emplace(r);
//			edge.emplace(other);
//			// cost is distance
//			float cost = (other->GetPosition() - r->GetPosition()).Length();
//			// add to edges map
//			graphEdges.emplace(edge, cost);
//		}
//	}
//
//	// prims algorithm finds min spanning tree
//	std::set<std::set<Room*>> tree; // set of edges that will be the final tree
//
//	// start with a vertex
//	Room* initialVertex = *graphVertices.begin();
//
//	// find all edges that contain this vertex
//	std::set<std::set<Room*>> potentialEdges;
//	for (auto it = graphEdges.begin(); it != graphEdges.end(); it++) // iterate over all edges
//	{
//		std::set<Room*> graphEdge = it->first;
//		if (graphEdge.find(initialVertex) != graphEdge.end()) // if edge contains init vertex, add edge to potential edges
//			potentialEdges.emplace(graphEdge); 
//	}
//	int minWeight = INT32_MAX;
//	std::set<Room*> cheapestEdge;
//	for (std::set<Room*> edge : potentialEdges) // iterate over potential edges to find the cheapest one
//	{
//		if (graphEdges[edge] < minWeight)
//		{
//			cheapestEdge = edge;
//			minWeight = graphEdges[edge];
//		}
//	}
//	tree.emplace(cheapestEdge);
//
//	while (tree.size() < graphVertices.size() - 1)
//	{
//		std::set<Room*> treeVertices;
//		// for each edge in the tree
//		for (std::set<Room*> treeEdge : tree)
//		{
//			// for each vertex in each edge
//			for (Room* treeVert : treeEdge)
//			{
//				treeVertices.emplace(treeVert);
//			}
//		}
//
//		// find all edges in graph that contain one of tree's vertices
//		potentialEdges.clear();
//		// for every vertex in the tree
//		for (Room* vertex : treeVertices)
//		{
//			// for every edge in the graph
//			for (auto it = graphEdges.begin(); it != graphEdges.end(); it++)
//			{
//				std::set<Room*> graphEdge = it->first;
//				if (graphEdge.find(initialVertex) != graphEdge.end()) // if edge contains init vertex, add edge to potential edges
//					potentialEdges.emplace(graphEdge);
//			}
//		}
//
//		// weed out the edges that are already in the tree
//		std::set<std::set<Room*>> tempEdges = potentialEdges;
//		potentialEdges.clear();
//		// for all potential edges
//		for (std::set<Room*> edge : tempEdges)
//		{
//			// if the edge is not in the tree, keep it
//			if (tree.find(edge) == tree.end())
//				potentialEdges.emplace(edge);
//		}
//
//		// pick cheapest potential edge
//		minWeight = INT32_MAX;
//		cheapestEdge.clear();
//		for (std::set<Room*> edge : potentialEdges) // iterate over potential edges to find the cheapest one
//		{
//			if (graphEdges[edge] < minWeight)
//			{
//				cheapestEdge = edge;
//				minWeight = graphEdges[edge];
//			}
//		}
//		if (!cheapestEdge.empty())
//			tree.emplace(cheapestEdge);
//		else
//		{
//			std::cerr << "Could not find cheapest edge" << std::endl;
//			return;
//		}
//	}
//
//	// add back in some edges
//	//int addedEdgeCount = 0.15f * graphEdges.size();
//	//for (int i = 0; i < addedEdgeCount; i++)
//	//{
//	//	int random = rand() % graphEdges.size();
//
//	//	auto it = graphEdges.begin();
//	//	std::advance(it, random);
//
//	//	std::set<Room*> edge = it->first;
//
//	//	// only add it if it isn't already in tree
//	//	if (tree.find(edge) == tree.end())
//	//		tree.emplace(edge);
//	//}
//	mMinSpanningTree = tree;
//
//	std::cout << "Done!" << std::endl;
//}

void Dungeon::ConnectRooms()
{
	std::cout << "Connecting Rooms..." << std::endl;
	// set of edges
	std::set<std::set<Room*>> tree;

	Room* initRoom = mRooms[0];

	// find closest room and create shortest edge possible to add to tree
	float minDist = INT32_MAX;
	Room* closestRoom = nullptr;
	for (Room* r : mRooms)
	{
		if (r == initRoom)
			continue;

		float dist = (r->GetPosition() - initRoom->GetPosition()).Length();
		if (dist < minDist)
		{
			closestRoom = r;
			minDist = dist;
		}
	}

	std::set<Room*> firstEdge;
	firstEdge.emplace(initRoom);
	firstEdge.emplace(closestRoom);
	tree.emplace(firstEdge);

	for (int i = 1; i < mRooms.size() - 1; i++)
	{
		// map of edges and their weights
		std::map<std::set<Room*>, float> potentialEdges;

		for (std::set<Room*> edge : tree)
		{
			// get smallest edge with each room in tree
			for (Room* room : edge)
			{
				std::vector<Room*> possibleRooms;
				possibleRooms = mRooms;

				bool inTree = true;
				while (inTree)
				{
					// find closest room for every room
					float min = INT32_MAX;
					Room* closest = nullptr;
					for (Room* r : possibleRooms)
					{
						if (r == room)
							continue;

						float distance = (r->GetPosition() - initRoom->GetPosition()).Length();
						if (distance < min)
						{
							closest = r;
							min = distance;
						}
					}

					std::set<Room*> newEdge;
					newEdge.emplace(room);
					newEdge.emplace(closest);

					// if the new edge is not part of the tree, break and emplace
					if (tree.find(newEdge) == tree.end())
					{
						inTree = false;
						potentialEdges.emplace(newEdge, min);
					}
					else
					{
						// remove the closest room from possible rooms
						auto it = std::find(possibleRooms.begin(), possibleRooms.end(), closest);
						if (it != possibleRooms.end())
							possibleRooms.erase(it);
					}
				}
			}
		}

		// get shortest edge of them all
		std::set<Room*> smallestEdge;
		float shortestLength = INT32_MAX;
		for (auto it = potentialEdges.begin(); it != potentialEdges.end(); it++)
		{
			if (potentialEdges[it->first] < shortestLength)
			{
				smallestEdge = it->first;
				shortestLength = potentialEdges[it->first];
			}
		}
		tree.emplace(smallestEdge);
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

// greedy best-first search
//std::vector<Node*> Dungeon::Pathfind(Room* start, Room* end)
//{
//	// to push back if there is an error
//	std::vector<Node*> emptyPath;
//
//	// get potential starting nodes
//	std::vector<Node*> potentialStartNodes;
//	for (Node* n : start->GetNodes())
//	{
//		// the node has to be unused and on the edge
//		bool nodeUsed = true;
//		bool usableNeighbors = false;
//		nodeUsed = mNodesUsed[n];
//		for (Node* neighbor : n->GetNeighbors())
//		{
//			// at least one of the neighbors is not part of the room and isn't used
//			if (neighbor->GetRoom() == nullptr && !mNodesUsed[neighbor])
//			{
//				usableNeighbors = true;
//				break;
//			}
//		}
//
//		if (!nodeUsed && usableNeighbors)
//			potentialStartNodes.push_back(n);
//	}
//	if (potentialStartNodes.empty())
//	{
//		std::cerr << "No available starting nodes" << std::endl;
//		return emptyPath;
//	}
//
//	// same process for end node
//	std::vector<Node*> potentialEndNodes;
//	for (Node* n : end->GetNodes())
//	{
//		// the node has to be unused and on the edge
//		bool nodeUsed = true;
//		bool usableNeighbors = false;
//		nodeUsed = mNodesUsed[n];
//		for (Node* neighbor : n->GetNeighbors())
//		{
//			// at least one of the neighbors is not part of the room and isn't used
//			if (neighbor->GetRoom() == nullptr && !mNodesUsed[neighbor])
//			{
//				usableNeighbors = true;
//				break;
//			}
//		}
//
//		if (!nodeUsed && usableNeighbors)
//			potentialEndNodes.push_back(n);
//	}
//	if (potentialEndNodes.empty())
//	{
//		std::cerr << "No available starting nodes" << std::endl;
//		return emptyPath;
//	}
//
//	// get closest combination of start and end node
//	float minDist = INT32_MAX;
//	Node* startNode = nullptr;
//	Node* endNode = nullptr;
//	for (Node* start : potentialStartNodes)
//	{
//		for (Node* end : potentialEndNodes)
//		{
//			float dist = (end->GetPosition() - start->GetPosition()).Length();
//
//			if (dist < minDist)
//			{
//				minDist = dist;
//				startNode = start;
//				endNode = end;
//			}
//		}
//	}
//	if (startNode == nullptr || endNode == nullptr)
//	{
//		std::cerr << "Start and/or end node are null" << std::endl;
//		return emptyPath;
//	}
//	startNode->GetComponent<SpriteComponent>()->SetTexture(mGame->GetTexture("assets/debug/yellow.png"));
//	endNode->GetComponent<SpriteComponent>()->SetTexture(mGame->GetTexture("assets/debug/yellow.png"));
//
//	std::vector<Node*> openSet;
//	openSet.push_back(startNode);
//
//	std::vector<Node*> closedSet;
//
//	while (true)
//	{
//		if (openSet.empty())
//		{
//			std::cerr << "No path found" << std::endl;
//			return emptyPath;
//		}
//
//		// depth first search (stack)
//		// Node* currentNode = openSet[openSet.size() - 1];
//		// openSet.pop_back();
//
//		// breadth first search (queue)
//		// Node* currentNode = openSet[0];
//		// openSet.erase(openSet.begin());
//
//		// greedy best first search
//		Node* currentNode;
//		float minDist = INT32_MAX;
//		for (Node* n : openSet)
//		{
//			//float dist = (endNode->GetPosition() - n->GetPosition()).Length();
//			float dist = abs(endNode->GetPosition().x - n->GetPosition().x) + abs(endNode->GetPosition().y - n->GetPosition().y);
//			if (dist < minDist)
//			{
//				currentNode = n;
//				minDist = dist;
//				break;
//			}
//		}
//		// remove current node form open set
//		auto it = std::find(openSet.begin(), openSet.end(), currentNode);
//		if (it != openSet.end())
//		{
//			openSet.erase(it);
//		}
//
//		if (currentNode == endNode)
//		{
//			// the end
//			// reconstruct path
//			std::vector<Node*> path;
//			path.push_back(currentNode);
//
//			Node* tempNode = currentNode;
//			while (tempNode->GetPreviousNode() != nullptr)
//			{
//				tempNode = tempNode->GetPreviousNode();
//				path.push_back(tempNode);
//			}
//
//			for (Node* n : path)
//			{
//				mNodesUsed[n] = true;
//			}
//			std::cout << "Done!" << std::endl;
//			return path;
//		}
//
//		closedSet.push_back(currentNode);
//		for (Node* neighbor : currentNode->GetNeighbors())
//		{
//			// if neighbor in open set, skip
//			auto openIt = std::find(openSet.begin(), openSet.end(), neighbor);
//			if (openIt != openSet.end())
//				continue;
//
//			// if neighbor in closed set, skip
//			auto closedIt = std::find(closedSet.begin(), closedSet.end(), neighbor);
//			if (closedIt != closedSet.end())
//				continue;
//
//			// if neighbor is associated with a room and that neighbor is not the final node, then skip this neighbor
//			if (neighbor->GetRoom() != nullptr && neighbor != endNode)
//			{
//				closedSet.push_back(neighbor);
//				continue;
//			}
//			// if the neighbor is already used, skip to next neighbor
//			else if (mNodesUsed[neighbor])
//			{
//				closedSet.push_back(neighbor);
//				continue;
//			}
//
//			neighbor->SetPreviousNode(currentNode);
//			openSet.push_back(neighbor);
//		}
//	}
//}

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