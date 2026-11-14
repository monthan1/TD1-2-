#pragma once

#include <Novice.h>

namespace Controller {

    // Novice側のPadButton型を利用
    using Button = ::PadButton;

    void Initialize();
    void Update();

    bool IsConnected();

    bool IsDown(Button button);
    bool IsPressed(Button button);
    bool IsReleased(Button button);

    float GetLeftStickX();
    float GetLeftStickY();

    float GetRightStickX();
    float GetRightStickY();
}