#ifndef DRAWING_H
#define DRAWING_H

#include "pch.h"

class Drawing
{
public:
	static bool bDisplay;
	static BOOL bInit;
	static bool bSetPos;

	static HRESULT APIENTRY hkPresent(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flag);

private:
	static ImVec2 vWindowPos;
	static ImVec2 vWindowSize;

	static void InitImGui(IDXGISwapChain* pSwapChain);
};

#endif