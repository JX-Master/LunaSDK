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
            return WindowNative.IWindowIsClosed(_iwindow) != 0;
        }
    }

    public bool HasInputFocus
    {
        get
        {
            EnsureNotDisposed();
            return WindowNative.IWindowHasInputFocus(_iwindow) != 0;
        }
    }

    public bool HasMouseFocus
    {
        get
        {
            EnsureNotDisposed();
            return WindowNative.IWindowHasMouseFocus(_iwindow) != 0;
        }
    }

    public bool IsMinimized
    {
        get
        {
            EnsureNotDisposed();
            return WindowNative.IWindowIsMinimized(_iwindow) != 0;
        }
    }

    public bool IsMaximized
    {
        get
        {
            EnsureNotDisposed();
            return WindowNative.IWindowIsMaximized(_iwindow) != 0;
        }
    }

    public bool IsHovered
    {
        get
        {
            EnsureNotDisposed();
            return WindowNative.IWindowIsHovered(_iwindow) != 0;
        }
    }

    public bool IsVisible
    {
        get
        {
            EnsureNotDisposed();
            return WindowNative.IWindowIsVisible(_iwindow) != 0;
        }
    }

    public WindowStyleFlags Style
    {
        get
        {
            EnsureNotDisposed();
            return (WindowStyleFlags)WindowNative.IWindowGetStyle(_iwindow);
        }
        set
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.IWindowSetStyle(_iwindow, (uint)value)));
        }
    }

    public Point2I Position
    {
        get
        {
            EnsureNotDisposed();
            WindowNative.IWindowGetPosition(_iwindow, out var x, out var y);
            return new Point2I(x, y);
        }
        set
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.IWindowSetPosition(_iwindow, value.X, value.Y)));
        }
    }

    public Size2U Size
    {
        get
        {
            EnsureNotDisposed();
            WindowNative.IWindowGetSize(_iwindow, out var width, out var height);
            return new Size2U(width, height);
        }
        set
        {
            EnsureNotDisposed();
            RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.IWindowSetSize(_iwindow, value.Width, value.Height)));
        }
    }

    public Size2U FramebufferSize
    {
        get
        {
            EnsureNotDisposed();
            WindowNative.IWindowGetFramebufferSize(_iwindow, out var width, out var height);
            return new Size2U(width, height);
        }
    }

    public float DpiScaleFactor
    {
        get
        {
            EnsureNotDisposed();
            return WindowNative.IWindowGetDpiScaleFactor(_iwindow);
        }
    }

    public bool IsTextInputActive
    {
        get
        {
            EnsureNotDisposed();
            return WindowNative.IWindowIsTextInputActive(_iwindow) != 0;
        }
    }

    public void Close()
    {
        EnsureNotDisposed();
        WindowNative.IWindowClose(_iwindow);
    }

    public void SetForeground()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.IWindowSetForeground(_iwindow)));
    }

    public void SetMinimized()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.IWindowSetMinimized(_iwindow)));
    }

    public void SetMaximized()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.IWindowSetMaximized(_iwindow)));
    }

    public void SetRestored()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.IWindowSetRestored(_iwindow)));
    }

    public void SetVisible(bool visible)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.IWindowSetVisible(_iwindow, visible ? 1 : 0)));
    }

    public void SetTitle(string title)
    {
        EnsureNotDisposed();
        ArgumentNullException.ThrowIfNull(title);
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.IWindowSetTitle(_iwindow, title)));
    }

    public Point2I ScreenToClient(Point2I point)
    {
        EnsureNotDisposed();
        WindowNative.IWindowScreenToClient(_iwindow, point.X, point.Y, out var x, out var y);
        return new Point2I(x, y);
    }

    public Point2I ClientToScreen(Point2I point)
    {
        EnsureNotDisposed();
        WindowNative.IWindowClientToScreen(_iwindow, point.X, point.Y, out var x, out var y);
        return new Point2I(x, y);
    }

    public void BeginTextInput()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.IWindowBeginTextInput(_iwindow)));
    }

    public void SetTextInputArea(RectI inputRect, int cursor)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.IWindowSetTextInputArea(_iwindow, inputRect, cursor)));
    }

    public void EndTextInput()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(WindowNative.IWindowEndTextInput(_iwindow)));
    }

    private void EnsureNotDisposed()
    {
        base.EnsureNotDisposed();
    }
}
