using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Luna.Runtime;
using Luna.Runtime.Internal;
using Luna.VariantUtils.Internal;

namespace Luna.VariantUtils;

public sealed class Variant : IDisposable, IEquatable<Variant>
{
    private IntPtr _nativeVariant;

    private Variant(IntPtr nativeVariant)
    {
        if (nativeVariant == IntPtr.Zero)
        {
            throw new ArgumentNullException(nameof(nativeVariant));
        }
        _nativeVariant = nativeVariant;
    }

    public Variant()
        : this(VariantUtilsNativeGenerated.VariantCreate((byte)VariantType.Null).Variant)
    {
    }

    public Variant(VariantType type)
        : this(VariantUtilsNativeGenerated.VariantCreate((byte)type).Variant)
    {
    }

    public Variant(long value)
        : this(VariantUtilsNativeGenerated.VariantCreateI64(value).Variant)
    {
    }

    public Variant(ulong value)
        : this(VariantUtilsNativeGenerated.VariantCreateU64(value).Variant)
    {
    }

    public Variant(double value)
        : this(VariantUtilsNativeGenerated.VariantCreateF64(value).Variant)
    {
    }

    public Variant(string value)
        : this(VariantUtilsNativeGenerated.VariantCreateString(value ?? string.Empty).Variant)
    {
    }

    public Variant(bool value)
        : this(VariantUtilsNativeGenerated.VariantCreateBoolean(value ? 1 : 0).Variant)
    {
    }

    public Variant(byte[]? blob, ulong alignment = 0)
        : this(CreateBlobVariant(blob, alignment))
    {
    }

    public bool IsDisposed => _nativeVariant == IntPtr.Zero;

    public VariantType Type
    {
        get
        {
            EnsureNotDisposed();
            return (VariantType)VariantUtilsNativeGenerated.VariantGetType(Handle);
        }
    }

    public VariantNumberType NumberType
    {
        get
        {
            EnsureNotDisposed();
            return (VariantNumberType)VariantUtilsNativeGenerated.VariantGetNumberType(Handle);
        }
    }

    public bool IsValid
    {
        get
        {
            EnsureNotDisposed();
            return VariantUtilsNativeGenerated.VariantValid(Handle) != 0;
        }
    }

    public ulong Size
    {
        get
        {
            EnsureNotDisposed();
            return VariantUtilsNativeGenerated.VariantGetSize(Handle);
        }
    }

    internal NativeVariantHandle Handle
    {
        get
        {
            EnsureNotDisposed();
            return new NativeVariantHandle(_nativeVariant);
        }
    }

    public Variant Clone()
    {
        EnsureNotDisposed();
        return new Variant(VariantUtilsNativeGenerated.VariantClone(Handle).Variant);
    }

    public bool Contains(string key)
    {
        ArgumentException.ThrowIfNullOrEmpty(key);
        EnsureNotDisposed();
        return VariantUtilsNativeGenerated.VariantContains(Handle, key) != 0;
    }

    public Variant GetArrayItem(ulong index)
    {
        EnsureNotDisposed();
        return new Variant(VariantUtilsNativeGenerated.VariantGetArrayItem(Handle, index).Variant);
    }

    public Variant GetObjectItem(string key)
    {
        ArgumentException.ThrowIfNullOrEmpty(key);
        EnsureNotDisposed();
        return new Variant(VariantUtilsNativeGenerated.VariantGetObjectItem(Handle, key).Variant);
    }

