#include "MoveComponent.h"
#include "Actor.h"

MoveComponent::MoveComponent(class Actor* owner)
	:Component(owner, 50)
	, mAngularSpeed(0.0f)
	, mVelocity(Vector2::Zero)
	, mAccel(Vector2::Zero)
{

}

void MoveComponent::Update(float deltaTime)
{
	mOwner->SetRotation((deltaTime * mAngularSpeed) + mOwner->GetRotation());
	SetVelocity(deltaTime * mAccel + mVelocity);
	mOwner->SetPosition((deltaTime * mVelocity) + mOwner->GetPosition());
}
