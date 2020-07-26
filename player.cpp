#include "player.h"
#include "game.h"
#include "playermove.h"
#include "collisioncomponent.h"
#include "AnimatedSprite.h"
#include "math.h"

Player::Player(Game* game)
	:Actor(game)
{
	SetScale(2.0f);
	// animation
	mASprite = new AnimatedSprite(this, 200);
	// idle
	std::vector<SDL_Texture*> idleAnim{
		mGame->GetTexture("assets/player/idle/idle1.png"),
		mGame->GetTexture("assets/player/idle/idle1.png"),
		mGame->GetTexture("assets/player/idle/idle2.png"),
		mGame->GetTexture("assets/player/idle/idle2.png"),
		mGame->GetTexture("assets/player/idle/idle3.png"),
		mGame->GetTexture("assets/player/idle/idle3.png"),
		mGame->GetTexture("assets/player/idle/idle4.png"),
		mGame->GetTexture("assets/player/idle/idle4.png"),
		mGame->GetTexture("assets/player/idle/idle5.png"),
		mGame->GetTexture("assets/player/idle/idle5.png"),
		mGame->GetTexture("assets/player/idle/idle6.png"),
		mGame->GetTexture("assets/player/idle/idle6.png"),
		mGame->GetTexture("assets/player/idle/idle7.png"),
		mGame->GetTexture("assets/player/idle/idle7.png"),
		mGame->GetTexture("assets/player/idle/idle8.png"),
		mGame->GetTexture("assets/player/idle/idle8.png")
	};
	// left
	std::vector<SDL_Texture*> moveLeftAnim{
		mGame->GetTexture("assets/player/run-left/run-left1.png"),
		mGame->GetTexture("assets/player/run-left/run-left2.png"),
		mGame->GetTexture("assets/player/run-left/run-left3.png"),
		mGame->GetTexture("assets/player/run-left/run-left4.png"),
		mGame->GetTexture("assets/player/run-left/run-left5.png"),
		mGame->GetTexture("assets/player/run-left/run-left6.png"),
		mGame->GetTexture("assets/player/run-left/run-left7.png"),
		mGame->GetTexture("assets/player/run-left/run-left8.png"),
		mGame->GetTexture("assets/player/run-left/run-left9.png"),
		mGame->GetTexture("assets/player/run-left/run-left10.png"),
		mGame->GetTexture("assets/player/run-left/run-left11.png")
	};
	// right
	std::vector<SDL_Texture*> moveRightAnim{
		mGame->GetTexture("assets/player/run-right/run-right1.png"),
		mGame->GetTexture("assets/player/run-right/run-right2.png"),
		mGame->GetTexture("assets/player/run-right/run-right3.png"),
		mGame->GetTexture("assets/player/run-right/run-right4.png"),
		mGame->GetTexture("assets/player/run-right/run-right5.png"),
		mGame->GetTexture("assets/player/run-right/run-right6.png"),
		mGame->GetTexture("assets/player/run-right/run-right7.png"),
		mGame->GetTexture("assets/player/run-right/run-right8.png"),
		mGame->GetTexture("assets/player/run-right/run-right9.png"),
		mGame->GetTexture("assets/player/run-right/run-right10.png"),
		mGame->GetTexture("assets/player/run-right/run-right11.png")
	};
	// up
	std::vector<SDL_Texture*> moveUpAnim{
		mGame->GetTexture("assets/player/run-back/run-back1.png"),
		mGame->GetTexture("assets/player/run-back/run-back2.png"),
		mGame->GetTexture("assets/player/run-back/run-back3.png"),
		mGame->GetTexture("assets/player/run-back/run-back4.png"),
		mGame->GetTexture("assets/player/run-back/run-back5.png"),
		mGame->GetTexture("assets/player/run-back/run-back6.png"),
		mGame->GetTexture("assets/player/run-back/run-back7.png"),
		mGame->GetTexture("assets/player/run-back/run-back8.png"),
		mGame->GetTexture("assets/player/run-back/run-back9.png"),
		mGame->GetTexture("assets/player/run-back/run-back10.png"),
		mGame->GetTexture("assets/player/run-back/run-back11.png")
	};
	// down
	std::vector<SDL_Texture*> moveDownAnim{
		mGame->GetTexture("assets/player/run-forward/run-forward1.png"),
		mGame->GetTexture("assets/player/run-forward/run-forward2.png"),
		mGame->GetTexture("assets/player/run-forward/run-forward3.png"),
		mGame->GetTexture("assets/player/run-forward/run-forward4.png"),
		mGame->GetTexture("assets/player/run-forward/run-forward5.png"),
		mGame->GetTexture("assets/player/run-forward/run-forward6.png"),
		mGame->GetTexture("assets/player/run-forward/run-forward7.png"),
		mGame->GetTexture("assets/player/run-forward/run-forward8.png"),
		mGame->GetTexture("assets/player/run-forward/run-forward9.png"),
		mGame->GetTexture("assets/player/run-forward/run-forward10.png"),
		mGame->GetTexture("assets/player/run-forward/run-forward11.png")
	};
	mASprite->AddAnimation("idle", idleAnim);
	mASprite->AddAnimation("runLeft", moveLeftAnim);
	mASprite->AddAnimation("runRight", moveRightAnim);
	mASprite->AddAnimation("runUp", moveUpAnim);
	mASprite->AddAnimation("runDown", moveDownAnim);
	mASprite->SetAnimation("idleAnim");
	mASprite->SetIsPaused(false);

	// player move must come after anim init
	mPlayerMove = new PlayerMove(this);
	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(16, 16);
}

void Player::OnUpdate(float deltaTime)
{
}