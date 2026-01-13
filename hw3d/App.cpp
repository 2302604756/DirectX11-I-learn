#include "App.h"
#include <algorithm>
#include "ChiliMath.h"
#include "imgui/imgui.h"
#include "ChiliUtil.h"
#include "Testing.h"
#include "PerfLog.h"
#include "TestModelProbe.h"
#include "Testing.h"
#include "Camera.h"
#include "Channels.h"
#include "TestCube.h"


#include <sstream>

namespace dx = DirectX;


App::App( const std::string& commandLine )
	:
	commandLine( commandLine ),
	wnd( 1280,720,"The Donkey Fart Box" ),
	scriptCommander( TokenizeQuoted( commandLine ) ),
	light( wnd.Gfx(),{ 10.0f,5.0f,0.0f } )
	
{
	

	cameras.AddCamera( std::make_unique<Camera>( wnd.Gfx(),"A",dx::XMFLOAT3{ -13.5f,6.0f,3.5f },0.0f,PI / 2.0f ) );
	cameras.AddCamera( std::make_unique<Camera>( wnd.Gfx(),"B",dx::XMFLOAT3{ -13.5f,28.8f,-6.4f },PI / 180.0f * 13.0f,PI / 180.0f * 61.0f ) );
	cameras.AddCamera( light.ShareCamera() );


#pragma region 50个cube生成
	std::mt19937 rng(std::random_device{}());
	// 生成 80 个方块
	for (int i = 0; i < 80; i++)
	{
		// 构造 FloatingCube，它会自动随机位置
		cubes.emplace_back(wnd.Gfx(), rng);

		// !!! 关键修复：必须链接到渲染图 !!!
		// FloatingCube.pCube 是 unique_ptr<TestCube>
		cubes.back().pCube->LinkTechniques(rg);
	}
#pragma endregion

	

	cube.SetPos( { 10.0f,5.0f,6.0f } );
	cube2.SetPos( { 10.0f,5.0f,14.0f } );
	nano.SetRootTransform(
		dx::XMMatrixRotationY( PI / 2.f ) *
		dx::XMMatrixTranslation( 27.f,-0.56f,1.7f )
	);
	gobber.SetRootTransform(
		dx::XMMatrixRotationY( -PI / 2.f ) *
		dx::XMMatrixTranslation( -8.f,10.f,0.f )
	);
	
	cube.LinkTechniques( rg );
	cube2.LinkTechniques( rg );
	light.LinkTechniques( rg );
	sponza.LinkTechniques( rg );
	gobber.LinkTechniques( rg );
	nano.LinkTechniques( rg );
	cameras.LinkTechniques( rg );
	
	rg.BindShadowCamera( *light.ShareCamera() );
}

void App::HandleInput( float dt )
{
	while( const auto e = wnd.kbd.ReadKey() )
	{
		if( !e->IsPress() )
		{
			continue;
		}

		switch( e->GetCode() )
		{
		case VK_ESCAPE:
			if( wnd.CursorEnabled() )
			{
				wnd.DisableCursor();
				wnd.mouse.EnableRaw();
			}
			else
			{
				wnd.EnableCursor();
				wnd.mouse.DisableRaw();
			}
			break;
		case VK_F1:
			showDemoWindow = true;
			break;
		case VK_RETURN:
			savingDepth = true;
			break;
		}
	}

	if( !wnd.CursorEnabled() )
	{
		if( wnd.kbd.KeyIsPressed( 'W' ) )
		{
			cameras->Translate( { 0.0f,0.0f,dt } );
		}
		if( wnd.kbd.KeyIsPressed( 'A' ) )
		{
			cameras->Translate( { -dt,0.0f,0.0f } );
		}
		if( wnd.kbd.KeyIsPressed( 'S' ) )
		{
			cameras->Translate( { 0.0f,0.0f,-dt } );
		}
		if( wnd.kbd.KeyIsPressed( 'D' ) )
		{
			cameras->Translate( { dt,0.0f,0.0f } );
		}
		if( wnd.kbd.KeyIsPressed( 'R' ) )
		{
			cameras->Translate( { 0.0f,dt,0.0f } );
		}
		if( wnd.kbd.KeyIsPressed( 'F' ) )
		{
			cameras->Translate( { 0.0f,-dt,0.0f } );
		}
	}

	while( const auto delta = wnd.mouse.ReadRawDelta() )
	{
		if( !wnd.CursorEnabled() )
		{
			cameras->Rotate( (float)delta->x,(float)delta->y );
		}
	}
}

