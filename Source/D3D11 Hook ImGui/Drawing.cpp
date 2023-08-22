#include "Drawing.h"
#include "Hook.h"

BOOL Drawing::bInit = FALSE; // Status of the initialization of ImGui.
bool Drawing::bDisplay = true; // Status of the menu display.
bool Drawing::bSetPos = false; // Status to update ImGui window size / position.
ImVec2 Drawing::vWindowPos = { 0, 0 }; // Last ImGui window position.
ImVec2 Drawing::vWindowSize = { 0, 0 }; // Last ImGui window size.

/**
    @brief : Hook function of Present, the function render ImGui menu.
			 (https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-present)
**/
HRESULT Drawing::hkPresent(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flag)
{
	if (!Hook::pSwapChain)
		Hook::pSwapChain = pThis;

	if (!bInit)
		InitImGui(pThis);

	if (GetAsyncKeyState(VK_INSERT) & 1)
		bDisplay = !bDisplay;

	if (GetAsyncKeyState(VK_END) & 1)
	{
		Hook::UnHookDirectX();
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, Hook::hDDLModule, 0, nullptr);
		return Hook::oPresent(pThis, SyncInterval, Flag);
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (bDisplay)
	{
		ImGui::Begin("Menu Window Title", &bDisplay);
		{
			ImGui::SetWindowSize({ 500, 300 }, ImGuiCond_Once);

			if (vWindowPos.x != 0.0f && vWindowPos.y != 0.0f && vWindowSize.x != 0.0f && vWindowSize.y != 0.0f && bSetPos)
			{
				ImGui::SetWindowPos(vWindowPos);
				ImGui::SetWindowSize(vWindowSize);
				bSetPos = false;
			}

			if (bSetPos == false)
			{
				vWindowPos = { ImGui::GetWindowPos().x, ImGui::GetWindowPos().y };
				vWindowSize = { ImGui::GetWindowSize().x, ImGui::GetWindowSize().y };
			}

			ImGui::Text("Draw your menu here.");
		}
		ImGui::End();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return Hook::oPresent(pThis, SyncInterval, Flag);
}

/**
    @brief : Funation that init ImGui and retrieve D3D11 device and device context.
    @param pSwapChain : Pointer to the SwapCahin from the Present hook.
**/
void Drawing::InitImGui(IDXGISwapChain* pSwapChain)
{
	Hook::HookWindow();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = nullptr;
	io.Fonts->AddFontDefault();

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(Hook::window);

	pSwapChain->GetDevice(__uuidof(Hook::pDevice), (void**)&Hook::pDevice);
	Hook::pDevice->GetImmediateContext(&Hook::pDeviceContext);

	ImGui_ImplDX11_Init(Hook::pDevice, Hook::pDeviceContext);

	bInit = TRUE;
}