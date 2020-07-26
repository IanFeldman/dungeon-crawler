#include "SpriteComponent.h"
#include "Actor.h"
#include "Game.h"

SpriteComponent::SpriteComponent(Actor* owner, int drawOrder)
	:Component(owner)
	, mTexture(nullptr)
	, mDrawOrder(drawOrder)
	, mTexWidth(0)
	, mTexHeight(0)
{
	mOwner->GetGame()->AddSprite(this);
}

SpriteComponent::~SpriteComponent()
{
	mOwner->GetGame()->RemoveSprite(this);
}

void SpriteComponent::Draw(SDL_Renderer* renderer)
{
	Vector2 ownerPos = mOwner->GetPosition();
	Camera* cam = mOwner->GetGame()->GetCamera();
	SDL_Point windowSize = mOwner->GetGame()->GetWindowSize();

	Matrix3 manipulatorMatrix = Matrix3::Identity;
	Matrix3 scaleMatrix = Matrix3::CreateScale(cam->scale);
	// i dont know why the camera pos has to be inverted, but it works this way
	Matrix3 translateMatrix = Matrix3::CreateTranslation(Vector2(-cam->position.x * cam->scale + windowSize.x / 2, -cam->position.y * cam->scale + windowSize.y / 2));
	manipulatorMatrix = scaleMatrix * translateMatrix;
	Vector2 screenPos = Vector2::Transform(ownerPos, manipulatorMatrix);

	// Dont draw what is outside of the camera
	float halfWidth = (mTexWidth * mOwner->GetScale()) / 2.0f;
	float halfHeight = (mTexHeight * mOwner->GetScale()) / 2.0f;
	if (screenPos.x < -halfWidth || screenPos.y < -halfHeight || screenPos.x > windowSize.x + halfWidth || screenPos.y > windowSize.y + halfHeight)
		return;

	if (mTexture)
	{
		SDL_Rect r;
		r.w = static_cast<int>(mTexWidth * mOwner->GetScale() * cam->scale);
		r.h = static_cast<int>(mTexHeight * mOwner->GetScale() * cam->scale);
		// Center the rectangle around the position of the owner
		r.x = static_cast<int>(screenPos.x - r.w / 2);
		r.y = static_cast<int>(screenPos.y - r.h / 2 );

		// Draw (have to convert angle from radians to degrees, and clockwise to counter)
		SDL_RenderCopyEx(renderer,
			mTexture,
			nullptr,
			&r,
			-Math::ToDegrees(mOwner->GetRotation()),
			nullptr,
			SDL_FLIP_NONE);
	}
}

void SpriteComponent::SetTexture(SDL_Texture* texture)
{
	mTexture = texture;
	// Set width/height
	SDL_QueryTexture(texture, nullptr, nullptr, &mTexWidth, &mTexHeight);
}
