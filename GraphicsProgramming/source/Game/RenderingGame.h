    #pragma once


#include "Game.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ModelFromFile.h"
#include "FpsComponent.h"
#include "RenderStateHelper.h"
using namespace Library;

namespace Library
{
    class FirstPersonCamera;
	class RenderStateHelper;
    class keyboard;
    class Mouse;
    class FpsComponent;
    
}

namespace Rendering
{
    class TriangleDemo;
    class ObjectDiffuseLight;
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
        Mouse* mMouse;
        Keyboard* mKeyboard;
        ModelFromFile* mModel;
        FpsComponent* mFpsComponent;
        RenderStateHelper* mRenderStateHelper;
        ObjectDiffuseLight* mObjectDiffuseLight;
    };
}