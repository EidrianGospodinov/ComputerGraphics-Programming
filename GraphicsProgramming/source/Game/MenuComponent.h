#pragma once

#include "GameComponent.h"
#include "Keyboard.h"
#include <DirectXMath.h>

using namespace Library;
using namespace DirectX;

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{

	enum MenuMode
	{
		MainMenu,
		PauseMenu
	};

	class MenuComponent : public GameComponent
	{
	public:
		MenuComponent(Game& game, SpriteBatch* spriteBatch, SpriteFont* spriteFont, Keyboard* keyboard);
		~MenuComponent();

		virtual void Update(const GameTime& gameTime) override;
		void Draw();

		int GetSelectedOption() const { return mSelectedOption; }
		bool IsConfirmed() const { return mConfirmed; }
		void ResetConfirmed() { mConfirmed = false; }
		void SetMenuMode(MenuMode mode);
		MenuMode GetMenuMode() const;

	private:
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		Keyboard* mKeyboard;

		int mSelectedOption;
		bool mConfirmed;
		bool mWasUpPressed;
		bool mWasDownPressed;
		MenuMode mMenuMode;

		static const int OPTION_COUNT = 2;
		const wchar_t* mMainMenuOptions[OPTION_COUNT] = { L"> START", L"  QUIT" };
		const wchar_t* mPauseMenuOptions[OPTION_COUNT] = { L"> CONTINUE", L"  QUIT" };
		const wchar_t** mCurrentOptions;
	};
}
