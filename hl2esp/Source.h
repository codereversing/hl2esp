#pragma once

#include <string>

#include "VectorWrapper.h"

struct IDirect3DDevice9;
struct ID3DXFont;

bool IsEntityEnemy(const std::string& entityName);
bool FrustomTransform(const VMatrix& worldToSurface, const Vector3& point, Vector2& screen);
bool WorldToScreen(const Vector3& position, const Vector2& screenPosition);

ID3DXFont* GetFont(IDirect3DDevice9* device);
void DrawTextGDI(const Vector2& screenPosition, const std::string text);
void DrawTextD3D9(const Vector2& screenPosition, const std::string text, IDirect3DDevice9* device);

void DrawEnemyEntityText(IDirect3DDevice9* device);