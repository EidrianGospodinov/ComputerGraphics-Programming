#include "TriangleDemo.h"
#include "Game.h"
#include "GameException.h"
#include "MatrixHelper.h"
#include "Camera.h"
#include "Utility.h"
#include "D3DCompiler.h"

namespace Rendering
{
    RTTI_DEFINITIONS(TriangleDemo)

    TriangleDemo::TriangleDemo(Game& game, Camera& camera)
        : DrawableGameComponent(game, camera),
          mEffect(nullptr), mTechnique(nullptr), mPass(nullptr), mWvpVariable(nullptr),
          mInputLayout(nullptr), mWorldMatrix(MatrixHelper::Identity), mVertexBuffer(nullptr), mAngle(0.0f)
    {
    }

    TriangleDemo::~TriangleDemo()
    {
        ReleaseObject(mWvpVariable);
        ReleaseObject(mPass);
        ReleaseObject(mTechnique);
        ReleaseObject(mEffect);		
        ReleaseObject(mInputLayout);
        ReleaseObject(mVertexBuffer);
    }

    void TriangleDemo::Initialize()
    {
        SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

        // Compile the shader
        UINT shaderFlags = 0;

#if defined( DEBUG ) || defined( _DEBUG )
    shaderFlags |= D3DCOMPILE_DEBUG;
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        ID3D10Blob* compiledShader = nullptr;
        ID3D10Blob* errorMessages = nullptr;	
		
		//1. load the effect file (vertex and pixel shader)
		//insert code here
        
	

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

        // Create the input layout
        D3DX11_PASS_DESC passDesc;
        mPass->GetDesc(&passDesc);

		//2. create the vetex layout
		//insert code here
		
    


        // 3. Create the vertex buffer
		//insert code here
	
    
		
    }

	void TriangleDemo::Update(const GameTime& gameTime)
	{

	}

    void TriangleDemo::Draw(const GameTime& gameTime)
    {
		//4. draw function
		//insert the code here

     
    }
}