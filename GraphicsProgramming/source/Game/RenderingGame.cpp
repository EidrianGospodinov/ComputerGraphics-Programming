#include "RenderingGame.h"
#include "GameException.h"
#include "FirstPersonCamera.h"
#include "TriangleDemo.h"
#include "ModelFromFile.h"
namespace Rendering
{;

	const XMFLOAT4 RenderingGame::BackgroundColor = { 0.0f, 0.0f, 0.0f, 0.0f }; //Changes the background color of the scene

    RenderingGame::RenderingGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand)
        :  Game(instance, windowClass, windowTitle, showCommand),
        mDemo(nullptr)
    {
        mDepthStencilBufferEnabled = true;
        mMultiSamplingEnabled = true;

        mDirectInput = nullptr;
		mMouse = nullptr;
		mKeyboard = nullptr;
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

        if (FAILED(DirectInput8Create(mInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&mDirectInput, nullptr)))
        {
            throw GameException("DirectInput8Create() failed.");
        }
        //Initialises the keyboard
		mKeyboard = new Keyboard(*this, mDirectInput);
		mComponents.push_back(mKeyboard);
		mServices.AddService(Keyboard::TypeIdClass(), mKeyboard);
       
        // Initialises the mouse
		mMouse = new Mouse(*this, mDirectInput);
        mComponents.push_back(mMouse);
		mServices.AddService(Mouse::TypeIdClass(), mMouse);

		HRESULT CreateDevice(REFGUID rguid, LPDIRECTINPUTDEVICE8 * lplpDirectInputDevice, LPUNKNOWN pUnkOuter);
        //Add the following code in Initialize() to create and position the model object :
        mModel = new ModelFromFile(*this, *mCamera, "Content\\Models\\bench.3ds"); //Gets mesh
        mModel->SetPosition(-1.57f, -0.0f, -0.0f, 0.005f, 2.0f, 0.6f, 0.0f); //Sets mesh position
        mComponents.push_back(mModel); //Puts the model as a component in the scene
        //Add the following code in Initialize() to create and position the model object :
        mModel = new ModelFromFile(*this, *mCamera, "Content\\Models\\bench.3ds"); //Gets mesh
        mModel->SetPosition(-1.57f, -0.0f, -0.0f, 0.005f, -2.0f, 0.6f, 0.0f); //Sets mesh position
        mComponents.push_back(mModel); //Puts the model as a component in the scene
        
        mFpsComponent = new FpsComponent(*this);
        mFpsComponent->Initialize();

        mRenderStateHelper = new RenderStateHelper(*this);

        Game::Initialize(); //Makes the game render stuff
		mCamera->SetPosition(0.0f, 0.0f, 5.0f);

       
    }

    void RenderingGame::Shutdown()
    {
		DeleteObject(mDemo);
        DeleteObject(mCamera);
        DeleteObject(mKeyboard);
        DeleteObject(mMouse);
        DeleteObject(mFpsComponent);
        DeleteObject(mRenderStateHelper);

		ReleaseObject(mDirectInput); 
        Game::Shutdown();
    }

    void RenderingGame::Update(const GameTime &gameTime)
    {
     
		//Exists the game when the escape key is pressed
        if (mKeyboard->WasKeyPressedThisFrame(DIK_ESCAPE))
        {
            Exit();
		}
        mFpsComponent->Update(gameTime);
        Game::Update(gameTime);


    }

    void RenderingGame::Draw(const GameTime &gameTime)
    {
        mDirect3DDeviceContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&BackgroundColor));
        mDirect3DDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        Game::Draw(gameTime);
       
        mRenderStateHelper->SaveAll();
        mFpsComponent->Draw(gameTime);
        mRenderStateHelper->RestoreAll();
        HRESULT hr = mSwapChain->Present(0, 0);
        if (FAILED(hr))
        {
            throw GameException("IDXGISwapChain::Present() failed.", hr);
        }

    }
}