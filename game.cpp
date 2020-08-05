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
    float zoomSpeed = 2.0f;

    // Converting mouse coordinates from screen to world
    // Same matrix math as in spritecomponent, but inverted at the end with custom matrix3 invert code in matrix class of math.h
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    Vector2 mousePos(mouseX, mouseY);

    Matrix3 manipulatorMatrix = Matrix3::Identity;
    Matrix3 scaleMatrix = Matrix3::CreateScale(mCamera->scale);
    Matrix3 translateMatrix = Matrix3::CreateTranslation(Vector2(-mCamera->position.x * mCamera->scale + mWindowSize.x / 2, -mCamera->position.y * mCamera->scale + mWindowSize.y / 2));
    manipulatorMatrix = scaleMatrix * translateMatrix;
    manipulatorMatrix.Invert();
    mousePos = Vector2::Transform(mousePos, manipulatorMatrix);
    
    Vector2 toMouse = Vector2::Zero;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type)
        {
        // looking for exit clicked
        case SDL_QUIT:
            mRunning = false;
            break;
        // looking for escape key
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                mRunning = false;
                break;
            default:
                break;
            }
            break;
        // looking for scroll wheel
        case SDL_MOUSEWHEEL:
            if (event.wheel.y > 0)
                mCamera->scale *= zoomSpeed;
            else
                mCamera->scale /= zoomSpeed;

            // std::cout << mCamera->scale << std::endl;
            
            // clamp camera scale
            /*
            if (mCamera->scale < 0.421875f)
                mCamera->scale = 0.421875f;
            else if (mCamera->scale > 1.0f)
                mCamera->scale = 1.0f;
            */
            break;
        case SDL_MOUSEBUTTONDOWN:
            switch (event.button.button)
            {
            case SDL_BUTTON_LEFT:
                toMouse = Vector2::Normalize(mousePos - mPlayer->GetPosition());
                mPlayer->Attack(toMouse);
                //std::cout << "Mouse Position: " + std::to_string(toMouse.x) + ", " + std::to_string(toMouse.y) << std::endl;
                //std::cout << "Mouse Position: " + std::to_string(mousePos.x) + ", " + std::to_string(mousePos.y) << std::endl;
                //std::cout << "Player Position: " + std::to_string(mPlayer->GetPosition().x) + ", " + std::to_string(mPlayer->GetPosition().y) << std::endl;
                //std::cout << "Distance to Mouse: " + std::to_string(toMouse.x) + ", " + std::to_string(toMouse.y) << std::endl;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }

    // processing input for player
    const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
    mPlayer->GetComponent<PlayerMove>()->MovementInput(keyboardState);
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
    // setting up different types of rooms
    InitializeRooms();
    // loading in csv file for room
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
    // entrances
    CreateRoom("assets/dungeon/rooms/entrances/n5x5.csv", Vector2(5.0f, 5.0f), OpenSide::None, OpenSide::North, true, false, false);
    CreateRoom("assets/dungeon/rooms/entrances/e5x5.csv", Vector2(5.0f, 5.0f), OpenSide::None, OpenSide::East, true, false, false);
    CreateRoom("assets/dungeon/rooms/entrances/s5x5.csv", Vector2(5.0f, 5.0f), OpenSide::None, OpenSide::South, true, false, false);
    CreateRoom("assets/dungeon/rooms/entrances/w5x5.csv", Vector2(5.0f, 5.0f), OpenSide::None, OpenSide::West, true, false, false);
    // exits
    CreateRoom("assets/dungeon/rooms/exits/n5x5.csv", Vector2(5.0f, 5.0f), OpenSide::North, OpenSide::None, false, true, false);
    CreateRoom("assets/dungeon/rooms/exits/e5x5.csv", Vector2(5.0f, 5.0f), OpenSide::East, OpenSide::None, false, true, false);
    CreateRoom("assets/dungeon/rooms/exits/s5x5.csv", Vector2(5.0f, 5.0f), OpenSide::South, OpenSide::None, false, true, false);
    CreateRoom("assets/dungeon/rooms/exits/w5x5.csv", Vector2(5.0f, 5.0f), OpenSide::West, OpenSide::None, false, true, false);
    // normals
    CreateRoom("assets/dungeon/rooms/normals/ew5x5.csv", Vector2(5.0f, 5.0f), OpenSide::East, OpenSide::West, false, false, true);
    CreateRoom("assets/dungeon/rooms/normals/ew5x5.csv", Vector2(5.0f, 5.0f), OpenSide::West, OpenSide::East, false, false, true);
    CreateRoom("assets/dungeon/rooms/normals/ns5x5.csv", Vector2(5.0f, 5.0f), OpenSide::North, OpenSide::South, false, false, true);
    CreateRoom("assets/dungeon/rooms/normals/ns5x5.csv", Vector2(5.0f, 5.0f), OpenSide::South, OpenSide::North, false, false, true);

    CreateRoom("assets/dungeon/rooms/normals/ew10x5.csv", Vector2(10.0f, 5.0f), OpenSide::East, OpenSide::West, false, false, true);
    CreateRoom("assets/dungeon/rooms/normals/ew10x5.csv", Vector2(10.0f, 5.0f), OpenSide::West, OpenSide::East, false, false, true);

    CreateRoom("assets/dungeon/rooms/normals/ns9x9.csv", Vector2(9.0f, 9.0f), OpenSide::North, OpenSide::South, false, false, true);
    CreateRoom("assets/dungeon/rooms/normals/ns9x9.csv", Vector2(9.0f, 9.0f), OpenSide::South, OpenSide::North, false, false, true);
    
    CreateRoom("assets/dungeon/rooms/normals/ne19x19.csv", Vector2(19.0f, 19.0f), OpenSide::North, OpenSide::East, false, false, true);
    CreateRoom("assets/dungeon/rooms/normals/ne19x19.csv", Vector2(19.0f, 19.0f), OpenSide::East, OpenSide::North, false, false, true);
    
    CreateRoom("assets/dungeon/rooms/normals/sw15x7.csv", Vector2(15.0f, 7.0f), OpenSide::South, OpenSide::West, false, false, true);
    CreateRoom("assets/dungeon/rooms/normals/sw15x7.csv", Vector2(15.0f, 7.0f), OpenSide::West, OpenSide::South, false, false, true);

    // angle normals
    CreateRoom("assets/dungeon/rooms/normals/nw5x5.csv", Vector2(5.0f, 5.0f), OpenSide::North, OpenSide::West, false, false, true);
    CreateRoom("assets/dungeon/rooms/normals/nw5x5.csv", Vector2(5.0f, 5.0f), OpenSide::West, OpenSide::North, false, false, true);
    CreateRoom("assets/dungeon/rooms/normals/ne5x5.csv", Vector2(5.0f, 5.0f), OpenSide::North, OpenSide::East, false, false, true);
    CreateRoom("assets/dungeon/rooms/normals/ne5x5.csv", Vector2(5.0f, 5.0f), OpenSide::East, OpenSide::North, false, false, true);
    CreateRoom("assets/dungeon/rooms/normals/se5x5.csv", Vector2(5.0f, 5.0f), OpenSide::South, OpenSide::East, false, false, true);
    CreateRoom("assets/dungeon/rooms/normals/se5x5.csv", Vector2(5.0f, 5.0f), OpenSide::East, OpenSide::South, false, false, true);
    CreateRoom("assets/dungeon/rooms/normals/sw5x5.csv", Vector2(5.0f, 5.0f), OpenSide::South, OpenSide::West, false, false, true);
    CreateRoom("assets/dungeon/rooms/normals/sw5x5.csv", Vector2(5.0f, 5.0f), OpenSide::West, OpenSide::South, false, false, true);
}

void Game::CreateRoom(const char* fileName, Vector2 size, OpenSide entranceDir, OpenSide exitDir, bool entranceRoom, bool exitRoom, bool normalRoom)
{
    struct Room* r = new Room;
    r->fileName = fileName;
    r->size = size;
    r->entranceDir = entranceDir;
    r->exitDir = exitDir;

    if (entranceRoom)
        mEntrances.push_back(r);
    if (exitRoom)
        mExits.push_back(r);
    if (normalRoom)
        mNormals.push_back(r);

    mAllRoomTypes.push_back(r);
}

void Game::PreloadRooms()
{
    // load in all csv data
    for (class Room* r : mAllRoomTypes)
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