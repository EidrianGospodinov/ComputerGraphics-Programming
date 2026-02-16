#pragma once

#include "DrawableGameComponent.h"

using namespace Library;

namespace Library
{
	class Mesh;
}

namespace Rendering
{
	class ModelFromFile : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(ModelFromFile, DrawableGameComponent)

	public:
		ModelFromFile(Game& game, Camera& camera, const std::string modelFilename);
		~ModelFromFile();

		virtual void Update(const GameTime& gameTime) override;


		//add to support multiple model in the scene, remove this function

		

		
		virtual void Initialize() override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		typedef struct _TextureMappingVertex
		{
			XMFLOAT4 Position;
			XMFLOAT2 TextureCoordinates;

			_TextureMappingVertex() { }

			_TextureMappingVertex(XMFLOAT4 position, XMFLOAT2 textureCoordinates)
				: Position(position), TextureCoordinates(textureCoordinates) { }
		} TextureMappingVertex;

		ModelFromFile();
		ModelFromFile(const ModelFromFile& rhs);
		ModelFromFile& operator=(const ModelFromFile& rhs);

		void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const;

		ID3DX11Effect* mEffect;
        ID3DX11EffectTechnique* mTechnique;
        ID3DX11EffectPass* mPass;
        ID3DX11EffectMatrixVariable* mWvpVariable;
		
		ID3D11ShaderResourceView* mTextureShaderResourceView;
		ID3DX11EffectShaderResourceVariable* mColorTextureVariable;		

        ID3D11InputLayout* mInputLayout;		
        ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		UINT mIndexCount;

		XMFLOAT4X4 mWorldMatrix;	
		float mAngle;

		const std::string modelFile;
	

	};
}
