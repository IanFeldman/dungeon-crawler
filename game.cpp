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
#include "game.h"
#include "actor.h"
#include "math.h"
#include "player.h"
#include "playermove.h"
#include "enemy.h"
#include "enemymove.h"
#include "dungeon.h"
#include "room.h"
#include "wall.h"
#include "spritecomponent.h"

Game::Game()
    :mRunning(true)
    ,mWindowSize({1600, 900})
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
                mCamera->scale /= zoomSpeed;
            else
                mCamera->scale *= zoomSpeed;

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
    Vector2 pos = mPlayer->GetPosition();
    if (mCameraShake) {
        float xOffset = (rand() % mShakeMag) - ((float)mShakeMag * 0.5f);
        float yOffset = (rand() % mShakeMag) - ((float)mShakeMag * 0.5f);
        pos.x += xOffset;
        pos.y += yOffset;
        mShakes++;
    }
    mCamera->position = Vector2::Lerp(mCamera->position, pos, 0.1f);
    if (mShakes >= mShakeLength)
    {
        mCameraShake = false;
        mShakes = 0;
    }
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
    CreateRooms();
    // loading in csv file for room
    PreloadRooms();

    mDungeon = new Dungeon(this);
    mDungeon->GenerateLevel();
    RemoveExampleRooms();
    
    mCamera = new Camera;
    mCamera->position = Vector2::Zero;
    mCamera->scale = 1.0f;

    mPlayer = new Player(this);
    //Vector2 playerPos = Vector2(mWallSize.x, mWallSize.y);
    mPlayer->SetPosition(Vector2::Zero);

    class Enemy* enemy = new Enemy(this);
    enemy->Initialize(Vector2(-10.0f, -10.0f), EnemyType::Normal);
    mEnemies.push_back(enemy);

    // Removes the rooms initialized here as the different room types
    // also removes the walls that were generated with them
}


void Game::CreateRooms()
{
    /*
    Room* r1 = new Room(this, Vector2(3, 3), (const char*)"assets/dungeon/rooms/3x3test.csv");
    mAllRoomTypes.push_back(r1);

    Room* r2 = new Room(this, Vector2(3, 6), (const char*)"assets/dungeon/rooms/3x6test.csv");
    mAllRoomTypes.push_back(r2);
    */

    Room* r3 = new Room(this, Vector2(6, 15), (const char*)"assets/dungeon/rooms/6x15test.csv");
    mAllRoomTypes.push_back(r3);

    //Room* r4 = new Room(this, Vector2(9, 15), (const char*)"assets/dungeon/rooms/9x15test.csv");
    //mAllRoomTypes.push_back(r4);

    //Room* r5 = new Room(this, Vector2(12, 6), (const char*)"assets/dungeon/rooms/12x6test.csv");
    //mAllRoomTypes.push_back(r5);

    //Room* r6 = new Room(this, Vector2(21, 21), (const char*)"assets/dungeon/rooms/21x21test.csv");
    //mAllRoomTypes.push_back(r6);
}

void Game::PreloadRooms()
{
    // load in all csv data
    for (Room* r : mAllRoomTypes)
    {
        // create blank texture to start
        SDL_Texture* finalTex = SDL_CreateTexture(mRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, r->GetSize().x * mCsvSize.x, r->GetSize().y * mCsvSize.y);
        // set render target to texture
        SDL_SetRenderTarget(mRenderer, finalTex);

        SDL_Texture* floor = GetTexture("assets/floor.png");


        std::ifstream inFile;
        inFile.open(r->GetFileName());
        if (!inFile) {
            std::cerr << "Unable to open file datafile.csv";
            mRunning = false;
        }
        std::string temp;
        for (int j = 0; j < r->GetSize().y; j++)
        {
            for (int i = 0; i < r->GetSize().x; i++)
            {
                if (i == r->GetSize().x - 1)
                    std::getline(inFile, temp);
                else
                    std::getline(inFile, temp, ',');

                char tempChar = temp[0];
                
                // add walls
                if (tempChar == 'w')
                {
                    // set the relative wall position as world coordinates
                    Vector2 wallPos = Vector2((i * mCsvSize.x) - (r->GetSize().x * 0.5f * mCsvSize.x) + (mCsvSize.x * 0.5f), (j * mCsvSize.y) - (r->GetSize().y * 0.5f * mCsvSize.y) + (mCsvSize.y * 0.5f));
                    Wall* wall = new Wall(this, wallPos, r);
                    r->AddWall(wall);
                }

                // set texture if we are at a point that is a multiple of 3
                if (j % 3 == 0 && i % 3 == 0)
                {
                    SDL_Rect* rect = new SDL_Rect;
                    rect->x = i * mCsvSize.x;
                    rect->y = j * mCsvSize.y;
                    rect->w = 96;
                    rect->h = 96;
                    SDL_RenderCopy(mRenderer, floor, NULL, rect);
                }

            }
        }

        // reset render target
        SDL_SetRenderTarget(mRenderer, NULL);
        // set final texture
        r->SetSprite(finalTex);
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

void Game::RemoveExampleRooms()
{
    for (Room* r : mAllRoomTypes)
    {
        for (Wall* w : r->GetWalls())
        {
            w->SetState(ActorState::Destroy);
        }
        r->SetState(ActorState::Destroy);
    }
    mAllRoomTypes.clear();
}