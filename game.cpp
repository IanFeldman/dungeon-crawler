#include <SDL_image.h>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <Windows.h>
#include <WinDef.h>
#include <fstream>
#include <string>
#include <cstring>
#include <random>
#include <ctime>
#include "Game.h"
#include "Actor.h"
#include "Math.h"
#include "Player.h"
#include "PlayerMove.h"
#include "enemy.h"
#include "enemymove.h"
#include "dungeon.h"

Game::Game()
    :mRunning(true)
    ,mWindowSize({1500, 750})
{
    mWindowSizeVector.Set(mWindowSize.x, mWindowSize.y);
}

bool Game::Initialize() {
    int initialized = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    if (initialized != 0) {
        return false;
    }
    mWindow = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mWindowSize.x, mWindowSize.y, 0);
    if (mWindow == NULL) {
        return false;
    }
    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    int imgInitialized = IMG_Init(IMG_INIT_PNG);
    if ((imgInitialized & IMG_INIT_PNG) != IMG_INIT_PNG) {
        return false;
    }
    if (mRenderer == NULL) {
        return false;
    }

    // setting random seed as time
    srand(time(NULL));

    LoadData();

    mPreviousTime = SDL_GetTicks();

    return true;
};

void Game::Shutdown() {
    UnloadData();
    IMG_Quit();
    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
};

void Game::Runloop() {
    while (mRunning) {
        ProcessUpdate();
        UpdateGame();
        GenerateOutput();
    }
};

void Game::ProcessUpdate() {
    float zoomSpeed = 0.75f;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type)
        {
        // looking for exit clicked
        case SDL_QUIT:
            mRunning = false;
            break;
        // looking for scroll wheel
        case SDL_MOUSEWHEEL:
            if (event.wheel.y > 0)
                mCamera->scale /= zoomSpeed;
            else
                mCamera->scale *= zoomSpeed;
            
            // clamp camera scale
            /*
            if (mCamera->scale < 0.421875f)
                mCamera->scale = 0.421875f;
            else if (mCamera->scale > 1.0f)
                mCamera->scale = 1.0f;
            */

            break;
        default:
            break;
        }
    }

    // processing input for player
    const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
    mPlayer->GetComponent<PlayerMove>()->ProcessInput(keyboardState);
}

void Game::UpdateGame() {
    // Setting fps
    while ((int)SDL_GetTicks() < mPreviousTime + 16) {}
    int currentTime = SDL_GetTicks();
    float deltaTime = (float)(currentTime - mPreviousTime) / 1000;
    if (deltaTime > 0.033f) {
        deltaTime = 0.033f;
    }
    mPreviousTime = currentTime;

    std::vector<Actor*> actors = mActors;
    for (Actor* a : actors) {
        a->Update(deltaTime);
    }

    std::vector<Actor*> tempActors;
    for (Actor* a : mActors) {
        if (a->GetState() == ActorState::Destroy) {
            tempActors.emplace_back(a);
        }
    }

    if (!tempActors.empty()) {
        for (Actor* tempActor : tempActors) {
            delete tempActor;
        }
    }

    // update camera pos
    mCamera->position = Vector2::Lerp(mCamera->position, mPlayer->GetPosition(), 0.1f);
}

void Game::GenerateOutput() {
    SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 0);
    SDL_RenderClear(mRenderer);

    for (SpriteComponent* sprite : mSprites) {
        if (sprite->IsVisible()) {
            sprite->Draw(mRenderer);
        }
    }

    SDL_RenderPresent(mRenderer);
}

void Game::AddActor(Actor* actor) {
    mActors.emplace_back(actor);
}

void Game::RemoveActor(Actor* actor) {
    std::vector<Actor*>::iterator it;
    it = std::find(mActors.begin(), mActors.end(), actor);

    if (it != mActors.end()) {
        mActors.erase(it);
    }
}

void Game::LoadData()
{
    InitializeRooms();
    PreloadRooms();

    mDungeon = new Dungeon(this);
    mDungeon->GenerateLevel();

    mCamera = new Camera;
    mCamera->position = Vector2(320, 320);
    mCamera->scale = 1.0f;

    Vector2 pos = Vector2(16, 16);
    mPlayer = new Player(this);
    mPlayer->SetPosition(pos);

    class Enemy* _enemy = new Enemy(this);
    _enemy->SetPosition(Vector2(112, 112));
    mEnemies.push_back(_enemy);
}

void Game::InitializeRooms()
{
    class Room* _room = new Room;
    _room->fileName = (const char*)"assets/rooms/test.csv";
    _room->size = Vector2(7, 7);
    mRoomTypes.push_back(_room);

    _room = new Room;
    _room->fileName = (const char*)"assets/rooms/test1.csv";
    _room->size = Vector2(11, 5);
    mRoomTypes.push_back(_room);

    _room = new Room;
    _room->fileName = (const char*)"assets/rooms/test2.csv";
    _room->size = Vector2(17, 17);
    mRoomTypes.push_back(_room);

    _room = new Room;
    _room->fileName = (const char*)"assets/rooms/test3.csv";
    _room->size = Vector2(20, 20);
    mRoomTypes.push_back(_room);
}

void Game::PreloadRooms()
{
    // load in all csv data
    for (class Room* r : mRoomTypes)
    {
        std::ifstream inFile;
        inFile.open(r->fileName);
        if (!inFile) {
            std::cerr << "Unable to open file datafile.csv";
            mRunning = false;
        }
        std::string temp;
        for (int j = 0; j < r->size.y; j++)
        {
            std::vector<char> _row;

            for (int i = 0; i < r->size.x; i++)
            {
                if (i == r->size.x - 1)
                    std::getline(inFile, temp);
                else
                    std::getline(inFile, temp, ',');

                char tempChar = temp[0];
                if (tempChar == NULL)
                    _row.push_back((char)'.');
                else
                    _row.push_back(tempChar);
            }

            r->characters.push_back(_row);
        }
    }
}

void Game::UnloadData() {
    if (!mActors.empty()) {
        mActors.erase(mActors.begin(), mActors.end());
    }

    for (auto it = mTextureCache.begin(); it != mTextureCache.end(); it++) {
        SDL_DestroyTexture(it->second);
    }

    mTextureCache.clear();
}

SDL_Texture* Game::GetTexture(const char* fileName) {
    std::unordered_map<std::string, SDL_Texture*>::const_iterator got = mTextureCache.find(fileName);
    if (got != mTextureCache.end())
        return got->second;
    else {
        SDL_Surface* tempSurface = IMG_Load(fileName);
        SDL_Texture* tempTexture = SDL_CreateTextureFromSurface(mRenderer, tempSurface);
        SDL_FreeSurface(tempSurface);
        mTextureCache.emplace(fileName, tempTexture);
        return tempTexture;
    }
}

void Game::AddSprite(SpriteComponent* sprite) {

    mSprites.emplace_back(sprite);
    std::sort(mSprites.begin(), mSprites.end(),
        [](SpriteComponent* a, SpriteComponent* b) {
            return a->GetDrawOrder() < b->GetDrawOrder();
        });
}

void Game::RemoveSprite(SpriteComponent* sprite) {
    std::vector<SpriteComponent*>::iterator it;
    it = std::find(mSprites.begin(), mSprites.end(), sprite);

    if (it != mSprites.end()) {
        mSprites.erase(it);
    }
}