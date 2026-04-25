#pragma once

#include "DrawableGameComponent.h"
#include <DirectXCollision.h>

using namespace Library;

namespace Rendering
{
	class Projectile : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(Projectile, DrawableGameComponent)

	public:
		Projectile(Game& game, Camera& camera, const XMFLOAT3& position, const XMFLOAT3& direction, float speed);
		~Projectile();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

		DirectX::BoundingSphere GetBoundingSphere() const { return mBoundingSphere; }
		bool IsAlive() const { return mIsAlive; }
		void SetAlive(bool alive) { mIsAlive = alive; }

	private:
		Projectile();
		Projectile(const Projectile& rhs);
		Projectile& operator=(const Projectile& rhs);

		XMFLOAT3 mPosition;
		XMFLOAT3 mVelocity;
		DirectX::BoundingSphere mBoundingSphere;
		bool mIsAlive;
		float mMaxDistance;
		float mDistanceTraveled;

		ID3DX11Effect* mEffect;
		ID3DX11EffectTechnique* mTechnique;
		ID3DX11EffectPass* mPass;
		ID3DX11EffectMatrixVariable* mWvpVariable;
		ID3D11InputLayout* mInputLayout;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		UINT mIndexCount;
		XMFLOAT4X4 mWorldMatrix;
	};
}
