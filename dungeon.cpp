#include "dungeon.h"
#include "game.h"
#include "tile.h"
#include <vector>
#include <set>
#include <iostream>
#include <string>

Dungeon::Dungeon(Game* game)
	:mGame(game)
	,mRoomCount(3)
	,mSize(50)
	,mTileSize(64)
{
}

void Dungeon::GenerateLevel()
{
	// pick which rooms to use
	PickRooms();
	// place rooms randomly - close but without overlap
	PlaceRooms();
	// create paths between them
	Passageways();
	// draw rooms and place nodes?
	DrawRooms();
}

void Dungeon::PickRooms()
{
	std::cout << "Picking Rooms";

	std::vector<class Room*> mRoomTypes = mGame->GetRoomTypes();

	// entrance
	// exit
	// boss
	// shop
	// normal

	for (int i = 0; i < mRoomCount; i++)
	{
		int _random = rand() % 3;
		class Room* newRoom = new Room();
		newRoom->size = mRoomTypes[_random]->size;
		newRoom->characters = mRoomTypes[_random]->characters;
		mRooms.push_back(newRoom);
		std::cout << ".";
	}

	std::cout << "Done!" << std::endl;
}

void Dungeon::PlaceRooms()
{
	std::cout << "Placing Rooms";

	class Room* lastRoom = nullptr;
	std::vector<class Room*> positionedRooms;

	// for each room
	for (Room* r : mRooms)
	{
		bool intersect = true;
		int attempts = 0;
		while (intersect)
		{
			if (attempts > 100)
			{
				std::cerr << std::endl;
				std::cerr << "Error: over 100 room placement attempts" << std::endl;
				std::cerr << "Stopping at " + std::to_string(positionedRooms.size()) + " rooms" << std::endl;
				return;
			}

			// pick random position that doesn't overlap
			int _randX = (rand() % mSize) * mTileSize;
			int _randY = (rand() % mSize) * mTileSize;

			r->position = Vector2(_randX, _randY);

			intersect = false;
			for (Room* other : positionedRooms)
			{
				intersect = Intersect(r, other);
				if (intersect)
					break;
			}
			attempts++;
		}
		positionedRooms.push_back(r);

		// set next room
		r->nextRoom = lastRoom;
		lastRoom = r;

		std::cout << ".";
	}

	std::cout << "Done!" << std::endl;
}

