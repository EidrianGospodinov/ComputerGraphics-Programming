#pragma once


#include "FpsComponent.h"
#include "Game.h"
#include "ModelFromFile.h"

using namespace Library;

namespace Library
{
    class FirstPersonCamera;
	class RenderStateHelper;
    class Keyboard;
    class Mouse;
    class FpsCounter;
}

namespace Rendering
{
    class TriangleDemo;

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

    private:
		static const XMFLOAT4 BackgroundColor;
        FirstPersonCamera * mCamera;
        TriangleDemo* mDemo;
        LPDIRECTINPUT8 mDirectInput;
        Keyboard* mKeyboard;
        Mouse* mMouse;

        ModelFromFile* mModel;

        FpsComponent* mFpsComponent;
        RenderStateHelper* mRenderStateHelper;
    };
}