#include "Actor.h"
#include "Game.h"
#include "Component.h"
#include <Windows.h>
#include <algorithm>

Actor::Actor(Game* game)
    :mGame(game)
    , mState(ActorState::Active)
    , mPosition(Vector2::Zero)
    , mScale(1.0f)
    , mRotation(0.0f)
{
    mGame->AddActor(this);
}

Actor::~Actor()
{
    mGame->RemoveActor(this);

    if (!mComponents.empty()) {
        for (Component* c : mComponents)
            delete c;
    }

    mComponents.clear();
}

void Actor::Update(float deltaTime)
{
    if (GetState() == ActorState::Active) {
        for (Component* c : mComponents)
            c->Update(deltaTime);

        OnUpdate(deltaTime);
    }
}

void Actor::OnUpdate(float deltaTime)
{
}

void Actor::AddComponent(Component* c)
{
    mComponents.emplace_back(c);
    std::sort(mComponents.begin(), mComponents.end(), [](Component* a, Component* b) {
        return a->GetUpdateOrder() < b->GetUpdateOrder();
        });
}
