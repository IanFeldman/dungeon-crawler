#pragma once
#include <SDL.h>
#include <vector>
#include <unordered_map>
#include <string>
#include "math.h"
#include "spritecomponent.h"
#include "player.h"
#include "enemy.h"
#include "dungeon.h"

struct Camera {
    Vector2 position;
    float scale;
};

class Game {

public:
    Game();
    bool Initialize(void);
    void Shutdown(void);
    void Runloop(void);
    void ProcessUpdate(void);
    void UpdateGame(void);
    void GenerateOutput(void);
    void LoadData(void);
    void AddActor(class Actor* actor);
    void RemoveActor(class Actor* actor);
    std::vector<class Actor*> GetActors() { return mActors; }
    void AddSprite(SpriteComponent* sprite);
    void RemoveSprite(SpriteComponent* sprite);
    class Player* GetPlayer() { return mPlayer; }
    std::vector<class Enemy*> GetEnemies() { return mEnemies; }
    SDL_Renderer* GetRenderer() { return mRenderer; }
    SDL_Texture* GetTexture(const char* fileName);
    SDL_Point GetWindowSize() { return mWindowSize; }
    Camera* GetCamera() { return mCamera; }

    void InitializeRooms();
    void CreateRoom(const char* fileName, Vector2 size, OpenSide entranceDir, OpenSide exitDir, bool entrance, bool exit, bool normal);
    void PreloadRooms();
    std::vector<struct Room*> GetRoomTypes() { return mAllRoomTypes; }
    std::vector<struct Room*> GetEntranceRooms() { return mEntrances; }
    std::vector<struct Room*> GetExitRooms() { return mExits; }
    std::vector<struct Room*> GetNormalRooms() { return mNormals; }

private:
    void UnloadData(void);

protected:
    Camera* mCamera;
    bool mRunning;
    int mPreviousTime;
    SDL_Point mWindowSize;
    Vector2 mWindowSizeVector;
    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    std::vector<class Actor*> mActors;
    std::unordered_map<std::string, SDL_Texture*> mTextureCache;
    std::vector<SpriteComponent*> mSprites;
    class Player* mPlayer;
    class Dungeon* mDungeon;
    std::vector<class Enemy*> mEnemies;
    
    std::vector<struct Room*> mAllRoomTypes;
    std::vector<struct Room*> mEntrances;
    std::vector<struct Room*> mExits;
    std::vector<struct Room*> mNormals;
};