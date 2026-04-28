#include "RenderingGame.h"
#include "GameException.h"
#include "FirstPersonCamera.h"
#include "TriangleDemo.h"

#include "Keyboard.h"
#include "Mouse.h"
#include "ModelFromFile.h"
#include "Projectile.h"
#include "FpsComponent.h"
#include "RenderStateHelper.h"
#include "ObjectDiffuseLight.h"
#include "MenuComponent.h"
#include "SamplerStates.h"
#include "RasterizerStates.h"

//display score
#include <SpriteFont.h>
#include <sstream>
#include <algorithm>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

namespace Rendering
{;

	const XMFLOAT4 RenderingGame::BackgroundColor = { 0.75f, 0.75f, 0.75f, 1.0f };

	RenderingGame::RenderingGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand)
		: Game(instance, windowClass, windowTitle, showCommand),
		mDemo(nullptr), mDirectInput(nullptr), mKeyboard(nullptr), mMouse(nullptr), mModel1(nullptr),
		mFpsComponent(nullptr), mRenderStateHelper(nullptr), mObjectDiffuseLight(nullptr),
		mMenu(nullptr), mGameState(GameState::Menu), mLastFireTime(0.0f), mPickupMessageTimer(0.0f),
		mCurrentWave(0), mWaveState(WaveState::WaitingToStart), mWaveStateTimer(1.5f), mWaveElapsed(0.0f), mInfiniteMode(false), mGameOverTimer(0.0f)
    {
        mDepthStencilBufferEnabled = true;
        mMultiSamplingEnabled = true;

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

     
		// Pooled collidables — all use ModelMovementSettings(true, 2.5f) so they drift forward
		// (toward camera at +Z) once spawned by the wave system. Faster speed than the default 1.0.
		ModelMovementSettings collidableMove(true, 2.5f);
		ModelMovementSettings starMove(true, 2.5f, 3.0f);  // stars also spin around Y at 3 rad/sec

		mModel1 = new ModelFromFile(*this, *mCamera, "Content\\Models\\bench.3ds", L"A Bench", -20, collidableMove);
		mModel1->SetPosition(-1.57f, 0.0f, -0.0f, 0.005f, -2.0f, 0.6f, 2.0f);
		mComponents.push_back(mModel1);
		mCollidableModels.push_back(mModel1);

		auto starModel = new ModelFromFile(*this, *mCamera, "Content\\Models\\Star.obj", L"A Star", 20, starMove);
		starModel->SetPosition(-1.57f, 0.0f, -0.0f, 0.05f, -2.0f, 0.6f, 0.0f);
		mComponents.push_back(starModel);
		mCollidableModels.push_back(starModel);

		auto treeModel = new ModelFromFile(*this, *mCamera, "Content\\Models\\Orb.fbx", L"Red Orb", -30, collidableMove);
		treeModel->SetPosition(0.0f, 0.0f, 0.0f, 0.50f, 4.0f, 0.0f, 4.0f);
		mComponents.push_back(treeModel);
		mCollidableModels.push_back(treeModel);

		auto bench3 = new ModelFromFile(*this, *mCamera, "Content\\Models\\bench.3ds", L"A Bench", -20, collidableMove);
		bench3->SetPosition(-1.57f, 0.0f, 0.0f, 0.005f, 0.0f, 0.6f, 0.0f);
		mComponents.push_back(bench3);
		mCollidableModels.push_back(bench3);

		auto bench4 = new ModelFromFile(*this, *mCamera, "Content\\Models\\bench.3ds", L"A Bench", -20, collidableMove);
		bench4->SetPosition(-1.57f, 0.0f, 0.0f, 0.005f, 0.0f, 0.6f, 0.0f);
		mComponents.push_back(bench4);
		mCollidableModels.push_back(bench4);

		auto star2 = new ModelFromFile(*this, *mCamera, "Content\\Models\\Star.obj", L"A Star", 20, starMove);
		star2->SetPosition(-1.57f, 0.0f, 0.0f, 0.05f, 0.0f, 0.6f, 0.0f);
		mComponents.push_back(star2);
		mCollidableModels.push_back(star2);

		auto tree2 = new ModelFromFile(*this, *mCamera, "Content\\Models\\Orb.fbx", L"Red Orb", -30, collidableMove);
		tree2->SetPosition(0.0f, 0.0f, 0.0f, 0.50f, 0.0f, 0.0f, 0.0f);
		mComponents.push_back(tree2);
		mCollidableModels.push_back(tree2);


		// All collidables start hidden — wave system spawns them
		for (auto* m : mCollidableModels) m->SetVisible(false);

		// Wave configuration: 4, 6, 8 objects
		mWaveCounts = { 4, 6, 8 };

		// Spawn points: 8 staggered positions. FirstPersonCamera clamps X to [-2.2, +2.2],
		// so X is kept within ±2. Z values are deliberately varied (not in straight rows) so
		// objects aren't parallel — the player has to navigate forward as well as side-to-side.
		mSpawnPoints = {
			{-2.0f, 0.6f, -2.5f},
			{-0.7f, 0.6f, -5.0f},
			{ 0.7f, 0.6f, -3.5f},
			{ 2.0f, 0.6f, -6.5f},
			{-2.0f, 0.6f, -8.5f},
			{-0.7f, 0.6f, -10.5f},
			{ 0.7f, 0.6f, -9.0f},
			{ 2.0f, 0.6f, -11.5f},
		};

		// Car pool — shootable, not collidable. 3 cars for waves 1/2/3 (1, 2, 3 cars per wave).
		// All start hidden; the wave system positions and shows them on wave start.
		ModelMovementSettings carMove(true, 5.0f);
		for (int i = 0; i < 3; i++)
		{
			auto car = new ModelFromFile(*this, *mCamera, "Content\\Models\\Car.obj", L"A Car", 50, carMove);
			car->SetPosition(1.0f, 0.0f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f);
			car->SetVisible(false);
			mComponents.push_back(car);
			mShootableModels.push_back(car);
			mCars.push_back(car);
		}
		//house object with diffuse lighting effect:
		mObjectDiffuseLight = new ObjectDiffuseLight(*this, *mCamera);
		mObjectDiffuseLight->SetPosition(-1.57f, -0.0f, -0.0f, 0.01f, -1.0f, 0.75f, -2.5f);
		mComponents.push_back(mObjectDiffuseLight);
		
		RasterizerStates::Initialize(mDirect3DDevice);
		SamplerStates::Initialize(mDirect3DDevice);

	
		mFpsComponent = new FpsComponent(*this);
		mFpsComponent->Initialize();
		mRenderStateHelper = new RenderStateHelper(*this);

		mSpriteBatch = new SpriteBatch(mDirect3DDeviceContext);
		mSpriteFont = new SpriteFont(mDirect3DDevice,
		L"Content\\Fonts\\Arial_14_Regular.spritefont");

		mMenu = new MenuComponent(*this, mSpriteBatch, mSpriteFont, mKeyboard);
		mMenu->Initialize();

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


		for (auto model : mCollidableModels)
		{
			DeleteObject(model);
		}
		mCollidableModels.clear();

		for (auto model : mShootableModels)
		{
			DeleteObject(model);
		}
		mShootableModels.clear();

		for (auto projectile : mProjectiles)
		{
			ModelFromFile* model = projectile->GetModel();
			if (model != nullptr) delete model;
			DeleteObject(projectile);
		}
		mProjectiles.clear();

		mModel1 = nullptr;
		mCars.clear();

		DeleteObject(mFpsComponent);
		DeleteObject(mRenderStateHelper);

		DeleteObject(mObjectDiffuseLight);

		DeleteObject(mMenu);
		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);


        Game::Shutdown();
    }

    void RenderingGame::LookForCollsion_Manual(BoundingSphere cameraSphere)
    {
	    // 3. Collision Check
	    bool collision = false;
	    float obsticleValue = 0;
	    if (mModel1 != nullptr && mModel1->Visible() && cameraSphere.Intersects(mModel1->mWorldBox))
	    {
		    collision = true;
		    obsticleValue = mModel1->ModelValue();
	    }

	    if (collision)
	    {
		    OutputDebugString(L"HIT\n");
		    mScore += obsticleValue;
	    }
    }

    void RenderingGame::DetectingCollsion_AllCollidables(BoundingSphere cameraSphere)
    {
	    for (ModelFromFile* model : mCollidableModels)
	    {
		    if (!model->Visible()) continue;

		    if (cameraSphere.Intersects(model->mWorldBox))
		    {
			    OutputDebugString(L"Player hit a model!\n");
			    mScore += model->ModelValue();
				ShowPickupMessage(model->ModelValue());
		    	model->SetVisible(false);
			    break;
		    }
	    }
    }

	void RenderingGame::ShowPickupMessage(int points)
	{
		std::wostringstream ss;
		if (points >= 0) ss << L"+" << points << L" POINTS!";
		else ss << points << L" POINTS!";
		mPickupMessage = ss.str();
		mPickupMessageTimer = PICKUP_MESSAGE_DURATION;
	}

	void RenderingGame::StartWave(int waveIndex)
	{
		int n = mWaveCounts[waveIndex];

		// Hide all cars first, then activate (waveIndex + 1) cars at random positions.
		// Wave 1 = 1 car, Wave 2 = 2 cars, Wave 3 = 3 cars.
		for (auto* car : mCars) car->SetVisible(false);

		int carsThisWave = waveIndex + 1;
		if (carsThisWave > (int)mCars.size()) carsThisWave = (int)mCars.size();

		for (int i = 0; i < carsThisWave; i++)
		{
			float carX = ((rand() % 2001) / 100.0f) - 10.0f;  // -10 to +10
			float carZ = -((rand() % 1500) / 100.0f) - 2.0f;  // -2 to -17
			mCars[i]->SetPosition(1.0f, 0.0f, 0.0f, 0.5f, carX, 1.0f, carZ);
			mCars[i]->SetVisible(true);
		}

		// Pick spawn points without repeats
		std::vector<XMFLOAT3> shuffled = mSpawnPoints;
		for (int i = (int)shuffled.size() - 1; i > 0; --i)
		{
			int j = rand() % (i + 1);
			std::swap(shuffled[i], shuffled[j]);
		}

		int spawned = 0;
		for (auto* model : mCollidableModels)
		{
			if (spawned >= n) break;
			if (!model->Visible() && spawned < (int)shuffled.size())
			{
				const auto& p = shuffled[spawned];

				// Use the appropriate scale for each model type (case-insensitive substring match
				// since some models were created with inconsistent description casing)
				std::wstring desc = model->GetModelDes();
				std::wstring lower = desc;
				std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);

				float scale = 0.05f;
				if (lower.find(L"bench") != std::wstring::npos) scale = 0.003f;
				else if (lower.find(L"star") != std::wstring::npos) scale = 0.05f;
				else if (lower.find(L"tree") != std::wstring::npos || lower.find(L"oak") != std::wstring::npos) scale = 0.30f;
				else if (lower.find(L"orb") != std::wstring::npos) scale = 0.30f;

				model->SetPosition(-1.57f, 0.0f, 0.0f, scale, p.x, p.y, p.z);
				model->SetVisible(true);
				spawned++;
			}
		}
	}

	void RenderingGame::RestartGame()
	{
		// Reset score and HUD state
		mScore = 0;
		mPickupMessage.clear();
		mPickupMessageTimer = 0.0f;

		// Reset wave state
		mCurrentWave = 0;
		mWaveState = WaveState::WaitingToStart;
		mWaveStateTimer = WAVE_START_DELAY;
		mWaveElapsed = 0.0f;
		mInfiniteMode = false;

		// Hide all collidables and cars — wave system will respawn them
		for (auto* m : mCollidableModels) m->SetVisible(false);
		for (auto* car : mCars) car->SetVisible(false);

		// Clear active projectiles and their models
		for (auto* projectile : mProjectiles)
		{
			auto it = std::find(mComponents.begin(), mComponents.end(), projectile);
			if (it != mComponents.end()) mComponents.erase(it);

			ModelFromFile* model = projectile->GetModel();
			if (model != nullptr)
			{
				auto mit = std::find(mComponents.begin(), mComponents.end(), model);
				if (mit != mComponents.end()) mComponents.erase(mit);
				delete model;
			}
			delete projectile;
		}
		mProjectiles.clear();
		mLastFireTime = 0.0f;

		// Reset camera position
		if (mCamera != nullptr)
		{
			mCamera->SetPosition(0.0f, 1.0f, 10.0f);
		}
	}

	void RenderingGame::BeginGameOver()
	{
		mGameState = GameState::GameOver;
		mGameOverTimer = GAME_OVER_DURATION;
	}

	void RenderingGame::UpdateGameOver(const GameTime& gameTime)
	{
		mGameOverTimer -= (float)gameTime.ElapsedGameTime();
		if (mGameOverTimer > 0.0f)
		{
			return;
		}

		RestartGame();
		mGameState = GameState::Menu;
		mMenu->SetMenuMode(MenuMode::MainMenu);
		mMenu->ResetConfirmed();
	}

	void RenderingGame::UpdateWaves(const GameTime& gameTime)
	{
		float dt = (float)gameTime.ElapsedGameTime();

		if (mWaveState == WaveState::WaitingToStart)
		{
			mWaveStateTimer -= dt;
			if (mWaveStateTimer <= 0.0f)
			{
				StartWave(mCurrentWave);
				mWaveState = WaveState::Spawning;
				mWaveElapsed = 0.0f;
			}
		}
		else if (mWaveState == WaveState::GameWon)
		{
			mWaveStateTimer -= dt;
			if (mWaveStateTimer <= 0.0f)
			{
				mWaveState = WaveState::WaitingToStart;
				mWaveStateTimer = WAVE_START_DELAY;
			}
		}
		else if (mWaveState == WaveState::Spawning)
		{
			mWaveElapsed += dt;

			// Auto-hide collidables that have drifted past the player. Objects move at +Z toward
			// the camera; once they're behind the player there's no way to interact with them,
			// so treat them as "done" and let the wave end naturally.
			float playerZ = 10.0f;
			if (mCamera != nullptr)
			{
				XMFLOAT3 camPos;
				XMStoreFloat3(&camPos, mCamera->PositionVector());
				playerZ = camPos.z;
			}
			for (auto* m : mCollidableModels)
			{
				if (!m->Visible()) continue;
				float z = m->WorldMatrix()->_43;
				if (z > playerZ + 1.0f) m->SetVisible(false);
			}

			int liveCount = 0;
			for (auto* m : mCollidableModels) if (m->Visible()) liveCount++;

			// Wave ends when all collected OR the wave time limit is reached
			bool waveOver = (liveCount == 0) || (mWaveElapsed >= WAVE_MAX_DURATION);

			if (waveOver)
			{
				// If timer ran out, hide any leftovers so the next wave starts clean
				if (mWaveElapsed >= WAVE_MAX_DURATION)
				{
					for (auto* m : mCollidableModels) m->SetVisible(false);
				}

				mCurrentWave++;
				if (mCurrentWave >= (int)mWaveCounts.size())
				{
					if (!mInfiniteMode)
					{
						mInfiniteMode = true;
						mCurrentWave = 0;
						mWaveState = WaveState::GameWon;
						mWaveStateTimer = INFINITE_MODE_DELAY;
					}
					else
					{
						mCurrentWave = 0;
						mWaveState = WaveState::WaitingToStart;
						mWaveStateTimer = WAVE_START_DELAY;
					}
				}
				else
				{
					mWaveState = WaveState::WaitingToStart;
					mWaveStateTimer = WAVE_START_DELAY;
				}
			}
		}

		// Out-of-world check for moving cars — teleport any car that strays past WORLD_BOUNDS
		for (auto* car : mCars)
		{
			if (!car->Visible()) continue;
			XMFLOAT4X4* w = car->WorldMatrix();
			float x = w->_41;
			float z = w->_43;
			if (fabsf(x) > WORLD_BOUNDS || fabsf(z) > WORLD_BOUNDS)
			{
				float carX = ((rand() % 2001) / 100.0f) - 10.0f;
				float carZ = -((rand() % 1500) / 100.0f) - 2.0f;
				car->SetPosition(1.0f, 0.0f, 0.0f, 0.5f, carX, 1.0f, carZ);
			}
		}
	}

    void RenderingGame::Update(const GameTime &gameTime)
    {
		if (mKeyboard != nullptr)
			mKeyboard->Update(gameTime);

		mMenu->Update(gameTime);

		if (mGameState == GameState::Menu)
		{
			if (mMenu->IsConfirmed())
			{
				if (mMenu->GetSelectedOption() == 0)
				{
					mGameState = GameState::Playing;
				}
				else if (mMenu->GetSelectedOption() == 1)
				{
					Exit();
				}
				mMenu->ResetConfirmed();
			}
			return;
		}

		if (mGameState == GameState::Paused)
		{
			if (mMenu->IsConfirmed())
			{
				int sel = mMenu->GetSelectedOption();
				if (sel == 0)        // CONTINUE
				{
					mGameState = GameState::Playing;
					mMenu->SetMenuMode(MenuMode::MainMenu);
				}
				else if (sel == 1)   // RESTART
				{
					RestartGame();
					mGameState = GameState::Playing;
					mMenu->SetMenuMode(MenuMode::MainMenu);
				}
				else if (sel == 2)   // QUIT
				{
					Exit();
				}
				mMenu->ResetConfirmed();
			}
			return;
		}

		if (mGameState == GameState::GameOver)
		{
			UpdateGameOver(gameTime);
			return;
		}

		// ESC key to pause (check before Game::Update which updates keyboard again)
		if (mKeyboard->WasKeyPressedThisFrame(DIK_ESCAPE))
		{
			mGameState = GameState::Paused;
			mMenu->SetMenuMode(MenuMode::PauseMenu);
			mMenu->ResetConfirmed();
			return;
		}

		// Only update game when Playing
		mFpsComponent->Update(gameTime);
		Game::Update(gameTime);

		if (mPickupMessageTimer > 0.0f)
		{
			mPickupMessageTimer -= (float)gameTime.ElapsedGameTime();
		}

		UpdateWaves(gameTime);

		// Handle shooting
		mLastFireTime += (float)gameTime.ElapsedGameTime();
		if (mMouse != nullptr && mMouse->IsButtonDown(MouseButtonsRight))
		{
			if (mLastFireTime >= FIRE_COOLDOWN)
			{
				// Get camera direction
				XMVECTOR cameraDir = mCamera->DirectionVector();
				XMFLOAT3 direction;
				XMStoreFloat3(&direction, cameraDir);

				// Get camera position, spawn slightly ahead of camera so it's visible
				XMFLOAT3 cameraPos;
				XMStoreFloat3(&cameraPos, mCamera->PositionVector());
				cameraPos.x += direction.x * 2.0f;
				cameraPos.y += direction.y * 2.0f;
				cameraPos.z += direction.z * 2.0f;

				// Create projectile
				Projectile* projectile = new Projectile(*this, *mCamera, cameraPos, direction, 50.0f);
				mComponents.push_back(projectile);
				if (projectile->GetModel() != nullptr)
				{
					mComponents.push_back(projectile->GetModel());
				}
				mProjectiles.push_back(projectile);
				OutputDebugString(L"Projectile created!\n");

				PlaySound(L"Content\\Audio\\gun-gunshot-02.wav", NULL, SND_FILENAME | SND_ASYNC);

				mLastFireTime = 0.0f;
			}
		}

		// Update projectiles and check collisions
		for (int i = (int)mProjectiles.size() - 1; i >= 0; --i)
		{
			Projectile* projectile = mProjectiles[i];

			if (!projectile->IsAlive())
			{
				// Remove projectile from mComponents
				auto it = std::find(mComponents.begin(), mComponents.end(), projectile);
				if (it != mComponents.end()) mComponents.erase(it);

				// Remove projectile's model from mComponents and delete it
				ModelFromFile* model = projectile->GetModel();
				if (model != nullptr)
				{
					auto mit = std::find(mComponents.begin(), mComponents.end(), model);
					if (mit != mComponents.end()) mComponents.erase(mit);
					delete model;
				}

				DeleteObject(projectile);
				mProjectiles.erase(mProjectiles.begin() + i);
				continue;
			}

			// Check collision only with shootable models (different interaction from picking/walking)
			DirectX::BoundingSphere projSphere = projectile->GetBoundingSphere();
			for (ModelFromFile* model : mShootableModels)
			{
				if (!model->Visible()) continue;

				if (projSphere.Intersects(model->mWorldBox))
				{
					model->SetVisible(false);
					projectile->SetAlive(false);
					mScore += model->ModelValue();
					ShowPickupMessage(model->ModelValue());
					break;
				}
			}
		}

		BoundingSphere cameraSphere;
		XMStoreFloat3(&cameraSphere.Center, mCamera->PositionVector());
		cameraSphere.Radius = 0.8f;  // smaller pickup radius so player can navigate past objects

		DetectingCollsion_AllCollidables(cameraSphere);

		// Picking disabled - bench/star/tree are collected only via walk-into collision.
		// Right-click is reserved for the shooting interaction (targets only the car).
		Game::toPick = false;

		if (mScore < 0)
		{
			BeginGameOver();
			return;
		}

	}
	bool RenderingGame::CheckCollision(ModelFromFile* a, ModelFromFile* b)
	{
		// Must use mWorldBox for objects that have been moved!
		return a->mWorldBox.Intersects(b->mWorldBox);
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

		if (mGameState == GameState::Menu)
		{
			mMenu->Draw();
		}
		else if (mGameState == GameState::Playing)
		{
			Game::Draw(gameTime);
			mRenderStateHelper->SaveAll();
			mFpsComponent->Draw(gameTime);

			mSpriteBatch->Begin();
			std::wostringstream scoreLabel;
			scoreLabel << L"Your current score: " << mScore << "\n";
			if (mWaveState == WaveState::GameWon)
			{
				scoreLabel << L"YOU WIN! Infinite Mode unlocked\n";
			}
			else if (mInfiniteMode)
			{
				scoreLabel << L"Infinite Mode\n";
				if (mWaveState == WaveState::WaitingToStart)
				{
					int seconds = (int)mWaveStateTimer + 1;
					scoreLabel << L"Next round starts in " << seconds << L"...\n";
				}
				else if (mWaveState == WaveState::Spawning)
				{
					int remaining = (int)(WAVE_MAX_DURATION - mWaveElapsed) + 1;
					if (remaining < 0) remaining = 0;
					scoreLabel << L"Time left: " << remaining << L"s\n";
				}
			}
			else
			{
				scoreLabel << L"Wave " << (mCurrentWave + 1) << L" / " << mWaveCounts.size() << L"\n";
				if (mWaveState == WaveState::WaitingToStart)
				{
					int seconds = (int)mWaveStateTimer + 1;
					scoreLabel << L"Wave starts in " << seconds << L"...\n";
				}
				else if (mWaveState == WaveState::Spawning)
				{
					int remaining = (int)(WAVE_MAX_DURATION - mWaveElapsed) + 1;
					if (remaining < 0) remaining = 0;
					scoreLabel << L"Time left: " << remaining << L"s\n";
				}
			}
			scoreLabel << L"Press ESC to Pause";
			mSpriteFont->DrawString(mSpriteBatch, scoreLabel.str().c_str(),
			XMFLOAT2(0.0f, 120.0f), Colors::Red);

			// Crosshair at center of screen
			float centerX = (float)Game::DefaultScreenWidth / 2.0f;
			float centerY = (float)Game::DefaultScreenHeight / 2.0f;
			mSpriteFont->DrawString(mSpriteBatch, L"+",
				XMFLOAT2(centerX - 6.0f, centerY - 10.0f), Colors::White);

			// Pickup popup near the crosshair
			if (mPickupMessageTimer > 0.0f && !mPickupMessage.empty())
			{
				mSpriteFont->DrawString(mSpriteBatch, mPickupMessage.c_str(),
					XMFLOAT2(centerX - 40.0f, centerY + 20.0f), Colors::Yellow);
			}

			mSpriteBatch->End();

			mRenderStateHelper->RestoreAll();
		}
		else if (mGameState == GameState::Paused)
		{
			mMenu->Draw();

			mRenderStateHelper->SaveAll();
			mSpriteBatch->Begin();
			mSpriteFont->DrawString(mSpriteBatch, L"PAUSED", XMFLOAT2(500.0f, 50.0f), Colors::Red);
			mSpriteBatch->End();
			mRenderStateHelper->RestoreAll();
		}
		else if (mGameState == GameState::GameOver)
		{
			mRenderStateHelper->SaveAll();
			mSpriteBatch->Begin();
			mSpriteFont->DrawString(mSpriteBatch, L"YOU LOST", XMFLOAT2(470.0f, 220.0f), Colors::Red);
			mSpriteFont->DrawString(mSpriteBatch, L"Returning to menu...", XMFLOAT2(410.0f, 270.0f), Colors::White);
			mSpriteBatch->End();
			mRenderStateHelper->RestoreAll();
		}

        HRESULT hr = mSwapChain->Present(0, 0);
        if (FAILED(hr))
        {
            throw GameException("IDXGISwapChain::Present() failed.", hr);
        }

    }
}
