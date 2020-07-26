#include "CollisionComponent.h"
#include <algorithm>
#include "Actor.h"

CollisionComponent::CollisionComponent(Actor* owner)
:Component(owner)
,mWidth(0.0f)
,mHeight(0.0f)
{
    mOwner = Component::mOwner;
}

CollisionComponent::~CollisionComponent()
{
	
}

bool CollisionComponent::Intersect(const CollisionComponent* other)
{
    bool case1 = GetMax().x < other->GetMin().x;
    bool case2 = other->GetMax().x < GetMin().x;
    bool case3 = GetMax().y < other->GetMin().y;
    bool case4 = other->GetMax().y < GetMin().y;
    if (!case1 && !case2 && !case3 && !case4) {
        return true;
    }
    else {
        return false;
    }
}

Vector2 CollisionComponent::GetMin() const
{
	Vector2 min;
    min.x = mOwner->GetPosition().x - (mWidth * mOwner->GetScale()) / 2.0f;
    min.y = mOwner->GetPosition().y - (mHeight * mOwner->GetScale()) / 2.0f;
    return min;
}

Vector2 CollisionComponent::GetMax() const
{
	Vector2 max;
    max.x = mOwner->GetPosition().x + (mWidth * mOwner->GetScale()) / 2.0f;
    max.y = mOwner->GetPosition().y + (mHeight * mOwner->GetScale()) / 2.0f;
    return max;
}

const Vector2& CollisionComponent::GetCenter() const
{
	return mOwner->GetPosition();
}

CollSide CollisionComponent::GetMinOverlap(
	const CollisionComponent* other, Vector2& offset)
{
	offset = Vector2::Zero;
    
    if(Intersect(other)) {
        Vector2 thisMin = GetMin();
        Vector2 thisMax = GetMax();
        Vector2 otherMin = other->GetMin();
        Vector2 otherMax = other->GetMax();
        float otherMinYDiff = otherMin.y - thisMax.y;
        float otherMinXDiff = otherMin.x - thisMax.x;
        float otherMaxYDiff = otherMax.y - thisMin.y;
        float otherMaxXDiff = otherMax.x - thisMin.x;
        float minDiff = std::min({abs(otherMinYDiff), abs(otherMinXDiff), abs(otherMaxYDiff), abs(otherMaxXDiff)});
        if (minDiff == abs(otherMinYDiff)) {
            offset.y += otherMinYDiff;
            return CollSide::Top;
        } else if (minDiff == abs(otherMinXDiff)) {
            offset.x += otherMinXDiff;
            return CollSide::Left;
        } else if (minDiff == abs(otherMaxYDiff)) {
            offset.y += otherMaxYDiff;
            return CollSide::Bottom;
        } else if (minDiff == abs(otherMaxXDiff)) {
            offset.x += otherMaxXDiff;
            return CollSide::Right;
        }
    }
    
    return CollSide::None;
}
