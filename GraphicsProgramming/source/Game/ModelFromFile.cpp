#include "ModelFromFile.h"

#include <SpriteBatch.h>
#include <SpriteFont.h>

#include "Game.h"
#include "GameException.h"
#include "MatrixHelper.h"
#include "Camera.h"
#include "Utility.h"
#include "D3DCompiler.h"
#include "Model.h"
#include "ModelMaterial.h"
#include "Mesh.h"
#include <WICTextureLoader.h>
#include <Shlwapi.h>

using namespace DirectX;

namespace Rendering
{
    namespace
    {
        bool UsesModelTextureFallback(const std::string& modelFile)
        {
            std::wstring extension = Utility::ToWideString(modelFile);
            extension = PathFindExtensionW(extension.c_str());

            return (_wcsicmp(extension.c_str(), L".obj") == 0 || _wcsicmp(extension.c_str(), L".fbx") == 0);
        }

        std::wstring ResolveTexturePath(const std::string& modelFile, const std::wstring& textureReference)
        {
            if (textureReference.empty() || textureReference[0] == L'*')
            {
                return L"";
            }

            std::wstring normalizedReference(textureReference);
            for (wchar_t& character : normalizedReference)
            {
                if (character == L'/')
                {
                    character = L'\\';
                }
            }

            if (PathFileExistsW(normalizedReference.c_str()))
            {
                return normalizedReference;
            }

            std::string modelDirectoryA;
            Utility::GetDirectory(modelFile, modelDirectoryA);
            std::wstring modelDirectory = Utility::ToWideString(modelDirectoryA);

            std::wstring candidatePath;
            Utility::PathJoin(candidatePath, modelDirectory, normalizedReference);
            if (PathFileExistsW(candidatePath.c_str()))
            {
                return candidatePath;
            }

            std::wstring executableRelativeModelDirectory;
            Utility::PathJoin(executableRelativeModelDirectory, Utility::ExecutableDirectory(), modelDirectory);
            Utility::PathJoin(candidatePath, executableRelativeModelDirectory, normalizedReference);
            if (PathFileExistsW(candidatePath.c_str()))
            {
                return candidatePath;
            }

            wchar_t textureFileName[MAX_PATH];
            wcscpy_s(textureFileName, normalizedReference.c_str());
            PathStripPathW(textureFileName);

            Utility::PathJoin(candidatePath, Utility::ExecutableDirectory(), L"Content\\Textures");
            Utility::PathJoin(candidatePath, candidatePath, textureFileName);
            if (PathFileExistsW(candidatePath.c_str()))
            {
                return candidatePath;
            }

            return L"";
        }

        void CreateSolidColorTexture(ID3D11Device* device, ID3D11ShaderResourceView** shaderResourceView, byte red, byte green, byte blue, byte alpha = 255)
        {
            assert(device != nullptr);
            assert(shaderResourceView != nullptr);

            const byte pixel[] = { red, green, blue, alpha };

            D3D11_TEXTURE2D_DESC textureDesc;
            ZeroMemory(&textureDesc, sizeof(textureDesc));
            textureDesc.Width = 1;
            textureDesc.Height = 1;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

            D3D11_SUBRESOURCE_DATA textureData;
            ZeroMemory(&textureData, sizeof(textureData));
            textureData.pSysMem = pixel;
            textureData.SysMemPitch = sizeof(pixel);

            ID3D11Texture2D* texture = nullptr;
            HRESULT hr = device->CreateTexture2D(&textureDesc, &textureData, &texture);
            if (FAILED(hr))
            {
                throw GameException("ID3D11Device::CreateTexture2D() failed.", hr);
            }

            hr = device->CreateShaderResourceView(texture, nullptr, shaderResourceView);
            ReleaseObject(texture);
            if (FAILED(hr))
            {
                throw GameException("ID3D11Device::CreateShaderResourceView() failed.", hr);
            }
        }

        bool TryCreateTextureFromFile(Game& game, const std::wstring& textureName, ID3D11ShaderResourceView** shaderResourceView)
        {
            HRESULT hr = DirectX::CreateWICTextureFromFile(game.Direct3DDevice(), game.Direct3DDeviceContext(), textureName.c_str(), nullptr, shaderResourceView);
            if (SUCCEEDED(hr))
            {
                return true;
            }

            std::wstring message = L"Texture load failed, falling back to solid color: ";
            message += textureName;
            message += L"\n";
            OutputDebugString(message.c_str());

            return false;
        }

