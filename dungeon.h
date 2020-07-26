#pragma once
#include "math.h"
#include <vector>
#include <string>

struct Room {
    const char* fileName;
    Vector2 position = Vector2::Zero;
    Vector2 size; // in tiles
    std::vector<std::vector<char>> characters;
    class Node* entrance = nullptr;
    class Node* exit = nullptr;
    class Room* nextRoom = nullptr;
};

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

private:
    // Dungeon generation
    void PickRooms();
    void PlaceRooms();
    void Passageways();
    std::vector<class Node*> PathFind(class Node* start, class Node* end);
    void DrawRooms();

    // Room intersection
    bool Intersect(class Room* room_a, class Room* room_b);
    Vector2 GetMax(class Room* room);
    Vector2 GetMin(class Room* room);

    void ProgressBar(float percent, std::string string);

protected:
    class Game* mGame;
    int mRoomCount;
    int mSize;
    int mTileSize;

    std::vector<class Room*> mRooms;
    std::vector<class Tile*> mTiles;
    std::vector<class Node*> mNodes;
    std::vector<std::vector<class Node*>> mPaths;
};