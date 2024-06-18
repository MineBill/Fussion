#include "e5pch.h"
#include "Camera.h"

#include "Log/Log.h"

Fussion::Camera::Camera()
{
    LOG_DEBUGF("Camera created!!! {}", FieldOfView);
}

Fussion::Camera::~Camera()
{
    LOG_DEBUG("Destroying camera!");
}

void Fussion::Camera::OnCreate()
{
    LOG_DEBUG("Camera::OnCreate!");
}