        std::wstring ReplaceExtension(const std::wstring& path, const std::wstring& newExtension)
        {
            std::wstring updatedPath(path);
            std::wstring currentExtension = PathFindExtensionW(updatedPath.c_str());
            if (currentExtension.empty())
            {
                return updatedPath + newExtension;
            }

            updatedPath.resize(updatedPath.size() - currentExtension.size());
            updatedPath += newExtension;
            return updatedPath;
        }
    }

    RTTI_DEFINITIONS(ModelFromFile)

    ModelFromFile::ModelFromFile(Game& game, Camera& camera, const std::string modelFilename)
        : DrawableGameComponent(game, camera),  
          mEffect(nullptr), mTechnique(nullptr), mPass(nullptr), mWvpVariable(nullptr), mColorTextureVariable(nullptr),
          mInputLayout(nullptr), mMeshParts(), mWorldMatrix(MatrixHelper::Identity), mShouldMove(true), mMoveSpeed(1.0f), modelFile(modelFilename)
    {
		//we don't use the model description and model value for this constructor
		mModelValue = 0;
    }

	ModelFromFile::ModelFromFile(Game& game, Camera& camera, const std::string modelFilename, const std::wstring ModelDes, int ModelValue) : ModelFromFile(game, camera, modelFilename, ModelDes, ModelValue, ModelMovementSettings())
	{
    	
	}

	ModelFromFile::ModelFromFile(Game& game, Camera& camera, const std::string modelFilename, const std::wstring ModelDes, int ModelValue, const ModelMovementSettings& movementSettings)
		: DrawableGameComponent(game, camera),
		mEffect(nullptr), mTechnique(nullptr), mPass(nullptr), mWvpVariable(nullptr), mColorTextureVariable(nullptr),
		mInputLayout(nullptr), mMeshParts(), mWorldMatrix(MatrixHelper::Identity), mShouldMove(movementSettings.ShouldMove), mMoveSpeed(movementSettings.MoveSpeed), modelFile(modelFilename), modelDes(ModelDes), mModelValue(ModelValue)
	{

	}

    ModelFromFile::~ModelFromFile()
    {
        ReleaseObject(mColorTextureVariable);
        ReleaseObject(mWvpVariable);
        ReleaseObject(mPass);
        ReleaseObject(mTechnique);
        ReleaseObject(mEffect);		
        ReleaseObject(mInputLayout);

		for (MeshPart& meshPart : mMeshParts)
		{
			ReleaseObject(meshPart.TextureShaderResourceView);
			ReleaseObject(meshPart.VertexBuffer);
			ReleaseObject(meshPart.IndexBuffer);
		}
    }



