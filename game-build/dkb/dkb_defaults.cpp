// SPDX-License-Identifier: GPL-3.0-or-later
// Donkey Kong Bananza: factory keyboard and mouse bindings for play mode (the settings-file format of runtime/src/gfx/play/play_types.h).
// They map keys to the Switch pad; the game's own button roles are not mapped yet. Pattern: runtime/games/pkp/pkp_game.cpp.
#include "play_types.h"

namespace {

constexpr const char* kSettingsText = R"INI(
[meta]
defaults_rev = 1

[bindings.all]
W = move_forward
S = move_back
A = move_left
D = move_right
Up = look_up
Down = look_down
Left = look_left
Right = look_right
Return = btn_a
Keypad Enter = btn_a
Space = btn_a
E = btn_a
Backspace = btn_b
Left Shift = btn_b
Tab = btn_x
F = btn_y
Mouse1 = btn_zr
Mouse3 = btn_zl
Q = btn_l
R = btn_r
Left Ctrl = btn_lstick
V = btn_rstick
Escape = btn_plus
C = btn_minus
1 = dpad_up
2 = dpad_down
3 = dpad_left
4 = dpad_right

[bindings.menu]
WheelUp = dpad_up
WheelDown = dpad_down
)INI";

}  // namespace

const nxrt::play::GameDefaults* dkb_play_defaults() {
    static const nxrt::play::GameDefaults d = [] {
        nxrt::play::GameDefaults g;
        g.settings_text = kSettingsText;
        g.defaults_rev = 1;
        return g;
    }();
    return &d;
}
