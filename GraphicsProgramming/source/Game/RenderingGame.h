#pragma once

#include <DirectXCollision.h>
#include "Game.h"

enum class GameState
{
	Menu,
	Playing,
	Paused
};

enum class WaveState
{
	WaitingToStart,
	Spawning,
	GameWon
};

using namespace Library;

namespace Library
{
    class FirstPersonCamera;
	class RenderStateHelper;

	class Keyboard;
	class Mouse;
	class FpsComponent;

}
namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}


namespace Rendering
{
    class TriangleDemo;
	class ModelFromFile;
	class ObjectDiffuseLight;
	class MenuComponent;
	class Projectile;

    class RenderingGame : public Game
    {
    public:
        RenderingGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand);
        ~RenderingGame();

        virtual void Initialize() override;		
        virtual void Update(const GameTime& gameTime) override;
        virtual void Draw(const GameTime& gameTime) override;


	protected:
        virtual void Shutdown() override;
        void LookForCollsion_Manual(BoundingSphere cameraSphere);
        void DetectingCollsion_AllCollidables(BoundingSphere cameraSphere);

    private:
		static const XMFLOAT4 BackgroundColor;
        FirstPersonCamera * mCamera;
        TriangleDemo* mDemo;

		//Define member variables for Keyboard and mouse
		LPDIRECTINPUT8 mDirectInput;

		Keyboard* mKeyboard;
		Mouse*    mMouse;
		ModelFromFile* mModel1;
    	std::vector<ModelFromFile*> mCars;
	

		FpsComponent* mFpsComponent;
		RenderStateHelper* mRenderStateHelper;

		ObjectDiffuseLight* mObjectDiffuseLight;

		MenuComponent* mMenu;
		GameState mGameState;

    	int mScore;
    	SpriteBatch* mSpriteBatch;
    	SpriteFont* mSpriteFont;

    	std::vector<ModelFromFile*> mCollidableModels;
    	std::vector<ModelFromFile*> mShootableModels;
    	void CheckAllCollisions(XMVECTOR oldPos);

		void Pick(int sx, int sy, ModelFromFile*);
        static bool CheckCollision(ModelFromFile* a, ModelFromFile* b);

		std::vector<Projectile*> mProjectiles;
		float mLastFireTime;
		const float FIRE_COOLDOWN = 0.5f;

		// Pickup popup HUD
		std::wstring mPickupMessage;
		float mPickupMessageTimer;
		const float PICKUP_MESSAGE_DURATION = 1.5f;
		void ShowPickupMessage(int points);

		// Wave system
		std::vector<int> mWaveCounts;       // objects to spawn per wave
		std::vector<XMFLOAT3> mSpawnPoints; // pool of positions
		int mCurrentWave;
		WaveState mWaveState;
		float mWaveStateTimer;
		const float WAVE_START_DELAY = 1.5f;
		const float WAVE_MAX_DURATION = 30.0f;
		const float WORLD_BOUNDS = 30.0f;
		float mWaveElapsed;
		void UpdateWaves(const GameTime& gameTime);
		void StartWave(int waveIndex);
		void RestartGame();
		void EndGame();
    };
}
