#include "Hook.h"
#include "Drawing.h"

IDXGISwapChain* Hook::pSwapChain = nullptr; // D3D11 Swap Chain
ID3D11Device* Hook::pDevice = nullptr; // D3D11 device.
ID3D11DeviceContext* Hook::pDeviceContext = nullptr; // D3D11 device context.
tPresent Hook::oPresent = nullptr; // Pointer of the original Present function
tResizeBuffers Hook::oResizeBuffers = nullptr; // Pointer of the original ResizeBuffer function
tSetFullscreenState Hook::oSetFullscreenState = nullptr;
HWND Hook::window = nullptr; // Window of the current process
HMODULE Hook::hDDLModule = nullptr; // HMODULE of the DLL

int Hook::windowHeight = 0; // Height of the window
int Hook::windowWidth = 0; // Width of the window
void* Hook::d3d11SwapChain[40]; // Array of pointer of the DirectX functions.
WNDPROC Hook::OWndProc = nullptr; // Pointer of the original window message handler.

/**
	@brief : Function that hook the ResizeBuffers and Present function.
**/
void Hook::HookDirectX()
{
	if (GetD3D11SwapChain(d3d11SwapChain, sizeof(d3d11SwapChain)))
	{
		oPresent = (tPresent)d3d11SwapChain[8];
		oResizeBuffers = (tResizeBuffers)d3d11SwapChain[13];
		oSetFullscreenState = (tSetFullscreenState)d3d11SwapChain[10];
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)oPresent, Drawing::hkPresent);
		DetourAttach(&(PVOID&)oResizeBuffers, hkResizeBuffers);
		DetourAttach(&(PVOID&)oSetFullscreenState, hkSetFullscreenState);
		DetourTransactionCommit();
	}
}

/**
	@brief : Function that unhook ResizeBuffers and Present functions, and also disable the window message handler.
**/
void Hook::UnHookDirectX()
{
	if (Drawing::bInit)
	{
		UnHookWindow();
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		pDeviceContext->Release();
		pDevice->Release();
	}

	Drawing::bInit = FALSE;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)oPresent, Drawing::hkPresent);
	DetourDetach(&(PVOID&)oResizeBuffers, hkResizeBuffers);
	DetourDetach(&(PVOID&)oSetFullscreenState, hkSetFullscreenState);
	DetourTransactionCommit();
}

/**
	@brief : Function that enumerate windows.
**/
BOOL CALLBACK Hook::enumWind(const HWND handle, LPARAM lp)
{
	DWORD procID;
	GetWindowThreadProcessId(handle, &procID);
	if (GetCurrentProcessId() != procID)
		return TRUE;

	window = handle;
	return FALSE;
}

/**
	@brief : Function that retrieve the window of the current process.
	@retval : window of the current process.
**/
HWND Hook::GetProcessWindow()
{
	window = nullptr;

	EnumWindows(enumWind, NULL);

	RECT size;
	if (window == nullptr)
		return nullptr;

	GetWindowRect(window, &size);

	windowWidth = size.right - size.left;
	windowHeight = size.bottom - size.top;

	windowHeight -= 29;
	windowWidth -= 5;

	return window;
}

/**
	@brief : Function that get a D3D11 Swap Chain.
	@param  pTable : Array of functions pointer.
	@param  size   : Size of the array of pointers.
	@retval : True if the function succeed else False.
**/
BOOL Hook::GetD3D11SwapChain(void** pTable, const size_t size)
{
	if (!pTable)
		return FALSE;

	ID3D11Device* pd3dDevice = nullptr;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = GetProcessWindow();
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = (GetWindowLongPtr(sd.OutputWindow, GWL_STYLE) & WS_POPUP) != 0 ? false : true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &pSwapChain, &pd3dDevice, nullptr, nullptr);
	if (res == DXGI_ERROR_UNSUPPORTED)
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &pSwapChain, &pd3dDevice, nullptr, nullptr);
	if (res != S_OK)
		return false;

	memcpy(pTable, *(void***)(pSwapChain), size);

	pSwapChain->Release();
	pd3dDevice->Release();
	return TRUE;
}

/**
	@brief : Function that setup the WndProc callback function.
**/
void Hook::HookWindow()
{
	OWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
}

/**
	@brief : Function that disable the WndProc callback function.
**/
void Hook::UnHookWindow()
{
	SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)OWndProc);
}

/**
	@brief : A callback function, which you define in your application, that processes messages sent to a window. (https://learn.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc)
	@param  hWnd : A handle to the window.
	@param  msg : The message.
	@param  wParam : Additional message information.
	@param  lParam : Additional message information.
	@retval : The return value is the result of the message processing, and depends on the message sent.
**/
LRESULT WINAPI Hook::WndProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	if (Drawing::bDisplay && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		ImGui::GetIO().MouseDrawCursor = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
		return true;
	}

	if (Drawing::bInit)
		ImGui::GetIO().MouseDrawCursor = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);

	if (msg == WM_CLOSE)
	{
		UnHookDirectX();
		UnHookWindow();
		TerminateProcess(GetCurrentProcess(), 0);
	}

	if (ImGui::GetIO().WantCaptureMouse)
	{
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
			return true;
		return false;
	}

	return CallWindowProc(OWndProc, hWnd, msg, wParam, lParam);
}

/**
    @brief : Hook function of ResizeBuffers, the function release ImGui context and D3D11 objects when window resize.
			 (https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-resizebuffers)
**/
HRESULT Hook::hkResizeBuffers(IDXGISwapChain* pThis, const UINT BufferCount, const UINT Width, const UINT Height, const DXGI_FORMAT NewFormat, const UINT SwapChainFlags)
{
	Drawing::bSetPos = true;
	UnHookWindow();
	if (Drawing::bInit)
	{
		Drawing::bInit = FALSE;
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		pDeviceContext->Release();
		pDevice->Release();
	}
	pSwapChain = nullptr;
	pDevice = nullptr;
	pDeviceContext = nullptr;

	return oResizeBuffers(pThis, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

/**
    @brief : Hook function of SetFullscreenState, the function release ImGui context and D3D11 objects when window fullscreen state change.
			 (https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-setfullscreenstate)
**/
HRESULT Hook::hkSetFullscreenState(IDXGISwapChain* pThis, const BOOL FullScreen, IDXGIOutput* pTarget)
{
	Drawing::bSetPos = true;
	UnHookWindow();
	if (Drawing::bInit)
	{
		Drawing::bInit = FALSE;
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		pDeviceContext->Release();
		pDevice->Release();
	}
	pSwapChain = nullptr;
	pDevice = nullptr;
	pDeviceContext = nullptr;

	return oSetFullscreenState(pThis, FullScreen, pTarget);
}