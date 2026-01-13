#pragma once
#include "Window.h"
#include "ChiliTimer.h"
#include "ImguiManager.h"
#include "CameraContainer.h"
#include "PointLight.h"
#include "TestCube.h"
#include "Model.h"
#include "ScriptCommander.h"
#include "BlurOutlineRenderGraph.h"
#include "ChiliMath.h"

class App
{
public:
	App( const std::string& commandLine = "" );
	// master frame / message loop
	int Go();
	~App();
private:
	void DoFrame( float dt );
	void HandleInput( float dt );
	void ShowImguiDemoWindow();


#pragma region 漂浮Cube结构体
    struct FloatingCube
    {
        std::unique_ptr<TestCube> pCube; // 真正的绘制对象
        DirectX::XMFLOAT3 pos;           // 位置
        DirectX::XMFLOAT3 vel;           // 速度 (Velocity)
        DirectX::XMFLOAT3 rot;           // 旋转角度
        DirectX::XMFLOAT3 dRot;          // 旋转速度 (Delta Rotation)

        // 构造函数：初始化位置和随机速度
        FloatingCube(Graphics& gfx, std::mt19937& rng)
            :
            pCube(std::make_unique<TestCube>(gfx, 4.0f)) // 大小为 4.0
        {
            // 随机分布范围
            std::uniform_real_distribution<float> posDist(-40.0f, 40.0f); // 位置范围大一点
            std::uniform_real_distribution<float> velDist(-20.0f, 20.0f); // 移动速度
            std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * PI);
            std::uniform_real_distribution<float> dRotDist(-2.0f, 2.0f);  // 旋转速度

            pos = { posDist(rng), posDist(rng), posDist(rng) + 40.0f }; // +40 让它们在相机前面
            vel = { velDist(rng), velDist(rng), velDist(rng) };
            rot = { angleDist(rng), angleDist(rng), angleDist(rng) };
            dRot = { dRotDist(rng), dRotDist(rng), dRotDist(rng) };

            // 初始化位置
            pCube->SetPos(pos);
            pCube->SetRotation(rot.x, rot.y, rot.z);
        }

        // === 关键：手动实现的 Update 函数 ===
        void Update(float dt)
        {
            // 1. 根据速度更新位置
            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            pos.z += vel.z * dt;

            // 2. 根据旋转速度更新角度
            rot.x += dRot.x * dt;
            rot.y += dRot.y * dt;
            rot.z += dRot.z * dt;

            // 3. 简单的边界反弹（防止飞太远看不见）
            const float boundary = 60.0f;
            if (std::abs(pos.x) > boundary) vel.x = -vel.x;
            if (std::abs(pos.y) > boundary) vel.y = -vel.y;
            if (std::abs(pos.z) > boundary) vel.z = -vel.z;

            // 4. 将计算好的新位置应用到 TestCube
            pCube->SetPos(pos);
            pCube->SetRotation(rot.x, rot.y, rot.z);
        }
    };
#pragma endregion


private:
	std::string commandLine;
	bool showDemoWindow = false;
	ImguiManager imgui;
	Window wnd;
	ScriptCommander scriptCommander;
	Rgph::BlurOutlineRenderGraph rg{ wnd.Gfx() };
	ChiliTimer timer;
	float speed_factor = 1.0f;
	CameraContainer cameras;
	PointLight light;
	TestCube cube{ wnd.Gfx(),4.0f };
	TestCube cube2{ wnd.Gfx(),4.0f };
	Model sponza{ wnd.Gfx(),"Models\\sponza\\sponza.obj",1.0f / 20.0f };
	Model gobber{ wnd.Gfx(),"Models\\gobber\\GoblinX.obj",4.0f };
	Model nano{ wnd.Gfx(),"Models\\nano_textured\\nanosuit.obj",2.0f };
	bool savingDepth = false;


    bool isImgui = true;
#pragma region 漂浮Cube容器
	// 替换原有的单个cube，改为容器
    std::vector<FloatingCube> cubes;
#pragma endregion


	float time = 0.0f;
};