#ifndef HOOK_H
#define HOOK_H

#include "pch.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
using tPresent = HRESULT(APIENTRY*)(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flag);
using tResizeBuffers = HRESULT(APIENTRY*)(IDXGISwapChain* pThis, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
using tSetFullscreenState = HRESULT(APIENTRY*)(IDXGISwapChain* pThis, BOOL FullScreen, IDXGIOutput* pTarget);

class Hook
{
public:
	static IDXGISwapChain* pSwapChain;
	static ID3D11Device* pDevice;
	static ID3D11DeviceContext* pDeviceContext;
	static tPresent oPresent;
	static HWND window;
	static HMODULE hDDLModule;

	static void HookDirectX();
	static void UnHookDirectX();
	static void HookWindow();
	static void UnHookWindow();

private:
	static int windowHeight, windowWidth;
	static void* d3d11SwapChain[40];
	static WNDPROC OWndProc;
	static tResizeBuffers oResizeBuffers;
	static tSetFullscreenState oSetFullscreenState;

	static BOOL CALLBACK enumWind(HWND handle, LPARAM lp);
	static HWND GetProcessWindow();
	static BOOL GetD3D11SwapChain(void** pTable, size_t size);
	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static HRESULT APIENTRY hkResizeBuffers(IDXGISwapChain* pThis, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
	static HRESULT APIENTRY hkSetFullscreenState(IDXGISwapChain* pThis, BOOL FullScreen, IDXGIOutput* pTarget);
};

#endif