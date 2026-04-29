#include "Projectile.h"
#include "Game.h"
#include "Camera.h"
#include "ModelFromFile.h"
#include <cmath>

using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(Projectile)

	Projectile::Projectile(Game& game, Camera& camera, const XMFLOAT3& position, const XMFLOAT3& direction, float speed)
		: GameComponent(game), mModel(nullptr), mPosition(position),
		  mVelocity(direction.x * speed, direction.y * speed, direction.z * speed),
		  mIsAlive(true), mMaxDistance(150.0f), mDistanceTraveled(0.0f)
	{
		mBoundingSphere.Center = position;
		mBoundingSphere.Radius = 0.5f;


		mModel = new ModelFromFile(game, camera, "Content\\Models\\Bullet.fbx", L"Projectile", 0);
		mModel->SetFallbackColor(255, 215, 0);
		mModel->Initialize();
		mModel->SetPosition(mRotation.x, mRotation.y, mRotation.z, BulletScale, position.x, position.y, position.z);
	}

	Projectile::~Projectile()
	{
	}

	void Projectile::Update(const GameTime& gameTime)
	{
		float elapsedTime = (float)gameTime.ElapsedGameTime();
		float distance = sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y + mVelocity.z * mVelocity.z) * elapsedTime;

		mPosition.x += mVelocity.x * elapsedTime;
		mPosition.y += mVelocity.y * elapsedTime;
		mPosition.z += mVelocity.z * elapsedTime;

		mDistanceTraveled += distance;
		mBoundingSphere.Center = mPosition;

		if (mDistanceTraveled >= mMaxDistance)
		{
			mIsAlive = false;
		}

		if (mModel != nullptr)
		{
			mModel->SetPosition(mRotation.x, mRotation.y, mRotation.z, BulletScale, mPosition.x, mPosition.y, mPosition.z);
		}
	}
}
