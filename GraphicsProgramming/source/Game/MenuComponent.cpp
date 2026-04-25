#include "MenuComponent.h"
#include "Game.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>

namespace Rendering
{
	MenuComponent::MenuComponent(Game& game, SpriteBatch* spriteBatch, SpriteFont* spriteFont, Keyboard* keyboard)
		: GameComponent(game), mSpriteBatch(spriteBatch), mSpriteFont(spriteFont), mKeyboard(keyboard),
		  mSelectedOption(0), mConfirmed(false), mWasUpPressed(false), mWasDownPressed(false),
		  mMenuMode(MenuMode::MainMenu), mCurrentOptions(nullptr)
	{
		mCurrentOptions = mMainMenuOptions;
	}

	MenuComponent::~MenuComponent()
	{
	}

	void MenuComponent::Update(const GameTime& gameTime)
	{
		if (mKeyboard == nullptr) return;

		// Handle UP arrow key
		if (mKeyboard->IsKeyDown(DIK_UP))
		{
			if (!mWasUpPressed)
			{
				mSelectedOption = (mSelectedOption - 1 + OPTION_COUNT) % OPTION_COUNT;
				mWasUpPressed = true;
			}
		}
		else
		{
			mWasUpPressed = false;
		}

		// Handle DOWN arrow key
		if (mKeyboard->IsKeyDown(DIK_DOWN))
		{
			if (!mWasDownPressed)
			{
				mSelectedOption = (mSelectedOption + 1) % OPTION_COUNT;
				mWasDownPressed = true;
			}
		}
		else
		{
			mWasDownPressed = false;
		}

		// Handle ENTER key
		if (mKeyboard->WasKeyPressedThisFrame(DIK_RETURN))
		{
			mConfirmed = true;
		}
	}

	void MenuComponent::Draw()
	{
		if (mSpriteBatch == nullptr || mSpriteFont == nullptr) return;

		mSpriteBatch->Begin();

		// Draw title
		XMFLOAT2 titlePosition(200.0f, 100.0f);
		mSpriteFont->DrawString(mSpriteBatch, L"GRAPHICS GAME", titlePosition, Colors::White);

		// Draw menu options
		XMFLOAT2 optionPosition(250.0f, 250.0f);
		for (int i = 0; i < OPTION_COUNT; ++i)
		{
			XMVECTORF32 color = (i == mSelectedOption) ? Colors::Yellow : Colors::White;
			mSpriteFont->DrawString(mSpriteBatch, mCurrentOptions[i], optionPosition, color);
			optionPosition.y += 60.0f;
		}

		// Draw instructions
		XMFLOAT2 instructionPosition(150.0f, 450.0f);
		mSpriteFont->DrawString(mSpriteBatch, L"Arrow Keys to select, ENTER to confirm", instructionPosition, Colors::Gray);

		mSpriteBatch->End();
	}

	void MenuComponent::SetMenuMode(MenuMode mode)
	{
		mMenuMode = mode;
		mSelectedOption = 0;
		if (mode == MenuMode::MainMenu)
		{
			mCurrentOptions = mMainMenuOptions;
		}
		else
		{
			mCurrentOptions = mPauseMenuOptions;
		}
	}

	MenuMode MenuComponent::GetMenuMode() const
	{
		return mMenuMode;
	}
}
