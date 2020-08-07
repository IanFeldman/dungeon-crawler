#include "dungeon.h"
#include "game.h"
#include "tile.h"
#include <vector>
#include <set>
#include <iostream>
#include <string>

Dungeon::Dungeon(Game* game)
	:mGame(game)
	,mRoomCount(50)
	,mTileSize(64.0f, 64.0f)
{
}

void Dungeon::GenerateLevel()
{
	PlaceRooms();
	DrawRooms();
}

void Dungeon::PlaceRooms()
{
	std::cout << "Placing Rooms..." << std::endl;

	// pick random entrance
	struct Room* entrance;
	int _rand = rand() % mGame->GetEntranceRooms().size();
	entrance = Clone(mGame->GetEntranceRooms()[_rand]);
	entrance->position = Vector2::Zero;
	mRooms.push_back(entrance);

	int intersectingRooms = 0;
	// normal rooms
	for (int i = 0; i < mRoomCount - 2; i++) // minus two for entrance and exit
	{
		// figure out entrance direction of next room based off exit direction of previous
		OpenSide nextEntranceDir = GetEntranceDir(mRooms.back()->exitDir);
		int x = 0;

		// create list of potential rooms with that entrance
		std::vector<struct Room*> potentialNextRooms;
		for (struct Room* r : mGame->GetNormalRooms())
		{
			if (r->entranceDir == nextEntranceDir)
				potentialNextRooms.push_back(r);
		}

		// return if there are no possible rooms
		if (potentialNextRooms.size() == 0)
		{
			std::cerr << "Error: no rooms with suitable entrance" << std::endl;
			return;
		}

		// pick random potential room
		_rand = rand() % potentialNextRooms.size();
		struct Room* currentRoom;
		currentRoom = Clone(potentialNextRooms[_rand]);
		
		// set room position
		currentRoom->position = SetRoomPosition(currentRoom, mRooms.back());

		// check if room intersects
		bool intersect = false;
		for (class Room* r : mRooms)
		{
			if (Intersect(currentRoom, r))
			{
				intersect = true;
				break;
			}
		}

		if (intersect)
		{
			// dont add this room
			// redo previous room, because this direction yields an overlap
			if (!mRooms.empty())
			{
				mRooms.pop_back(); // initial go back a room
				i-=2; // because we're redoing this room and the room before it
				intersectingRooms++;
			}
			else
			{
				std::cerr << "Error: Room vector empty while intersect true. Cannot go back a room" << std::endl;
				mRooms.clear();
				return;
			}

			/*
			// go back more rooms based off of how many intersects we have done
			int normalRoomCount = mGame->GetNormalRooms().size();
			int roomsToRetrace = intersectingRooms % normalRoomCount;
			for (int i = 0; i < roomsToRetrace; i++)
			{
				if (mRooms.empty())
					break;
				mRooms.pop_back();
				i--;
			}
			*/
		}
		else
		{
			// continue as normal
			// add room
			mRooms.push_back(currentRoom);
		}
		
		if (mRooms.empty())
		{
			std::cerr << "Error: No more rooms" << std::endl;
			return;
		}

		if (intersectingRooms >= 100)
		{
			std::cerr << "Error: Cannot find non-overlapping setup. Reconstructing dungeon" << std::endl;
			mRooms.clear();
			PlaceRooms();
			return;
		}
	}

	// exit

	// get exit room with correct entrance side
	OpenSide exitRoomEntrance = GetEntranceDir(mRooms.back()->exitDir);

	// create list of potential rooms with that entrance
	std::vector<struct Room*> potentialExits;
	for (struct Room* r : mGame->GetExitRooms())
	{
		if (r->entranceDir == exitRoomEntrance)
			potentialExits.push_back(r);
	}

	// return if there are no possible rooms
	if (potentialExits.empty())
	{
		std::cerr << "Error: no exits with suitable entrance. Skipping exit room." << std::endl;
		return;
	}

	// pick random potential exit
	struct Room* exit;
	_rand = rand() % potentialExits.size();
	exit = Clone(potentialExits[_rand]);
	// set position
	exit->position = SetRoomPosition(exit, mRooms.back());
	// check if it intersects
	bool exitIntersect = false;
	for (class Room* r : mRooms)
	{
		if (Intersect(exit, r))
		{
			exitIntersect = true;
			break;
		}
	}
	// if it intersects, delete the previous room and make that the exit. One room is lost
	if (exitIntersect)
	{
		std::cout << "Exit intersects. Redrawing..." << std::endl;
		// dont add the exit
		// make the previous room the exit

		mRooms.pop_back(); // remove prev room

		// get exit room with correct entrance side
		OpenSide exitRoomEntrance = GetEntranceDir(mRooms.back()->exitDir);

		// create list of potential rooms with that entrance
		std::vector<struct Room*> potentialExits;
		for (struct Room* r : mGame->GetExitRooms())
		{
			if (r->entranceDir == exitRoomEntrance)
				potentialExits.push_back(r);
		}

		// return if there are no possible rooms
		if (potentialExits.empty())
		{
			std::cerr << "Error: no exits with suitable entrance. Skipping exit room." << std::endl;
			return;
		}

		// pick random potential exit
		_rand = rand() % potentialExits.size();
		exit = Clone(potentialExits[_rand]);

		// set exit position
		exit->position = SetRoomPosition(exit, mRooms.back());
	}
	// add exit to rooms vector
	mRooms.push_back(exit);
	
	std::cout << "Done! (" + std::to_string(intersectingRooms) + " Room Intersects)" << std::endl;
}

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

struct Room* Dungeon::Clone(struct Room* room)
{
	struct Room* newRoom = new Room();
	newRoom->fileName = room->fileName;
	newRoom->position = room->position;
	newRoom->size = room->size;
	newRoom->characters = room->characters;
	newRoom->entranceDir = room->entranceDir;
	newRoom->exitDir = room->exitDir;

	return newRoom;
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

bool Dungeon::Intersect(class Room* room_a, class Room* room_b)
{
	bool case1 = GetMax(room_a).x <= GetMin(room_b).x;
	bool case2 = GetMax(room_b).x <= GetMin(room_a).x;
	bool case3 = GetMax(room_a).y <= GetMin(room_b).y;
	bool case4 = GetMax(room_b).y <= GetMin(room_a).y;
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
	max.x = room->position.x + (room->size.x * mTileSize.x);
	max.y = room->position.y + (room->size.y * mTileSize.y);
	return max;
}

Vector2 Dungeon::GetStartPosition()
{
	Vector2 start;
	start = mRooms.front()->position;
	start.x += floor(mRooms.front()->size.x * 0.5f) * mTileSize.x;
	start.y += floor(mRooms.front()->size.y * 0.5f) * mTileSize.y;
	return start;
}