#include "RenderingGame.h"
#include "GameException.h"
#include "FirstPersonCamera.h"
#include "TriangleDemo.h"

#include "Keyboard.h"
#include "Mouse.h"
#include "ModelFromFile.h"
#include "FpsComponent.h"
#include "RenderStateHelper.h"
#include "ObjectDiffuseLight.h"
#include "SamplerStates.h"
#include "RasterizerStates.h"

//display score
#include <SpriteFont.h>
#include <sstream>

namespace Rendering
{;

	const XMFLOAT4 RenderingGame::BackgroundColor = { 0.75f, 0.75f, 0.75f, 1.0f };

	RenderingGame::RenderingGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand)
		: Game(instance, windowClass, windowTitle, showCommand),
		mDemo(nullptr), mDirectInput(nullptr), mKeyboard(nullptr), mMouse(nullptr), mModel1(nullptr),
		mFpsComponent(nullptr), mRenderStateHelper(nullptr), mObjectDiffuseLight(nullptr)
    {
        mDepthStencilBufferEnabled = true;
        mMultiSamplingEnabled = true;
		mModel2 = nullptr;

		mSpriteBatch =nullptr;
		mSpriteFont = nullptr;
    }

    RenderingGame::~RenderingGame()
    {
    }

    void RenderingGame::Initialize()
    {
		
        mCamera = new FirstPersonCamera(*this);
        mComponents.push_back(mCamera);
        mServices.AddService(Camera::TypeIdClass(), mCamera);
    
        mDemo = new TriangleDemo(*this, *mCamera);
        mComponents.push_back(mDemo);

		//Remember that the component is a management class for all objects in the D3D rendering engine. 
		//It provides a centralized place to create and release objects. 
	    //NB: In C++ and other similar languages, to instantiate a class is to create an object.
		if (FAILED(DirectInput8Create(mInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&mDirectInput, nullptr)))
		{
			throw GameException("DirectInput8Create() failed");
		}

		mKeyboard = new Keyboard(*this, mDirectInput);
		mComponents.push_back(mKeyboard);
		mServices.AddService(Keyboard::TypeIdClass(), mKeyboard);

		mMouse = new Mouse(*this, mDirectInput);
		mComponents.push_back(mMouse);
		mServices.AddService(Mouse::TypeIdClass(), mMouse);

     
		
		mModel1 = new ModelFromFile(*this, *mCamera, "Content\\Models\\bench.3ds", L"A Bench",20);
		mModel1->SetPosition(-1.57f, 1.5f, -0.0f, 0.005f, -2.0f, -0.6f, 2.0f);
		mComponents.push_back(mModel1);

		mModel2 = new ModelFromFile(*this, *mCamera, "Content\\Models\\bench.3ds", L"A bench",10);
		mModel2->SetPosition(-1.57f, -1.5f, -0.0f, 0.005f, 5.0f, -0.6f, 2.0f);
		mComponents.push_back(mModel2);

		//house object with diffuse lighting effect:
		/*mObjectDiffuseLight = new ObjectDiffuseLight(*this, *mCamera);
		mObjectDiffuseLight->SetPosition(-1.57f, -0.0f, -0.0f, 0.01f, -1.0f, 0.75f, -2.5f);
		mComponents.push_back(mObjectDiffuseLight);*/
		
		RasterizerStates::Initialize(mDirect3DDevice);
		SamplerStates::Initialize(mDirect3DDevice);

	
		mFpsComponent = new FpsComponent(*this);
		mFpsComponent->Initialize();
		mRenderStateHelper = new RenderStateHelper(*this);
		
		mSpriteBatch = new SpriteBatch(mDirect3DDeviceContext);
		mSpriteFont = new SpriteFont(mDirect3DDevice,
		L"Content\\Fonts\\Arial_14_Regular.spritefont");


		Game::Initialize();

        mCamera->SetPosition(0.0f, 1.0f, 10.0f);

		



    }

    void RenderingGame::Shutdown()
    {
		

		
		DeleteObject(mDemo);
        DeleteObject(mCamera);
		
		
		DeleteObject(mKeyboard);
		DeleteObject(mMouse);
		ReleaseObject(mDirectInput);
		
		DeleteObject(mModel1);
		DeleteObject(mModel2);
		
	

		DeleteObject(mFpsComponent);
		DeleteObject(mRenderStateHelper);

		DeleteObject(mObjectDiffuseLight);

		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);
		

        Game::Shutdown();
    }

    void RenderingGame::Update(const GameTime &gameTime)
    {

		mFpsComponent->Update(gameTime);
		Game::Update(gameTime);
		

		//Add "ESC" to exit the application
		if (mKeyboard->WasKeyPressedThisFrame(DIK_ESCAPE))
		{
			Exit();
		}


		//bounding box , we need to see if we need to do the picking test
		if (Game::toPick)
		{
			
			if (mModel1->Visible())
				Pick(Game::screenX, Game::screenY, mModel1);
			if (mModel2->Visible())
				Pick(Game::screenX, Game::screenY, mModel2);
	
			
			
			Game::toPick = false;
		}

	}


	// do the picking here
	void RenderingGame::Pick(int sx, int sy, ModelFromFile* model)
	{
		//XMMATRIX P = mCam.Proj();
		XMFLOAT4X4 P;
		XMStoreFloat4x4(&P, mCamera->ProjectionMatrix());
		//Compute picking ray in view space.
		float vx = (+2.0f*sx / Game::DefaultScreenWidth - 1.0f) / P(0, 0);
		float vy = (-2.0f*sy / Game::DefaultScreenHeight + 1.0f) / P(1, 1);
		// Ray definition in view space.
		XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMVECTOR rayDir = XMVectorSet(vx, vy, -1.0f, 0.0f);
		// Tranform ray to local space of Mesh via the inverse of both of view and world transform
		XMMATRIX V = mCamera->ViewMatrix();
		XMVECTOR vDeterminant = XMMatrixDeterminant(V);
		XMMATRIX invView = XMMatrixInverse(&vDeterminant, V);
		XMMATRIX W = XMLoadFloat4x4(model->WorldMatrix());
		XMVECTOR vWorld = XMMatrixDeterminant(W);
		XMMATRIX invWorld = XMMatrixInverse(&vWorld, W);
		XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);
		rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
		rayDir = XMVector3TransformNormal(rayDir, toLocal);
		// Make the ray direction unit length for the intersection tests. Dr. Li Jin©
		rayDir = XMVector3Normalize(rayDir);
		float tmin = 0.0;
		if (model->mBoundingBox.Intersects(rayOrigin, rayDir, tmin))
		{
			std::wostringstream pickupString;
			pickupString << L"Do you want to pick up: " <<
			(model->GetModelDes()).c_str()<<'\n'<<'\t'<<'+'<<model->ModelValue()<<L"points";
			int result = MessageBox(0, pickupString.str().c_str(), L"Object Found",
			MB_ICONASTERISK | MB_YESNO);
			//To make the object invisible after being picked, in the Pick function, add the following code:
			if (result == IDYES)
			{ //hide the object
				model->SetVisible(false);

				//update the score
				mScore += model->ModelValue();
			}
		}
	}

	

    void RenderingGame::Draw(const GameTime &gameTime)
    {
        mDirect3DDeviceContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&BackgroundColor));
        mDirect3DDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        Game::Draw(gameTime);
		mRenderStateHelper->SaveAll();
		mFpsComponent->Draw(gameTime);

		
		mSpriteBatch->Begin();
		//draw the score
		std::wostringstream scoreLabel;
		scoreLabel << L"Your current score: " << mScore<< "\n";
		mSpriteFont->DrawString(mSpriteBatch, scoreLabel.str().c_str(),
		XMFLOAT2(0.0f, 120.0f), Colors::Red);
		mSpriteBatch->End();
		
		mRenderStateHelper->RestoreAll();

       
        HRESULT hr = mSwapChain->Present(0, 0);
        if (FAILED(hr))
        {
            throw GameException("IDXGISwapChain::Present() failed.", hr);
        }


		

    }
}