void Dungeon::Passageways()
{
	std::cout << "Initializing Node Network";

	// set up node grid as empty grid
	std::vector<std::vector<class Node*>> nodeGrid;
	for (int j = 0; j < mSize; j++)
	{
		std::vector<class Node*> row;
		nodeGrid.push_back(row);
		for (int i = 0; i < mSize; i++)
		{
			nodeGrid[j].push_back(nullptr);
		}
	}
	std::cout << ".";
	
	// used to check intersections with rooms
	Room* tempRoom = new Room();
	tempRoom->size = Vector2(1, 1);

	// place all nodes
	for (int j = 0; j < mSize; j++)
	{
		for (int i = 0; i < mSize; i++)
		{
			// set position
			Vector2 pos;
			pos.x = i * mTileSize;
			pos.y = j * mTileSize;
			// apply to temp room
			tempRoom->position = pos;
			
			// check if that position intersects with other rooms
			bool inRoom = false;
			for (class Room* r : mRooms)
			{
				if (Intersect(tempRoom, r))
				{
					// if this room doesn't have an exit
					if (r->exit == nullptr)
					{
						// place a node in the room and make it the exit
						Node* _node = new Node();
						_node->pos = pos;
						nodeGrid[j][i] = _node;
						mNodes.push_back(_node);
						r->exit = _node;
					}
					inRoom = true;
					break; // stop iterating over rooms
				}
			}
			if (inRoom)
				continue; // skip to next loop

			// normal node
			Node* _node = new Node();
			_node->pos = pos;
			mNodes.push_back(_node);

			// add to node grid
			nodeGrid[j][i] = _node;
		}
	}
	std::cout << ".";

	// setting neighboring nodes
	for (int j = 0; j < mSize; j++)
	{
		for (int i = 0; i < mSize; i++)
		{
			if (nodeGrid[j][i] != nullptr) {
				if (i > 0)
				{
					if (nodeGrid[j][i - 1] != nullptr)
					{
						nodeGrid[j][i]->neighbors.push_back(nodeGrid[j][i - 1]);
					}
				}
				if (i < mSize - 1)
				{
					if (nodeGrid[j][i + 1] != nullptr)
					{
						nodeGrid[j][i]->neighbors.push_back(nodeGrid[j][i + 1]);
					}
				}
				if (j > 0)
				{
					if (nodeGrid[j - 1][i] != nullptr)
					{
						nodeGrid[j][i]->neighbors.push_back(nodeGrid[j - 1][i]);
					}
				}
				if (j < mSize - 1)
				{
					if (nodeGrid[j + 1][i] != nullptr)
					{
						nodeGrid[j][i]->neighbors.push_back(nodeGrid[j + 1][i]);
					}
				}
			}
		}
	}
	std::cout << ".";

	std::cout << "Done!" << std::endl;

	std::cout << "Pathfinding";

	//std::cout << "Visualizing all " + std::to_string(mNodes.size()) + " nodes";
	
	// for each room
	// pathfind from exit to exit of next room
	for (class Room* r : mRooms)
	{
		if (r->nextRoom == nullptr)
			continue;

		// add this path to the path vector
		mPaths.push_back(PathFind(r->exit, r->nextRoom->exit));
		std::cout << ".";
	}
	/*
	int nodeCount = 0;
	for (class Node* n : mNodes)
	{
		class Tile* _tile = new Tile(mGame);
		_tile->SetPosition(n->pos);
		_tile->GetComponent<SpriteComponent>()->SetTexture(mGame->GetTexture("assets/dungeon/wall.png"));
		_tile->SetScale(1.0f);

		nodeCount++;
		if (nodeCount % 100 == 0)
			std::cout << ".";
	}
	*/
	std::cout << "Done!" << std::endl;
}

std::vector<class Node*> Dungeon::PathFind(class Node* start, class Node* end)
{
	// open set
	std::set<class Node*> openSet;
	openSet.emplace(start);

	// closed set
	//std::set<class Node*> closedSet;

	// initialize node values
	for (class Node* node : mNodes)
	{
		node->previousNode = nullptr;
		node->gScore = INT32_MAX;
		node->fScore = INT32_MAX;
	}

	// initialize starting node
	start->gScore = 0;
	// cost is manhattan dist
	start->fScore = abs(end->pos.x - start->pos.x) + abs(end->pos.y - start->pos.y);

	while (!openSet.empty())
	{
		// current node is the one with the lowest total cost

		// find node in open set with lowest score
		Node* currentNode = *openSet.begin();
		int minScore = currentNode->fScore;
		for (auto it = openSet.begin()++; it != openSet.end(); it++)
		{
			if (currentNode->fScore < minScore)
			{
				currentNode = *it;
				minScore = currentNode->fScore;
			}
		}

		if (currentNode == end)
		{
			// the end hurray!
			// reconstruct path
			std::vector<class Node*> path;
			path.push_back(currentNode);

			class Node* tempNode = currentNode;
			while (tempNode->previousNode != nullptr)
			{
				tempNode = tempNode->previousNode;
				path.push_back(tempNode);
			}
			return path;
		}

		// add current to closed set
		//closedSet.emplace(currentNode);
		
		// remove current node from open set
		openSet.erase(openSet.find(currentNode));

		// looking through neighbors
		for (class Node* neighbor : currentNode->neighbors)
		{
			int gScore = currentNode->gScore + 1;
			if (gScore < neighbor->gScore)
			{
				// best path I guess
				neighbor->previousNode = currentNode;
				neighbor->gScore = gScore;
				neighbor->fScore = neighbor->gScore + abs(end->pos.x - neighbor->pos.x) + abs(end->pos.y - neighbor->pos.y); // gscore plus manhattan dist

				// if neighbor isnt in closed set, add him to open set?
				//if (closedSet.find(neighbor) == closedSet.end())
					//openSet.emplace(neighbor);
				if (openSet.find(neighbor) == openSet.end())
					openSet.emplace(neighbor);
			}
		}
	}

	std::cerr << std::endl;
	std::cerr << "Error: No path found" << std::endl;
	std::vector<class Node*> emptyPath;
	return emptyPath;
}