//DoFrame用于更新每一帧画面
//dt是时间间隔
//如果把这一段代码置空 就只会生成一个空的Win32窗口
void App::DoFrame( float dt )
{
	const float t =   speed_factor;
	std::ostringstream oss;
	oss << "Timer:" << t << "s";
	wnd.SetTitle(oss.str());
    const float r = 0.5f + 0.5f * sin(t);
    const float g = 0.5f + 0.5f * sin(t + 2.0f);
    const float b = 0.5f + 0.5f * sin(t + 4.0f);
	
	static char buffer[1024];
	//imgui需要每一帧都刷新 所以要在DoFrame中调用
	//chili在window中加入了如果检测到是ui系统在得到输入 那么系统将不再读取输入 防止窗口命令和ui命令冲突
    wnd.Gfx().BeginFrame(0.07f, 0.0f, 0.12f);
	if (ImGui::Begin("Simluation Speed")) {
		ImGui::SliderFloat("Speed Factor", &speed_factor, 0.0f, 4.0f);
		ImGui::Text("Application average %.3f ms/frame(%.1f FPS)", 1000.0f/ImGui::GetIO().Framerate,ImGui::GetIO().Framerate);
		ImGui::InputText("Butts", buffer, sizeof(buffer));
		wnd.SetTitle(buffer);
	}
	ImGui::End();

    wnd.Gfx().CubeTest(t);
	
#pragma region 绘制多个三角形漂浮
	//wnd.Gfx().ConstantBufferTest(time);
	//light.Bind(wnd.Gfx(), cameras->GetMatrix());
	//rg.BindMainCamera(cameras.GetActiveCamera());
	//light.Submit(Chan::main);
	//// 确保 speed_factor 不是 0
	//const float loop_dt = dt * speed_factor;

	//// 更新所有方块
	// // === 2. 更新并提交方块 ===
	//for (auto& fc : cubes)
	//{
	//	// 调用我们刚才在 struct 里写的 Update
	//	// 传入 dt (时间增量) 和 speed_factor (全局速度系数)
	//	fc.Update(dt * speed_factor);

	//	// 提交渲染
	//	fc.pCube->Submit(Chan::main);
	//}
	//rg.Execute(wnd.Gfx());
#pragma endregion

#pragma region 教程最终渲染
	//light.Bind( wnd.Gfx(),cameras->GetMatrix() );
	//rg.BindMainCamera( cameras.GetActiveCamera() );
	//	
	//light.Submit( Chan::main );
	//cube.Submit( Chan::main );
	//sponza.Submit(Chan::main);
	/*cube2.Submit(Chan::main);
	gobber.Submit( Chan::main );
	nano.Submit( Chan::main );*/
	//cameras.Submit( Chan::main );

	//sponza.Submit( Chan::shadow );
	//cube.Submit( Chan::shadow );
	/*sponza.Submit(Chan::shadow);
	cube2.Submit( Chan::shadow );
	gobber.Submit( Chan::shadow );
	nano.Submit( Chan::shadow );*/

	//rg.Execute( wnd.Gfx() );

	/*if( savingDepth )
	{
		rg.DumpShadowMap( wnd.Gfx(),"shadow.png" );
		savingDepth = false;
	}*/
	//
	//// imgui windows
	//static MP sponzeProbe{ "Sponza" };
	//static MP gobberProbe{ "Gobber" };
	//static MP nanoProbe{ "Nano" };
	//sponzeProbe.SpawnWindow( sponza );
	//gobberProbe.SpawnWindow( gobber );
	//nanoProbe.SpawnWindow( nano );
	//cameras.SpawnWindow( wnd.Gfx() );
	//light.SpawnControlWindow();
	//ShowImguiDemoWindow();
	//cube.SpawnControlWindow( wnd.Gfx(),"Cube 1" );
	//cube2.SpawnControlWindow( wnd.Gfx(),"Cube 2" );
	//
	//rg.RenderWindows( wnd.Gfx() );
#pragma endregion

	
	wnd.Gfx().EndFrame();
	//rg.Reset();

	
	

	


	//// present
	
}

void App::ShowImguiDemoWindow()
{
	if( showDemoWindow )
	{
		ImGui::ShowDemoWindow( &showDemoWindow );
	}
}

App::~App()
{}

int App::Go()
{

	while( true )
	{
		// process all messages pending, but to not block for new messages
		//当这里是WM_QUIT消息时 我们返回ecode 否则继续下面的程序 调用DoFrame等
		if( const auto ecode = Window::ProcessMessages() )
		{
			// if return optional has value, means we're quitting so return exit code
			return *ecode;
		}
		// execute the game logic
		const auto dt = timer.Mark() * speed_factor;
		time += dt;
		HandleInput( dt );
		DoFrame( dt );
	}
}