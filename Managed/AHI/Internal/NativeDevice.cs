using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Luna.Runtime;
using Luna.Runtime.Internal;

namespace Luna.AHI.Internal;

internal sealed class NativeDevice : ObjectBase, IDevice, IDisposable
{
    private sealed class CallbackRegistration
    {
        public GCHandle Userdata;
    }

    private sealed class PlaybackCallbackState
    {
        public required NativeDevice Device;
        public required PlaybackDataCallback Callback;
    }

    private sealed class CaptureCallbackState
    {
        public required NativeDevice Device;
        public required CaptureDataCallback Callback;
    }

    private static readonly AhiNative.NativePlaybackDataCallback s_playbackThunk = PlaybackThunk;
    private static readonly AhiNative.NativeCaptureDataCallback s_captureThunk = CaptureThunk;

    private readonly IntPtr _idevice;
    private readonly Dictionary<ulong, CallbackRegistration> _playbackCallbacks = new();
    private readonly Dictionary<ulong, CallbackRegistration> _captureCallbacks = new();
    private readonly object _callbackLock = new();
    private Exception? _pendingCallbackException;

    internal NativeDevice(NativeDeviceHandle handle, bool retain)
        : base(handle.Object, retain)
    {
        if (handle.IDevice == IntPtr.Zero)
        {
            throw new ArgumentException("Native device handle is incomplete.", nameof(handle));
        }
        _idevice = handle.IDevice;
    }

    public uint SampleRate
    {
        get
        {
            EnsureNotDisposed();
            ThrowIfPendingCallbackException();
            return AhiNative.IDeviceGetSampleRate(_idevice);
        }
    }

    public DeviceFlag Flags
    {
        get
        {
            EnsureNotDisposed();
            ThrowIfPendingCallbackException();
            return (DeviceFlag)AhiNative.IDeviceGetFlags(_idevice);
        }
    }

    public uint PlaybackNumChannels
    {
        get
        {
            EnsureNotDisposed();
            ThrowIfPendingCallbackException();
            return AhiNative.IDeviceGetPlaybackNumChannels(_idevice);
        }
    }

    public BitDepth PlaybackBitDepth
    {
        get
        {
            EnsureNotDisposed();
            ThrowIfPendingCallbackException();
            return (BitDepth)AhiNative.IDeviceGetPlaybackBitDepth(_idevice);
        }
    }

    public uint CaptureNumChannels
    {
        get
        {
            EnsureNotDisposed();
            ThrowIfPendingCallbackException();
            return AhiNative.IDeviceGetCaptureNumChannels(_idevice);
        }
    }

    public BitDepth CaptureBitDepth
    {
        get
        {
            EnsureNotDisposed();
            ThrowIfPendingCallbackException();
            return (BitDepth)AhiNative.IDeviceGetCaptureBitDepth(_idevice);
        }
    }

    public ulong AddPlaybackDataCallback(PlaybackDataCallback callback)
    {
        ArgumentNullException.ThrowIfNull(callback);
        EnsureNotDisposed();
        ThrowIfPendingCallbackException();

        var state = new PlaybackCallbackState { Device = this, Callback = callback };
        var userdata = GCHandle.Alloc(state);
        try
        {
            RuntimeErrors.ThrowIfFailed(new ErrorCode(AhiNative.IDeviceAddPlaybackDataCallback(_idevice, s_playbackThunk, GCHandle.ToIntPtr(userdata), out var handle)));
            lock (_callbackLock)
            {
                _playbackCallbacks.Add(handle, new CallbackRegistration { Userdata = userdata });
            }
            return handle;
        }
        catch
        {
            if (userdata.IsAllocated)
            {
                userdata.Free();
            }
            throw;
        }
    }

