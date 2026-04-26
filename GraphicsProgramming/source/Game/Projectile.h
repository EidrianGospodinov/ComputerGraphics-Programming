#pragma once

#include "GameComponent.h"
#include <DirectXCollision.h>

namespace Library
{
	class Camera;
}

using namespace Library;

namespace Rendering
{
	class ModelFromFile;

	class Projectile : public GameComponent
	{
		RTTI_DECLARATIONS(Projectile, GameComponent)

	public:
		Projectile(Game& game, Camera& camera, const XMFLOAT3& position, const XMFLOAT3& direction, float speed);
		~Projectile();

		virtual void Update(const GameTime& gameTime) override;

		ModelFromFile* GetModel() const { return mModel; }
		DirectX::BoundingSphere GetBoundingSphere() const { return mBoundingSphere; }
		bool IsAlive() const { return mIsAlive; }
		void SetAlive(bool alive) { mIsAlive = alive; }

	private:
		Projectile();
		Projectile(const Projectile& rhs);
		Projectile& operator=(const Projectile& rhs);

		ModelFromFile* mModel;
		XMFLOAT3 mPosition;
		XMFLOAT3 mVelocity;
		DirectX::BoundingSphere mBoundingSphere;
		bool mIsAlive;
		float mMaxDistance;
		float mDistanceTraveled;
	};
}
