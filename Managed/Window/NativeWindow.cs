using System;
using Luna.Runtime;
using Luna.Window.Internal;

namespace Luna.Window;

internal sealed class NativeWindow : ObjectBase, IWindow
{
    private readonly IntPtr _iwindow;

    internal NativeWindow(IntPtr nativeObject, IntPtr nativeWindow, bool retain)
        : base(nativeObject, retain)
    {
        if (nativeWindow == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeWindow));
        }
        _iwindow = nativeWindow;
    }

    public bool IsClosed
    {
        get
        {
            EnsureNotDisposed();
            return WindowNativeGenerated.IwindowIsClosed(_iwindow) != 0;
        }
    }

    public bool HasInputFocus
    {
        get
        {
            EnsureNotDisposed();
            return WindowNativeGenerated.IwindowHasInputFocus(_iwindow) != 0;
        }
    }

    public bool HasMouseFocus
    {
        get
        {
            EnsureNotDisposed();
            return WindowNativeGenerated.IwindowHasMouseFocus(_iwindow) != 0;
        }
    }

    public bool IsMinimized
    {
        get
        {
            EnsureNotDisposed();
            return WindowNativeGenerated.IwindowIsMinimized(_iwindow) != 0;
        }
    }

    public bool IsMaximized
    {
        get
        {
            EnsureNotDisposed();
            return WindowNativeGenerated.IwindowIsMaximized(_iwindow) != 0;
        }
    }

    public bool IsHovered
    {
        get
        {
            EnsureNotDisposed();
            return WindowNativeGenerated.IwindowIsHovered(_iwindow) != 0;
        }
    }

    public bool IsVisible
    {
        get
        {
            EnsureNotDisposed();
            return WindowNativeGenerated.IwindowIsVisible(_iwindow) != 0;
        }
    }

    public WindowStyleFlags Style
    {
        get
        {
            EnsureNotDisposed();
            return (WindowStyleFlags)WindowNativeGenerated.IwindowGetStyle(_iwindow);
        }
        set
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNativeGenerated.IwindowSetStyle(_iwindow, (uint)value)));
        }
    }

    public Point2I Position
    {
        get
        {
            EnsureNotDisposed();
            WindowNativeGenerated.IwindowGetPosition(_iwindow, out var x, out var y);
            return new Point2I(x, y);
        }
        set
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNativeGenerated.IwindowSetPosition(_iwindow, value.X, value.Y)));
        }
    }

    public Size2U Size
    {
        get
        {
            EnsureNotDisposed();
            WindowNativeGenerated.IwindowGetSize(_iwindow, out var width, out var height);
            return new Size2U(width, height);
        }
        set
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNativeGenerated.IwindowSetSize(_iwindow, value.Width, value.Height)));
        }
    }

    public Size2U FramebufferSize
    {
        get
        {
            EnsureNotDisposed();
            WindowNativeGenerated.IwindowGetFramebufferSize(_iwindow, out var width, out var height);
            return new Size2U(width, height);
        }
    }

    public float DpiScaleFactor
    {
        get
        {
            EnsureNotDisposed();
            return WindowNativeGenerated.IwindowGetDpiScaleFactor(_iwindow);
        }
    }

    public bool IsTextInputActive
    {
        get
        {
            EnsureNotDisposed();
            return WindowNativeGenerated.IwindowIsTextInputActive(_iwindow) != 0;
        }
    }

    public void Close()
    {
        EnsureNotDisposed();
        WindowNativeGenerated.IwindowClose(_iwindow);
    }

    public void SetForeground()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNativeGenerated.IwindowSetForeground(_iwindow)));
    }

    public void SetMinimized()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNativeGenerated.IwindowSetMinimized(_iwindow)));
    }

    public void SetMaximized()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNativeGenerated.IwindowSetMaximized(_iwindow)));
    }

    public void SetRestored()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNativeGenerated.IwindowSetRestored(_iwindow)));
    }

    public void SetVisible(bool visible)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNativeGenerated.IwindowSetVisible(_iwindow, visible ? 1 : 0)));
    }

    public void SetTitle(string title)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(title);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNativeGenerated.IwindowSetTitle(_iwindow, title)));
    }

    public Point2I ScreenToClient(Point2I point)
    {
        EnsureNotDisposed();
        WindowNativeGenerated.IwindowScreenToClient(_iwindow, point.X, point.Y, out var x, out var y);
        return new Point2I(x, y);
    }

    public Point2I ClientToScreen(Point2I point)
    {
        EnsureNotDisposed();
        WindowNativeGenerated.IwindowClientToScreen(_iwindow, point.X, point.Y, out var x, out var y);
        return new Point2I(x, y);
    }

    public void BeginTextInput()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNativeGenerated.IwindowBeginTextInput(_iwindow)));
    }

    public void SetTextInputArea(RectI inputRect, int cursor)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNativeGenerated.IwindowSetTextInputArea(_iwindow, inputRect, cursor)));
    }

    public void EndTextInput()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNativeGenerated.IwindowEndTextInput(_iwindow)));
    }

    private void EnsureNotDisposed()
    {
        base.EnsureNotDisposed();
    }
}
