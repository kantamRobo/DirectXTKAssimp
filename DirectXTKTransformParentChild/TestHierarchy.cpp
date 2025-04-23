// TestHierarchy.cpp
#include "GameObjectManager.h"
#include <iostream>

void TestTransformHierarchy()
{
    GameObjectManager manager;

    auto parent = std::make_shared<GameObject>();
    parent->transform.position = { 0, 0, 0 };

    auto child = std::make_shared<GameObject>();
    child->transform.position = { 1, 0, 0 };
    parent->transform.AddChild(&child->transform);

    auto grandchild = std::make_shared<GameObject>();
    grandchild->transform.position = { 0, 1, 0 };
    child->transform.AddChild(&grandchild->transform);

    manager.Add(parent);
    manager.Add(child);
    manager.Add(grandchild);

    manager.UpdateAll();

    auto grandWorld = grandchild->GetWorldMatrix();
    DirectX::XMFLOAT4X4 mat;
    DirectX::XMStoreFloat4x4(&mat, grandWorld);
    std::wcout << L"Grandchild World Position: (" << mat._41 << L"," << mat._42 << L"," << mat._43 << L")\n";
}

// ŒÄ‚Ño‚µ—áimain“™‚Åj
// TestTransformHierarchy();
