#pragma once
#include "Component.h"
#include "Math.h"

class MoveComponent : public Component
{
public:
	MoveComponent(class Actor* owner);

	// Update the move component
	void Update(float deltaTime) override;

	// Getters/setters
	float GetAngularSpeed() const { return mAngularSpeed; }
	float GetForwardSpeed() const { return mVelocity.Length(); }
	void SetAngularSpeed(float speed) { mAngularSpeed = speed; }
	void SetForwardSpeed(float speed) { mVelocity = Vector2::Normalize(mVelocity) * speed; }

	Vector2 GetVelocity() const { return mVelocity; }
	Vector2 GetAccel() const { return mAccel; }
	void SetVelocity(Vector2 velocity) { mVelocity = velocity; }
	void SetAccel(Vector2 accel) { mAccel = accel;  }

private:
	// Angular speed (in radians/second)
	float mAngularSpeed;
	// Forward speed (in pixels/second)
	Vector2 mVelocity, mAccel;

};
