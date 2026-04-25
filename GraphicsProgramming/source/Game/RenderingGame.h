#pragma once

#include <DirectXCollision.h>
#include "Game.h"

enum class GameState
{
	Menu,
	Playing,
	Paused
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
    	ModelFromFile* mModel2;
    	ModelFromFile* mModel4;
	

		FpsComponent* mFpsComponent;
		RenderStateHelper* mRenderStateHelper;

		ObjectDiffuseLight* mObjectDiffuseLight;

		MenuComponent* mMenu;
		GameState mGameState;

    	int mScore;
    	SpriteBatch* mSpriteBatch;
    	SpriteFont* mSpriteFont;

    	std::vector<ModelFromFile*> mCollidableModels;
    	void CheckAllCollisions(XMVECTOR oldPos);

		void Pick(int sx, int sy, ModelFromFile*);
        static bool CheckCollision(ModelFromFile* a, ModelFromFile* b);

		std::vector<Projectile*> mProjectiles;
		float mLastFireTime;
		const float FIRE_COOLDOWN = 0.5f;
    };
}