    public string[] GetObjectKeys()
    {
        EnsureNotDisposed();
        var count = Size;
        var result = new string[checked((int)count)];
        for (ulong i = 0; i < count; ++i)
        {
            RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.VariantGetObjectKey(Handle, i, out var key)));
            try
            {
                result[i] = Marshal.PtrToStringUTF8(key) ?? string.Empty;
            }
            finally
            {
                VariantUtilsNativeGenerated.FreeString(key);
            }
        }
        return result;
    }

    public void SetArrayItem(ulong index, Variant value)
    {
        ArgumentNullException.ThrowIfNull(value);
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.VariantSetArrayItem(Handle, index, value.Handle)));
    }

    public void InsertArrayItem(ulong index, Variant value)
    {
        ArgumentNullException.ThrowIfNull(value);
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.VariantInsertArrayItem(Handle, index, value.Handle)));
    }

    public void PushBack(Variant value)
    {
        ArgumentNullException.ThrowIfNull(value);
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.VariantPushBack(Handle, value.Handle)));
    }

    public void EraseArrayItem(ulong index)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.VariantEraseArrayItem(Handle, index)));
    }

    public void PopBack()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.VariantPopBack(Handle)));
    }

    public void SetObjectItem(string key, Variant value)
    {
        ArgumentException.ThrowIfNullOrEmpty(key);
        ArgumentNullException.ThrowIfNull(value);
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.VariantSetObjectItem(Handle, key, value.Handle)));
    }

    public void EraseObjectItem(string key)
    {
        ArgumentException.ThrowIfNullOrEmpty(key);
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.VariantEraseObjectItem(Handle, key)));
    }

    public string GetString()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.VariantGetString(Handle, out var text)));
        try
        {
            return Marshal.PtrToStringUTF8(text) ?? string.Empty;
        }
        finally
        {
            VariantUtilsNativeGenerated.FreeString(text);
        }
    }

    public long GetSignedNumber()
    {
        EnsureNotDisposed();
        return VariantUtilsNativeGenerated.VariantGetI64(Handle);
    }

    public ulong GetUnsignedNumber()
    {
        EnsureNotDisposed();
        return VariantUtilsNativeGenerated.VariantGetU64(Handle);
    }

    public double GetFloatingNumber()
    {
        EnsureNotDisposed();
        return VariantUtilsNativeGenerated.VariantGetF64(Handle);
    }

    public bool GetBoolean()
    {
        EnsureNotDisposed();
        return VariantUtilsNativeGenerated.VariantGetBoolean(Handle) != 0;
    }

    public byte[] GetBlob()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantUtilsNativeGenerated.VariantGetBlob(Handle, out var data, out var size, out _)));
        try
        {
            if (size == 0 || data == IntPtr.Zero)
            {
                return Array.Empty<byte>();
            }
            if (size > int.MaxValue)
            {
                throw new InvalidOperationException("The blob is too large to copy into a managed byte array.");
            }
            var result = new byte[(int)size];
            Marshal.Copy(data, result, 0, result.Length);
            return result;
        }
        finally
        {
            VariantUtilsNativeGenerated.FreeBuffer(data);
        }
    }

    public bool Equals(Variant? other)
    {
        if (other is null)
        {
            return false;
        }
        EnsureNotDisposed();
        other.EnsureNotDisposed();
        return VariantUtilsNativeGenerated.VariantEquals(Handle, other.Handle) != 0;
    }

    public override bool Equals(object? obj)
    {
        return obj is Variant other && Equals(other);
    }

    public override int GetHashCode()
    {
        EnsureNotDisposed();
        return HashCode.Combine(Type, NumberType, Size, IsValid);
    }

    public void Dispose()
    {
        if (_nativeVariant != IntPtr.Zero)
        {
            VariantUtilsNativeGenerated.VariantDestroy(new NativeVariantHandle(_nativeVariant));
            _nativeVariant = IntPtr.Zero;
        }
        GC.SuppressFinalize(this);
    }

    public static bool operator ==(Variant? lhs, Variant? rhs)
    {
        if (ReferenceEquals(lhs, rhs))
        {
            return true;
        }
        if (lhs is null || rhs is null)
        {
            return false;
        }
        return lhs.Equals(rhs);
    }

    public static bool operator !=(Variant? lhs, Variant? rhs)
    {
        return !(lhs == rhs);
    }

    internal static Variant FromNative(NativeVariantHandle handle)
    {
        return new Variant(handle.Variant);
    }

    internal static NativeVariantHandle[] ToNativeArray(IReadOnlyList<Variant> variants)
    {
        var result = new NativeVariantHandle[variants.Count];
        for (var i = 0; i < variants.Count; ++i)
        {
            result[i] = variants[i].Handle;
        }
        return result;
    }

    internal void EnsureNotDisposed()
    {
        if (IsDisposed)
        {
            throw new ObjectDisposedException(nameof(Variant));
        }
    }

    private static IntPtr CreateBlobVariant(byte[]? blob, ulong alignment)
    {
        using var pinnedBlob = PinnedByteArray.Create(blob);
        return VariantUtilsNativeGenerated.VariantCreateBlob(pinnedBlob.Pointer, (ulong)(blob?.Length ?? 0), alignment).Variant;
    }

    private readonly struct PinnedByteArray : IDisposable
    {
        private readonly GCHandle _handle;

        private PinnedByteArray(GCHandle handle, IntPtr pointer)
        {
            _handle = handle;
            Pointer = pointer;
        }

        public IntPtr Pointer { get; }

        public static PinnedByteArray Create(byte[]? data)
        {
            if (data is null || data.Length == 0)
            {
                return new PinnedByteArray(default, IntPtr.Zero);
            }
            var handle = GCHandle.Alloc(data, GCHandleType.Pinned);
            return new PinnedByteArray(handle, handle.AddrOfPinnedObject());
        }

        public void Dispose()
        {
            if (_handle.IsAllocated)
            {
                _handle.Free();
            }
        }
    }
}
