#pragma once
#include "math.h"
#include <vector>
#include <string>

class Dungeon {

public:
    Dungeon(class Game* game);
    void GenerateLevel();

private:
    // Dungeon generation
    void PlaceNodes();
    void PickRooms();
    void PlaceRooms();
    //void ConnectRooms();
    //void Pathfind();

    class Room* Clone(class Room* room);
    void DestroyRoom(class Room* room);
    void FindNodes(class Room* room);

    void ProgressBar(float percent, std::string string);

protected:
    class Game* mGame;
    int mRoomCount;
    Vector2 mDungeonSize; // world units
    Vector2 mCsvSize;

    std::vector<std::vector<class Node*>> mNodeGrid;
    std::vector<class Room*> mRooms;
};