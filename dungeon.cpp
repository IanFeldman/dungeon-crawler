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

Dungeon::Dungeon(Game* game)
	:mGame(game)
	,mRoomCount(100)
	,mDungeonSize(300, 300)
	,mCsvSize(mGame->GetCsvSize())
{
	mDungeonSize.x *= mCsvSize.x;
	mDungeonSize.y *= mCsvSize.y;
}

void Dungeon::GenerateLevel()
{
	// Initialize node grid
	PlaceNodes();
	// Pick which room types to use
	PickRooms();
	//Place rooms
	PlaceRooms();
	//ConnectRooms();
	//Pathfind();
}

void Dungeon::PlaceNodes()
{
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
		}

		mNodeGrid.push_back(row);
	}

	std::cout << "Done!" << std::endl;
}

void Dungeon::PickRooms()
{
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
		mRooms.push_back(Clone(mRoomTypes[random]));
	}

	std::cout << "Done!" << std::endl;
}

Room* Dungeon::Clone(Room* room)
{
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
	std::cout << "Placing Rooms..." << std::endl;

	std::vector<Room*> positionedRooms;

	bool dungeonFull = false;
	// for each room
	for (Room* r : mRooms)
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
			int xIndex = rand() % (int)(mNodeGrid[0].size() - r->GetSize().x); // row length
			int yIndex = rand() % (int)(mNodeGrid.size() - r->GetSize().y); // number of columns

			Vector2 roomPos = mNodeGrid[xIndex][yIndex]->GetPosition();
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

		// position walls (for debugging)
		for (Wall* w : r->GetWalls())
			w->SetPosition(w->GetRelativePos() + r->GetPosition());
	}
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

/*
void Dungeon::DrawRooms()
{
	std::cout << "Drawing Rooms..." << std::endl;

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
				_pos.x = r->position.x + (i * mTileSize.x);
				_pos.y = r->position.y + (j * mTileSize.y);
				_tile->SetPosition(_pos);
				mTiles.push_back(_tile);

				char character = r->characters[j][i];
				switch (character)
				{
				case 'w':
					_tile->GetComponent<SpriteComponent>()->SetTexture(mGame->GetTexture("assets/dungeon/wall.png"));
					_tile->SetCollisionComponent(mTileSize * 0.5f);
					mGame->AddCollidable(_tile);
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
	}
	std::cout << "Done!" << std::endl;
}


OpenSide Dungeon::GetEntranceDir(OpenSide prevExitDir)
{
	switch (prevExitDir)
	{
	case OpenSide::North:
		return OpenSide::South;
		break;
	case OpenSide::South:
		return OpenSide::North;
		break;
	case OpenSide::East:
		return OpenSide::West;
		break;
	case OpenSide::West:
		return OpenSide::East;
		break;
	default:
		std::cerr << "Error: previous room has no exit" << std::endl;
		break;
	}
}

Vector2 Dungeon::SetRoomPosition(struct Room* currRoom, struct Room* prevRoom)
{
	Vector2 exitPos;
	Vector2 pos;
	switch (currRoom->entranceDir)
	{
		// put room below previous
	case OpenSide::North:
		exitPos = prevRoom->position + Vector2(floor(prevRoom->size.x * 0.5f) * mTileSize.x, prevRoom->size.y * mTileSize.y);
		pos = exitPos + Vector2(-floor(currRoom->size.x * 0.5f) * mTileSize.x, 0.0f);
		break;
		// put room above previous
	case OpenSide::South:
		exitPos = prevRoom->position + Vector2(floor(prevRoom->size.x * 0.5f) * mTileSize.x, 0.0f);
		pos = exitPos + Vector2(-floor(currRoom->size.x * 0.5f) * mTileSize.x, -currRoom->size.y * mTileSize.y);
		break;
		// put room to left of previous
	case OpenSide::East:
		exitPos = prevRoom->position + Vector2(0.0f, floor(prevRoom->size.y * 0.5f) * mTileSize.y);
		pos = exitPos + Vector2(-currRoom->size.x * mTileSize.x, -floor(currRoom->size.y * 0.5f) * mTileSize.y);
		break;
		// put room to right of previous
	case OpenSide::West:
		exitPos = prevRoom->position + Vector2(prevRoom->size.x * mTileSize.x, floor(prevRoom->size.y * 0.5f) * mTileSize.y);
		pos = exitPos + Vector2(0.0f, -floor(currRoom->size.y * 0.5f) * mTileSize.y);
		break;
	default:
		std::cerr << "Error: Current room has no entrance" << std::endl;
		// will probably break, because pos will stay uninitialized
		break;
	}

	return pos;
}
*/

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