void Dungeon::DrawRooms()
{
	std::cout << "Drawing Rooms...";

	// progress bar
	int roomsDrawn = 0;

	for (class Room* r : mRooms)
	{
		for (int j = 0; j < r->size.y; j++)
		{
			for (int i = 0; i < r->size.x; i++)
			{
				class Tile* _tile = new Tile(mGame);
				Vector2 _pos;
				_pos.x = r->position.x + (i * mTileSize);
				_pos.y = r->position.y + (j * mTileSize);
				_tile->SetPosition(_pos);
				mTiles.push_back(_tile);

				char character = r->characters[j][i];
				switch (character)
				{
				case 'w':
					_tile->GetComponent<SpriteComponent>()->SetTexture(mGame->GetTexture("assets/dungeon/wall.png"));
					break;
				case '.':
					_tile->GetComponent<SpriteComponent>()->SetTexture(mGame->GetTexture("assets/dungeon/floor.png"));
					break;
				default:
					break;
				}
			}
		}

		// progress bar
		roomsDrawn++;
		//ProgressBar((float)roomsDrawn / (float)mRoomCount, "Drawing Rooms:");
		std::cout << ".";
	}
	std::cout << "Done!" << std::endl;

	std::cout << "Drawing Passageways...";

	// draw passageways
	int passagewaysDrawn = 0;
	for (std::vector<class Node*> path : mPaths)
	{
		for (class Node* node : path)
		{
			// adding tile
			class Tile* _tile = new Tile(mGame);
			_tile->SetPosition(node->pos);
			mTiles.push_back(_tile);
			// setting sprite
			_tile->GetComponent<SpriteComponent>()->SetTexture(mGame->GetTexture("assets/dungeon/floor.png"));

		}
		// progress bar
		passagewaysDrawn++;
		//ProgressBar((float)passagewaysDrawn / (float)mPaths.size(), "Drawing Paths:");
		std::cout << ".";
	}

	std::cout << "Done!" << std::endl;
}

bool Dungeon::Intersect(class Room* room_a, class Room* room_b)
{
	bool case1 = GetMax(room_a).x < GetMin(room_b).x;
	bool case2 = GetMax(room_b).x < GetMin(room_a).x;
	bool case3 = GetMax(room_a).y < GetMin(room_b).y;
	bool case4 = GetMax(room_b).y < GetMin(room_a).y;
	if (!case1 && !case2 && !case3 && !case4) {
		return true;
	}
	else {
		return false;
	}
}

Vector2 Dungeon::GetMin(Room* room)
{
	Vector2 min = room->position;
	return min;
}

Vector2 Dungeon::GetMax(Room* room)
{
	Vector2 max;
	max.x = room->position.x + (room->size.x * mTileSize);
	max.y = room->position.y + (room->size.y * mTileSize);
	return max;
}

void Dungeon::ProgressBar(float percent, std::string string)
{
	system("cls");
	std::cout << string << std::endl;
	std::cout << " | ";

	percent *= 100;

	std::string progress;
	for (int i = 0; i < 100; i++)
	{
		if (i <= percent)
			progress += ">";
		else
			progress += " ";
	}
	std::cout << progress + " | " + std::to_string((int)percent) + "%" << std::endl;

	if ((int)percent >= 100)
	{
		std::cout << "Complete!" << std::endl;
		std::cout << "\n" << std::endl;
		std::cout << "\n" << std::endl;
	}
}