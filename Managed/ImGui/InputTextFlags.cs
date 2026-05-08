using System;

namespace Luna.ImGui;

[Flags]
public enum InputTextFlags : uint
{
    None = 0,
    CharsDecimal = 1u << 0,
    CharsHexadecimal = 1u << 1,
    CharsScientific = 1u << 2,
    CharsUppercase = 1u << 3,
    CharsNoBlank = 1u << 4,
    AllowTabInput = 1u << 5,
    EnterReturnsTrue = 1u << 6,
    EscapeClearsAll = 1u << 7,
    CtrlEnterForNewLine = 1u << 8,
    ReadOnly = 1u << 9,
    Password = 1u << 10,
    AlwaysOverwrite = 1u << 11,
    AutoSelectAll = 1u << 12,
    ParseEmptyRefVal = 1u << 13,
    DisplayEmptyRefVal = 1u << 14,
    NoHorizontalScroll = 1u << 15,
    NoUndoRedo = 1u << 16,
    ElideLeft = 1u << 17,
    CallbackCompletion = 1u << 18,
    CallbackHistory = 1u << 19,
    CallbackAlways = 1u << 20,
    CallbackCharFilter = 1u << 21,
    CallbackResize = 1u << 22,
    CallbackEdit = 1u << 23,
    WordWrap = 1u << 24
}
