#include "Projectile.h"
#include "Game.h"
#include "Camera.h"
#include "GameException.h"
#include <cmath>

using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(Projectile)

	struct SimpleVertex
	{
		XMFLOAT4 Position;
		XMFLOAT2 TextureCoordinates;

		SimpleVertex() : Position(0, 0, 0, 1), TextureCoordinates(0, 0) { }
		SimpleVertex(XMFLOAT4 position, XMFLOAT2 texCoord)
			: Position(position), TextureCoordinates(texCoord) { }
	};

	Projectile::Projectile(Game& game, Camera& camera, const XMFLOAT3& position, const XMFLOAT3& direction, float speed)
		: DrawableGameComponent(game, camera), mPosition(position), mVelocity(direction.x * speed, direction.y * speed, direction.z * speed),
		  mIsAlive(true), mMaxDistance(150.0f), mDistanceTraveled(0.0f),
		  mEffect(nullptr), mTechnique(nullptr), mPass(nullptr), mWvpVariable(nullptr),
		  mInputLayout(nullptr), mVertexBuffer(nullptr), mIndexBuffer(nullptr), mIndexCount(0)
	{
		mBoundingSphere.Center = position;
		mBoundingSphere.Radius = 0.5f;
		XMStoreFloat4x4(&mWorldMatrix, XMMatrixIdentity());
	}

	Projectile::~Projectile()
	{
		ReleaseObject(mIndexBuffer);
		ReleaseObject(mVertexBuffer);
		ReleaseObject(mInputLayout);
		ReleaseObject(mEffect);
	}

	void Projectile::Initialize()
	{
		ID3D11Device* device = mGame->Direct3DDevice();

		// Load TextureMapping effect
		std::wstring effectPath(L"Content\\Effects\\TextureMapping.cso");
		HRESULT hr = D3DX11CreateEffectFromFile(effectPath.c_str(), 0, device, &mEffect);
		if (FAILED(hr))
		{
			throw Library::GameException("Failed to load TextureMapping effect", hr);
		}

		mTechnique = mEffect->GetTechniqueByIndex(0);
		mPass = mTechnique->GetPassByIndex(0);
		mWvpVariable = mEffect->GetVariableByName("WorldViewProjection")->AsMatrix();

		// Create simple cube mesh for projectile
		SimpleVertex vertices[] =
		{
			SimpleVertex(XMFLOAT4(-0.3f, -0.3f, -0.3f, 1), XMFLOAT2(0, 0)),
			SimpleVertex(XMFLOAT4(0.3f, -0.3f, -0.3f, 1), XMFLOAT2(1, 0)),
			SimpleVertex(XMFLOAT4(0.3f, 0.3f, -0.3f, 1), XMFLOAT2(1, 1)),
			SimpleVertex(XMFLOAT4(-0.3f, 0.3f, -0.3f, 1), XMFLOAT2(0, 1)),
		};

		UINT indices[] = { 0, 1, 2, 0, 2, 3 };

		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(vertices);
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vinitData;
		vinitData.pSysMem = vertices;

		hr = device->CreateBuffer(&vbd, &vinitData, &mVertexBuffer);
		if (FAILED(hr))
		{
			throw Library::GameException("CreateBuffer() for vertex buffer failed.", hr);
		}

		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(indices);
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = indices;

		hr = device->CreateBuffer(&ibd, &iinitData, &mIndexBuffer);
		if (FAILED(hr))
		{
			throw Library::GameException("CreateBuffer() for index buffer failed.", hr);
		}

		mIndexCount = ARRAYSIZE(indices);

		// Create input layout for TextureMapping
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		D3DX11_PASS_DESC passDesc;
		mPass->GetDesc(&passDesc);
		hr = device->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions),
			passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout);
		if (FAILED(hr))
		{
			throw Library::GameException("CreateInputLayout() failed.", hr);
		}
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

		XMMATRIX worldMatrix = XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
		XMStoreFloat4x4(&mWorldMatrix, worldMatrix);
	}

	void Projectile::Draw(const GameTime& gameTime)
	{
		if (!mIsAlive || mEffect == nullptr || mPass == nullptr) return;

		ID3D11DeviceContext* context = mGame->Direct3DDeviceContext();

		context->IASetInputLayout(mInputLayout);

		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX viewMatrix = mCamera->ViewMatrix();
		XMMATRIX projectionMatrix = mCamera->ProjectionMatrix();
		XMMATRIX wvpMatrix = worldMatrix * viewMatrix * projectionMatrix;

		if (mWvpVariable != nullptr)
		{
			mWvpVariable->SetMatrix(reinterpret_cast<const float*>(&wvpMatrix));
		}

		mPass->Apply(0, context);
		context->DrawIndexed(mIndexCount, 0, 0);
	}
}
