#pragma once

#include "DrawableGameComponent.h"

#include <DirectXCollision.h>

using namespace Library;

namespace Library
{
	class Mesh;
}

namespace Rendering
{
	struct ModelMovementSettings
	{
		bool ShouldMove;
		float MoveSpeed;
		float RotationSpeed;  // radians/sec around Y axis, 0 = no spin

		ModelMovementSettings(bool shouldMove = true, float moveSpeed = 1.0f, float rotSpeed = 0.0f)
			: ShouldMove(shouldMove), MoveSpeed(moveSpeed), RotationSpeed(rotSpeed) { }
	};

	class ModelFromFile : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(ModelFromFile, DrawableGameComponent)

	public:
		ModelFromFile(Game& game, Camera& camera, const std::string modelFilename);
		ModelFromFile(Game& game, Camera& camera, const std::string modelFilename, const std::wstring modelDes, int modelValue);
		ModelFromFile(Game& game, Camera& camera, const std::string modelFilename, const std::wstring modelDes, int modelValue, const ModelMovementSettings& movementSettings);
		~ModelFromFile();

		virtual void Update(const GameTime& gameTime) override;
		void MoveModel(const GameTime& gameTime, float speed);

		//add to support multiple model in the scene, remove this function

		void SetPosition(const float rotateX, const float rotateY, const float rotateZ, const float scaleFactor, const float translateX, const float translateY, const float translateZ);

		//bounding box require to access the world matrix
		
		XMFLOAT4X4* WorldMatrix() { return &mWorldMatrix; }

		//need to access this , make this public for simplicity
		DirectX::BoundingBox mBoundingBox;
		DirectX::BoundingBox mWorldBox;
		const std::wstring GetModelDes() { return modelDes; }
		int const ModelValue() { return mModelValue; }

		
		virtual void Initialize() override;
		virtual void Draw(const GameTime& gameTime) override;

		void SetFallbackColor(byte r, byte g, byte b)
		{
			mFallbackR = r; mFallbackG = g; mFallbackB = b;
		}





	private:
		typedef struct _TextureMappingVertex
		{
			XMFLOAT4 Position;
			XMFLOAT2 TextureCoordinates;

			_TextureMappingVertex() : Position{0,0,0,0}, TextureCoordinates{0,0} { }

			_TextureMappingVertex(XMFLOAT4 position, XMFLOAT2 textureCoordinates)
				: Position(position), TextureCoordinates(textureCoordinates) { }
		} TextureMappingVertex;

		typedef struct _MeshPart
		{
			ID3D11Buffer* VertexBuffer;
			ID3D11Buffer* IndexBuffer;
			ID3D11ShaderResourceView* TextureShaderResourceView;
			UINT IndexCount;

			_MeshPart() : VertexBuffer(nullptr), IndexBuffer(nullptr), TextureShaderResourceView(nullptr), IndexCount(0) { }
		} MeshPart;

		ModelFromFile();
		ModelFromFile(const ModelFromFile& rhs);
		ModelFromFile& operator=(const ModelFromFile& rhs);

		void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer, XMVECTOR& minBounds, XMVECTOR& maxBounds) const;

		ID3DX11Effect* mEffect;
        ID3DX11EffectTechnique* mTechnique;
        ID3DX11EffectPass* mPass;
        ID3DX11EffectMatrixVariable* mWvpVariable;
		
		ID3DX11EffectShaderResourceVariable* mColorTextureVariable;		

        ID3D11InputLayout* mInputLayout;		
		std::vector<MeshPart> mMeshParts;

		XMFLOAT4X4 mWorldMatrix;	
		float mAngle;
		bool mShouldMove;
		float mMoveSpeed;
		float mRotationSpeed;

		const std::string modelFile;

		std::wstring modelDes;
		int mModelValue;
		
		byte mFallbackR, mFallbackG, mFallbackB;

	};
}
