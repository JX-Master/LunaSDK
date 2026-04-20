using Luna.Runtime;

namespace Luna.Window;

public interface IWindow : IObject
{
    bool IsClosed { get; }

    bool HasInputFocus { get; }

    bool HasMouseFocus { get; }

    bool IsMinimized { get; }

    bool IsMaximized { get; }

    bool IsHovered { get; }

    bool IsVisible { get; }

    WindowStyleFlags Style { get; set; }

    Point2I Position { get; set; }

    Size2U Size { get; set; }

    Size2U FramebufferSize { get; }

    float DpiScaleFactor { get; }

    void Close();

    void SetForeground();

    void SetMinimized();

    void SetMaximized();

    void SetRestored();

    void SetVisible(bool visible);

    void SetTitle(string title);

    Point2I ScreenToClient(Point2I point);

    Point2I ClientToScreen(Point2I point);

    void BeginTextInput();

    void SetTextInputArea(RectI inputRect, int cursor);

    void EndTextInput();

    bool IsTextInputActive { get; }
}