    public void RemovePlaybackDataCallback(ulong handle)
    {
        EnsureNotDisposed();
        ThrowIfPendingCallbackException();

        CallbackRegistration? registration = null;
        lock (_callbackLock)
        {
            if (_playbackCallbacks.Remove(handle, out var removed))
            {
                registration = removed;
            }
        }
        AhiNative.IDeviceRemovePlaybackDataCallback(_idevice, handle);
        registration?.Userdata.Free();
    }

    public ulong AddCaptureDataCallback(CaptureDataCallback callback)
    {
        ArgumentNullException.ThrowIfNull(callback);
        EnsureNotDisposed();
        ThrowIfPendingCallbackException();

        var state = new CaptureCallbackState { Device = this, Callback = callback };
        var userdata = GCHandle.Alloc(state);
        try
        {
            RuntimeErrors.ThrowIfFailed(new ErrorCode(AhiNative.IDeviceAddCaptureDataCallback(_idevice, s_captureThunk, GCHandle.ToIntPtr(userdata), out var handle)));
            lock (_callbackLock)
            {
                _captureCallbacks.Add(handle, new CallbackRegistration { Userdata = userdata });
            }
            return handle;
        }
        catch
        {
            if (userdata.IsAllocated)
            {
                userdata.Free();
            }
            throw;
        }
    }

    public void RemoveCaptureDataCallback(ulong handle)
    {
        EnsureNotDisposed();
        ThrowIfPendingCallbackException();

        CallbackRegistration? registration = null;
        lock (_callbackLock)
        {
            if (_captureCallbacks.Remove(handle, out var removed))
            {
                registration = removed;
            }
        }
        AhiNative.IDeviceRemoveCaptureDataCallback(_idevice, handle);
        registration?.Userdata.Free();
    }

    public new void Dispose()
    {
        lock (_callbackLock)
        {
            foreach (var pair in _playbackCallbacks)
            {
                AhiNative.IDeviceRemovePlaybackDataCallback(_idevice, pair.Key);
                if (pair.Value.Userdata.IsAllocated)
                {
                    pair.Value.Userdata.Free();
                }
            }
            _playbackCallbacks.Clear();

            foreach (var pair in _captureCallbacks)
            {
                AhiNative.IDeviceRemoveCaptureDataCallback(_idevice, pair.Key);
                if (pair.Value.Userdata.IsAllocated)
                {
                    pair.Value.Userdata.Free();
                }
            }
            _captureCallbacks.Clear();
        }
        base.Dispose();
    }

    void IDisposable.Dispose()
    {
        Dispose();
    }

    private static uint PlaybackThunk(IntPtr destinationBuffer, NativeWaveFormat format, uint numFrames, IntPtr userdata)
    {
        var handle = GCHandle.FromIntPtr(userdata);
        if (handle.Target is not PlaybackCallbackState state)
        {
            return 0;
        }
        try
        {
            return state.Callback(destinationBuffer, format.ToPublic(), numFrames);
        }
        catch (Exception ex)
        {
            state.Device.RecordCallbackException(ex);
            return 0;
        }
    }

    private static void CaptureThunk(IntPtr sourceBuffer, NativeWaveFormat format, uint numFrames, IntPtr userdata)
    {
        var handle = GCHandle.FromIntPtr(userdata);
        if (handle.Target is not CaptureCallbackState state)
        {
            return;
        }
        try
        {
            state.Callback(sourceBuffer, format.ToPublic(), numFrames);
        }
        catch (Exception ex)
        {
            state.Device.RecordCallbackException(ex);
        }
    }

    private void RecordCallbackException(Exception exception)
    {
        lock (_callbackLock)
        {
            _pendingCallbackException ??= exception;
        }
    }

    private void ThrowIfPendingCallbackException()
    {
        Exception? exception;
        lock (_callbackLock)
        {
            exception = _pendingCallbackException;
            _pendingCallbackException = null;
        }
        if (exception is not null)
        {
            throw new InvalidOperationException("An exception was thrown from an AHI callback.", exception);
        }
    }
}
