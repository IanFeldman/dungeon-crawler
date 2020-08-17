#pragma once
#include "math.h"
#include <vector>
#include <map>
#include <set>
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
    void ConnectRooms();
    void FindPaths();
    std::pair<class Node*, class Node*> GetStartAndEndNodes(class Room* start, class Room* end);
    std::vector<class Node*> Pathfind(class Node* startNode, class Node* endNode);

    class Room* Clone(class Room* room);
    void DestroyRoom(class Room* room);
    void FindNodes(class Room* room);

protected:
    class Game* mGame;
    int mRoomCount;
    Vector2 mDungeonSize; // world units
    Vector2 mCsvSize;

    std::map<class Node*, bool> mNodesUsed;
    std::vector<std::vector<class Node*>> mNodeGrid;
    std::vector<class Room*> mInitialRooms;
    std::vector<class Room*> mRooms;
    std::set<std::set<class Room*>> mMinSpanningTree;
};