    void ModelFromFile::Initialize()
    {
    	std::wstring defaultTexture = L"Content\\Textures\\bench.jpg";
        const bool usesModelTextureFallback = UsesModelTextureFallback(modelFile);
        SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

        // Compile the shader
        UINT shaderFlags = 0;

	#if defined( DEBUG ) || defined( _DEBUG )
	    shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	#endif

        ID3D10Blob* compiledShader = nullptr;
        ID3D10Blob* errorMessages = nullptr;		
        HRESULT hr = D3DCompileFromFile(L"Content\\Effects\\TextureMapping.fx", nullptr, nullptr, nullptr, "fx_5_0", shaderFlags, 0, &compiledShader, &errorMessages);

		
		if (FAILED(hr))
		{
			const char* errorMessage = (errorMessages != nullptr
				                            ? (char*)errorMessages->GetBufferPointer()
				                            : "D3DX11CompileFromFile() failed");
			GameException ex(errorMessage, hr);
			ReleaseObject(errorMessages);

			throw ex;
		}

        // Create an effect object from the compiled shader
        hr = D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), 0, mGame->Direct3DDevice(), &mEffect);
        if (FAILED(hr))
        {
            throw GameException("D3DX11CreateEffectFromMemory() failed.", hr);
        }

        ReleaseObject(compiledShader);

        // Look up the technique, pass, and WVP variable from the effect
        mTechnique = mEffect->GetTechniqueByName("main11");
        if (mTechnique == nullptr)
        {
            throw GameException("ID3DX11Effect::GetTechniqueByName() could not find the specified technique.", hr);
        }

        mPass = mTechnique->GetPassByName("p0");
        if (mPass == nullptr)
        {
            throw GameException("ID3DX11EffectTechnique::GetPassByName() could not find the specified pass.", hr);
        }

        ID3DX11EffectVariable* variable = mEffect->GetVariableByName("WorldViewProjection");
        if (variable == nullptr)
        {
            throw GameException("ID3DX11Effect::GetVariableByName() could not find the specified variable.", hr);
        }

        mWvpVariable = variable->AsMatrix();
        if (mWvpVariable->IsValid() == false)
        {
            throw GameException("Invalid effect variable cast.");
        }

        variable = mEffect->GetVariableByName("ColorTexture");
        if (variable == nullptr)
        {
            throw GameException("ID3DX11Effect::GetVariableByName() could not find the specified variable.", hr);
        }
        
        mColorTextureVariable = variable->AsShaderResource();
        if (mColorTextureVariable->IsValid() == false)
        {
            throw GameException("Invalid effect variable cast.");
        }

        // Create the input layout
        D3DX11_PASS_DESC passDesc;
        mPass->GetDesc(&passDesc);

        D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        if (FAILED(hr = mGame->Direct3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout)))
        {
            throw GameException("ID3D11Device::CreateInputLayout() failed.", hr);
        }

        // Load the model
        std::unique_ptr<Model> model(new Model(*mGame, modelFile, true));
        if (model->Meshes().empty())
        {
            throw GameException("Model does not contain any meshes.");
        }
        
		float min = -1e38f;
		float max = 1e38f;
		XMFLOAT3 minBoundsVector(max, max, max);
		XMFLOAT3 maxBoundsVector(min, min, min);
		XMVECTOR minBounds = XMLoadFloat3(&minBoundsVector);
		XMVECTOR maxBounds = XMLoadFloat3(&maxBoundsVector);

		for (Mesh* mesh : model->Meshes())
		{
			MeshPart meshPart;
			CreateVertexBuffer(mGame->Direct3DDevice(), *mesh, &meshPart.VertexBuffer, minBounds, maxBounds);
			mesh->CreateIndexBuffer(&meshPart.IndexBuffer);
			meshPart.IndexCount = mesh->Indices().size();

			std::wstring textureName = usesModelTextureFallback ? L"" : defaultTexture;
			ModelMaterial* material = mesh->GetMaterial();
			if (material != nullptr)
			{
				const auto textures = material->Textures();
				auto diffuseTextures = textures.find(TextureTypeDifffuse);
				if (diffuseTextures != textures.end() && diffuseTextures->second != nullptr && diffuseTextures->second->empty() == false)
				{
					std::wstring resolvedTexture = ResolveTexturePath(modelFile, diffuseTextures->second->at(0));
					if (resolvedTexture.empty() == false)
					{
						textureName = resolvedTexture;
					}
				}
			}

			if (textureName.empty())
			{
				CreateSolidColorTexture(mGame->Direct3DDevice(), &meshPart.TextureShaderResourceView, 255, 255, 255);
			}
			else if (TryCreateTextureFromFile(*mGame, textureName, &meshPart.TextureShaderResourceView) == false)
			{
				std::wstring extension = PathFindExtensionW(textureName.c_str());
				if (_wcsicmp(extension.c_str(), L".tga") == 0)
				{
					std::wstring pngTextureName = ReplaceExtension(textureName, L".png");
					if (TryCreateTextureFromFile(*mGame, pngTextureName, &meshPart.TextureShaderResourceView) == false)
					{
						CreateSolidColorTexture(mGame->Direct3DDevice(), &meshPart.TextureShaderResourceView, 255, 255, 255);
					}
				}
				else
				{
					CreateSolidColorTexture(mGame->Direct3DDevice(), &meshPart.TextureShaderResourceView, 255, 255, 255);
				}
			}

			mMeshParts.push_back(meshPart);
		}

		XMStoreFloat3(const_cast<XMFLOAT3*>(&mBoundingBox.Center), 0.5f * (minBounds + maxBounds));
		XMStoreFloat3(const_cast<XMFLOAT3*>(&mBoundingBox.Extents), 0.5f * (maxBounds - minBounds));

		


        //position model in the world space, the issue here is that models are from different sources need adjustment for scaling, rotation,
		/*
		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX RotationZ = XMMatrixRotationZ(-1.57f);
	    XMMATRIX RotationX = XMMatrixRotationX(-1.57f);
		XMMATRIX RotationY = XMMatrixRotationY(-1.57f);
		XMMATRIX Scale = XMMatrixScaling( 0.05f, 0.05f, 0.05f );
	    XMMATRIX Translation = XMMatrixTranslation( 0.0f, 0.0f, 0.0f );
	    worldMatrix= RotationZ*RotationX *RotationY* Scale * Translation;

	    XMStoreFloat4x4(&mWorldMatrix, worldMatrix);
*/
    }


	void ModelFromFile::SetPosition(const float rotateX, const float rotateY, const float rotateZ, const float scaleFactor, const float translateX, const float translateY, const float translateZ)
	{
		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX RotationZ = XMMatrixRotationZ(rotateZ);
		XMMATRIX RotationX = XMMatrixRotationX(rotateX);
		XMMATRIX RotationY = XMMatrixRotationY(rotateY);
		XMMATRIX Scale = XMMatrixScaling(scaleFactor, scaleFactor, scaleFactor);
		XMMATRIX Translation = XMMatrixTranslation(translateX, translateY, translateZ);
		worldMatrix = RotationZ*RotationX *RotationY* Scale * Translation;

		XMStoreFloat4x4(&mWorldMatrix, worldMatrix);
    	mBoundingBox.Transform(mWorldBox, worldMatrix);
	}



	void ModelFromFile::Update(const GameTime& gameTime)
	{
	//	XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
	//	mAngle += XM_PI * static_cast<float> (gameTime.ElapsedGameTime());
		
	//	mAngle = 0.002 * XM_PI;

	//	worldMatrix =   worldMatrix * XMMatrixRotationY(mAngle);
	//	XMStoreFloat4x4(&mWorldMatrix, XMMatrixRotationX(mAngle));

	//	XMStoreFloat4x4(&mWorldMatrix, worldMatrix);

    	if (mShouldMove)
    	{
    		MoveModel(gameTime, mMoveSpeed);
    	}
	
    	XMMATRIX world = XMLoadFloat4x4(&mWorldMatrix);
    	mBoundingBox.Transform(mWorldBox, world);
		
	}
	void ModelFromFile::MoveModel(const GameTime& gameTime, float speed)
    {
    	float deltaTime = static_cast<float>(gameTime.ElapsedGameTime());

    	XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);

    	XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, speed * deltaTime);

    	worldMatrix = worldMatrix * translation;

    	XMStoreFloat4x4(&mWorldMatrix, worldMatrix);
    }

    void ModelFromFile::Draw(const GameTime& gameTime)
    {
        ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();        
        direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        direct3DDeviceContext->IASetInputLayout(mInputLayout);

        XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
        XMMATRIX wvp = worldMatrix * mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
        mWvpVariable->SetMatrix(reinterpret_cast<const float*>(&wvp));

		for (const MeshPart& meshPart : mMeshParts)
		{
			UINT stride = sizeof(TextureMappingVertex);
			UINT offset = 0;
			direct3DDeviceContext->IASetVertexBuffers(0, 1, &meshPart.VertexBuffer, &stride, &offset);
			direct3DDeviceContext->IASetIndexBuffer(meshPart.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
			mColorTextureVariable->SetResource(meshPart.TextureShaderResourceView);
			mPass->Apply(0, direct3DDeviceContext);
			direct3DDeviceContext->DrawIndexed(meshPart.IndexCount, 0, 0);
		}
    }

    void ModelFromFile::CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer, XMVECTOR& minBounds, XMVECTOR& maxBounds) const
    {
        const std::vector<XMFLOAT3>& sourceVertices = mesh.Vertices();

        std::vector<TextureMappingVertex> vertices;
        vertices.reserve(sourceVertices.size());
        
        std::vector<XMFLOAT3>* textureCoordinates = mesh.TextureCoordinates().at(0);
        assert(textureCoordinates->size() == sourceVertices.size());
          

        for (UINT i = 0; i < sourceVertices.size(); i++)
        {
            XMFLOAT3 position = sourceVertices.at(i);
            XMFLOAT3 uv = textureCoordinates->at(i);
            vertices.push_back(TextureMappingVertex(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y)));

			XMVECTOR P = XMLoadFloat3(&position);
			minBounds = XMVectorMin(minBounds, P);
			maxBounds = XMVectorMax(maxBounds, P);
        }

		
        D3D11_BUFFER_DESC vertexBufferDesc;
        ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
        vertexBufferDesc.ByteWidth = sizeof(TextureMappingVertex) * vertices.size();
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;		
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexSubResourceData;
        ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
        vertexSubResourceData.pSysMem = &vertices[0];
        if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer)))
        {
            throw GameException("ID3D11Device::CreateBuffer() failed.");
        }
    }
}
