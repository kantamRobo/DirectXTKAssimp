#pragma once


// GameObjectManager.h
#pragma once
#include "GameObject.h"
#include <memory>

class GameObjectManager
{
public:
    void Add(std::shared_ptr<GameObject> obj)
    {
        objects.push_back(obj);
    }

    void UpdateAll()
    {
        for (auto& obj : objects)
            obj->Update();
    }

    void DrawAll(ID3D11DeviceContext* context, DirectX::XMMATRIX viewProj)
    {
        for (auto& obj : objects)
            obj->Draw(context, viewProj);
    }

private:
    std::vector<std::shared_ptr<GameObject>> objects;
};
