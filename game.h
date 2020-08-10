#pragma once
#include <SDL.h>
#include <vector>
#include <unordered_map>
#include <string>
#include "math.h"

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
    void AddSprite(class SpriteComponent* sprite);
    void RemoveSprite(class SpriteComponent* sprite);
    class Player* GetPlayer() { return mPlayer; }
    Vector2 GetCsvSize() { return mCsvSize; }
    std::vector<class Enemy*> GetEnemies() { return mEnemies; }
    std::vector<class Actor*> GetCollidables() { return mCollidables; }
    void AddCollidable(class Actor* collidable) { mCollidables.push_back(collidable); }
    SDL_Renderer* GetRenderer() { return mRenderer; }
    SDL_Texture* GetTexture(const char* fileName);
    SDL_Point GetWindowSize() { return mWindowSize; }
    Camera* GetCamera() { return mCamera; }
    void CameraShake(int magnitude, int length) { mCameraShake = true; mShakeMag = magnitude; mShakeLength = length; }
;
    void CreateRooms();
    void PreloadRooms();
    std::vector<struct Room*> GetRoomTypes() { return mAllRoomTypes; }

    void RemoveExampleRooms();

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
    std::vector<class SpriteComponent*> mSprites;
    class Player* mPlayer;
    class Dungeon* mDungeon;
    Vector2 mCsvSize = Vector2(32.0f, 32.0f);
    std::vector<class Enemy*> mEnemies;
 
    std::vector<class Room*> mAllRoomTypes;
    std::vector<class Actor*> mCollidables;

    bool mCameraShake = false;
    int mShakeMag = 100;
    int mShakeLength = 20;
    int mShakes;
};