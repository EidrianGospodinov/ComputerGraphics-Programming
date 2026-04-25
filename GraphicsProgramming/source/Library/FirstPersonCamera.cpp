#include "FirstPersonCamera.h"
#include "Game.h"
#include "GameTime.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "VectorHelper.h"

namespace Library
{
    RTTI_DEFINITIONS(FirstPersonCamera)

    const float FirstPersonCamera::DefaultRotationRate = XMConvertToRadians(1.0f);
    const float FirstPersonCamera::DefaultMovementRate = 10.0f;
    const float FirstPersonCamera::DefaultMouseSensitivity = 100.0f;

    FirstPersonCamera::FirstPersonCamera(Game& game)
        : Camera(game), mKeyboard(nullptr), mMouse(nullptr), 
          mMouseSensitivity(DefaultMouseSensitivity), mRotationRate(DefaultRotationRate), mMovementRate(DefaultMovementRate)
    {
    }

    FirstPersonCamera::FirstPersonCamera(Game& game, float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance)
        : Camera(game, fieldOfView, aspectRatio, nearPlaneDistance, farPlaneDistance), mKeyboard(nullptr), mMouse(nullptr),
          mMouseSensitivity(DefaultMouseSensitivity), mRotationRate(DefaultRotationRate), mMovementRate(DefaultMovementRate)
          
    {
    }

    FirstPersonCamera::~FirstPersonCamera()
    {
        mKeyboard = nullptr;
        mMouse = nullptr;
    }

    const Keyboard& FirstPersonCamera::GetKeyboard() const
    {
        return *mKeyboard;
    }

    void FirstPersonCamera::SetKeyboard(Keyboard& keyboard)
    {
        mKeyboard = &keyboard;
    }

    const Mouse& FirstPersonCamera::GetMouse() const
    {
        return *mMouse;
    }

    void FirstPersonCamera::SetMouse(Mouse& mouse)
    {
        mMouse = &mouse;
    }

    float&FirstPersonCamera:: MouseSensitivity()
    {
        return mMouseSensitivity;
    }


    float& FirstPersonCamera::RotationRate()
    {
        return mRotationRate;
    }

    float& FirstPersonCamera::MovementRate()
    {
        return mMovementRate;
    }

    void FirstPersonCamera::Initialize()
    {
        mKeyboard = (Keyboard*)mGame->Services().GetService(Keyboard::TypeIdClass());
        mMouse = (Mouse*)mGame->Services().GetService(Mouse::TypeIdClass());

        Camera::Initialize();
    }

    void FirstPersonCamera::Input(XMFLOAT3& movementAmount) const
    {
        if (mKeyboard != nullptr)
        {
            /*if (mKeyboard->IsKeyDown(DIK_W))
            {
                movementAmount.y = 1.0f;
            }*/

            /*if (mKeyboard->IsKeyDown(DIK_S))
            {
                movementAmount.y = -1.0f;
            }*/

            if (mKeyboard->IsKeyDown(DIK_A))
            {
                movementAmount.x = -1.0f;
            }

            if (mKeyboard->IsKeyDown(DIK_D))
            {
                movementAmount.x = 1.0f;
            }
            /*if (mKeyboard->IsKeyDown(DIK_Q)) {
                movementAmount.z = 1.0f;
            }
            if (mKeyboard->IsKeyDown(DIK_E)) {
                movementAmount.z = -1.0f;
            }*/
        }
    }

    void FirstPersonCamera::ApplyRotation(const GameTime& gameTime, float& elapsedTime)
    {
        XMFLOAT2 rotationAmount = Vector2Helper::Zero;
        if ((mMouse != nullptr) && (mMouse->IsButtonHeldDown(MouseButtonsLeft)))
        {
            LPDIMOUSESTATE mouseState = mMouse->CurrentState();
            rotationAmount.x = -mouseState->lX * mMouseSensitivity;
            rotationAmount.y = -mouseState->lY * mMouseSensitivity;
        }

        elapsedTime = (float)gameTime.ElapsedGameTime();
        XMVECTOR rotationVector = XMLoadFloat2(&rotationAmount) * mRotationRate * elapsedTime;
        XMVECTOR right = XMLoadFloat3(&mRight);

        XMMATRIX pitchMatrix = XMMatrixRotationAxis(right, XMVectorGetY(rotationVector));
        XMMATRIX yawMatrix = XMMatrixRotationY(XMVectorGetX(rotationVector));

        Camera::ApplyRotation(XMMatrixMultiply(pitchMatrix, yawMatrix));
    }

    void FirstPersonCamera::ApplyMovement(XMFLOAT3 movementAmount, float elapsedTime)
    {
        XMVECTOR position = XMLoadFloat3(&mPosition);
        XMVECTOR movement = XMLoadFloat3(&movementAmount) * mMovementRate * elapsedTime;

        //XMVECTOR strafe = right * XMVectorGetX(movement);
        XMVECTOR worldRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f); 
        XMVECTOR strafe = worldRight * XMVectorGetX(movement);
        position += strafe;

        XMVECTOR forward = XMLoadFloat3(&mDirection) * XMVectorGetY(movement);
        position += forward;
        XMVECTOR upDown = XMLoadFloat3(&mUp) * XMVectorGetZ(movement);
        position += upDown;

        ///Stops the player from moving out of bounds
        if (mPosition.x < -2.2f && movementAmount.x == -1.0f)
        {
            return;
        }
        else if (mPosition.x > 2.2f && movementAmount.x == 1.0f)
        {
            return;
		}
        XMStoreFloat3(&mPosition, position);
    }

    void FirstPersonCamera::Update(const GameTime& gameTime)
    {
        XMFLOAT3 movementAmount = Vector3Helper::Zero;
        Input(movementAmount);

        float elapsedTime;
        ApplyRotation(gameTime,elapsedTime);

        ApplyMovement(movementAmount, elapsedTime);

        Camera::Update(gameTime);
    }

    
}
