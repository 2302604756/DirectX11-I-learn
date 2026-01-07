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
	const float t = time;
	std::ostringstream oss;
	oss << "Timer:" << t << "s";
	wnd.SetTitle(oss.str());
    const float r = 0.5f + 0.5f * sin(t);
    const float g = 0.5f + 0.5f * sin(t + 2.0f);
    const float b = 0.5f + 0.5f * sin(t + 4.0f);

    wnd.Gfx().BeginFrame(r, g, b);
	wnd.Gfx().Test();
    wnd.Gfx().EndFrame();
	//light.Bind( wnd.Gfx(),cameras->GetMatrix() );
	//rg.BindMainCamera( cameras.GetActiveCamera() );
	//	
	//light.Submit( Chan::main );
	//cube.Submit( Chan::main );
	//sponza.Submit( Chan::main );
	//cube2.Submit( Chan::main );
	//gobber.Submit( Chan::main );
	//nano.Submit( Chan::main );
	//cameras.Submit( Chan::main );

	//sponza.Submit( Chan::shadow );
	//cube.Submit( Chan::shadow );
	//sponza.Submit( Chan::shadow );
	//cube2.Submit( Chan::shadow );
	//gobber.Submit( Chan::shadow );
	//nano.Submit( Chan::shadow );

	//rg.Execute( wnd.Gfx() );

	//if( savingDepth )
	//{
	//	rg.DumpShadowMap( wnd.Gfx(),"shadow.png" );
	//	savingDepth = false;
	//}
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

	//// present
	wnd.Gfx().EndFrame();
	//rg.Reset();
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