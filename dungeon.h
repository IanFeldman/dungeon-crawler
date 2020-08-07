#pragma once
#include "math.h"
#include <vector>
#include <string>

enum class OpenSide {
    None,
    North,
    East,
    South,
    West
};

struct Room {
    const char* fileName;
    Vector2 position = Vector2::Zero;
    Vector2 size; // in tiles
    std::vector<std::vector<char>> characters;
    OpenSide entranceDir = OpenSide::None;
    OpenSide exitDir = OpenSide::None;
};

// kept for enemy pathfinding?
struct Node {
    Vector2 pos;
    std::vector<class Node*> neighbors;
    class Node* previousNode;
    int fScore; // total cost
    int gScore; // dist to get to this node
};

class Dungeon {

public:
    Dungeon(class Game* game);
    void GenerateLevel();
    Vector2 GetStartPosition();

private:
    // Dungeon generation
    void PlaceRooms();
    void DrawRooms();

    struct Room* Clone(struct Room* room);
    OpenSide GetEntranceDir(OpenSide prevExitDir);
    Vector2 SetRoomPosition(struct Room* currRoom, struct Room* prevRoom);

    void ProgressBar(float percent, std::string string);

    // Room intersection
    bool Intersect(class Room* room_a, class Room* room_b);
    Vector2 GetMax(class Room* room);
    Vector2 GetMin(class Room* room);

protected:
    class Game* mGame;
    int mRoomCount;
    Vector2 mTileSize;

    std::vector<class Room*> mRooms;
    std::vector<class Tile*> mTiles;
    std::vector<class Node*> mNodes;
};