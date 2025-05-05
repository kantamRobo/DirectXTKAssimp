// HostD3D11.h
#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <winsock2.h>

// swap chain とレンダリングで共通に使う定義
static const UINT  TEX_W = 800;
static const UINT  TEX_H = 600;
static const USHORT PORT = 12345;
