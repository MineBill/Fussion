﻿#pragma once
#include "Fussion/Core/Layer.h"

class ProjectCreatorLayer final : public Fussion::Layer {
public:
    virtual void OnStart() override;
    virtual void OnUpdate(f32 delta) override;

private:
    Vector2 m_MouseDragStarPos{}, m_Offset{};
    bool m_StartedDragging{};

    bool m_OpenNewProjectPopup{};
};