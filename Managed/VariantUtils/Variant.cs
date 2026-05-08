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
        : this(VariantNative.Create((byte)VariantType.Null).Variant)
    {
    }

    public Variant(VariantType type)
        : this(VariantNative.Create((byte)type).Variant)
    {
    }

    public Variant(long value)
        : this(VariantNative.CreateI64(value).Variant)
    {
    }

    public Variant(ulong value)
        : this(VariantNative.CreateU64(value).Variant)
    {
    }

    public Variant(double value)
        : this(VariantNative.CreateF64(value).Variant)
    {
    }

    public Variant(string value)
        : this(VariantNative.CreateString(value ?? string.Empty).Variant)
    {
    }

    public Variant(bool value)
        : this(VariantNative.CreateBoolean(value ? 1 : 0).Variant)
    {
    }

    public Variant(byte[] blob, ulong alignment = 0)
        : this(VariantNative.CreateBlob(blob, (ulong)(blob?.Length ?? 0), alignment).Variant)
    {
    }

    public bool IsDisposed => _nativeVariant == IntPtr.Zero;

    public VariantType Type
    {
        get
        {
            EnsureNotDisposed();
            return (VariantType)VariantNative.GetType(Handle);
        }
    }

    public VariantNumberType NumberType
    {
        get
        {
            EnsureNotDisposed();
            return (VariantNumberType)VariantNative.GetNumberType(Handle);
        }
    }

    public bool IsValid
    {
        get
        {
            EnsureNotDisposed();
            return VariantNative.Valid(Handle) != 0;
        }
    }

    public ulong Size
    {
        get
        {
            EnsureNotDisposed();
            return VariantNative.GetSize(Handle);
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
        return new Variant(VariantNative.Clone(Handle).Variant);
    }

    public bool Contains(string key)
    {
        ArgumentException.ThrowIfNullOrEmpty(key);
        EnsureNotDisposed();
        return VariantNative.Contains(Handle, key) != 0;
    }

    public Variant GetArrayItem(ulong index)
    {
        EnsureNotDisposed();
        return new Variant(VariantNative.GetArrayItem(Handle, index).Variant);
    }

    public Variant GetObjectItem(string key)
    {
        ArgumentException.ThrowIfNullOrEmpty(key);
        EnsureNotDisposed();
        return new Variant(VariantNative.GetObjectItem(Handle, key).Variant);
    }

    public string[] GetObjectKeys()
    {
        EnsureNotDisposed();
        var count = Size;
        var result = new string[checked((int)count)];
        for (ulong i = 0; i < count; ++i)
        {
            RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantNative.GetObjectKey(Handle, i, out var key)));
            try
            {
                result[i] = Marshal.PtrToStringUTF8(key) ?? string.Empty;
            }
            finally
            {
                VariantNative.FreeString(key);
            }
        }
        return result;
    }

    public void SetArrayItem(ulong index, Variant value)
    {
        ArgumentNullException.ThrowIfNull(value);
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantNative.SetArrayItem(Handle, index, value.Handle)));
    }

    public void InsertArrayItem(ulong index, Variant value)
    {
        ArgumentNullException.ThrowIfNull(value);
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantNative.InsertArrayItem(Handle, index, value.Handle)));
    }

    public void PushBack(Variant value)
    {
        ArgumentNullException.ThrowIfNull(value);
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantNative.PushBack(Handle, value.Handle)));
    }

    public void EraseArrayItem(ulong index)
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantNative.EraseArrayItem(Handle, index)));
    }

    public void PopBack()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantNative.PopBack(Handle)));
    }

    public void SetObjectItem(string key, Variant value)
    {
        ArgumentException.ThrowIfNullOrEmpty(key);
        ArgumentNullException.ThrowIfNull(value);
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantNative.SetObjectItem(Handle, key, value.Handle)));
    }

    public void EraseObjectItem(string key)
    {
        ArgumentException.ThrowIfNullOrEmpty(key);
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantNative.EraseObjectItem(Handle, key)));
    }

    public string GetString()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantNative.GetString(Handle, out var text)));
        try
        {
            return Marshal.PtrToStringUTF8(text) ?? string.Empty;
        }
        finally
        {
            VariantNative.FreeString(text);
        }
    }

    public long GetSignedNumber()
    {
        EnsureNotDisposed();
        return VariantNative.GetI64(Handle);
    }

    public ulong GetUnsignedNumber()
    {
        EnsureNotDisposed();
        return VariantNative.GetU64(Handle);
    }

    public double GetFloatingNumber()
    {
        EnsureNotDisposed();
        return VariantNative.GetF64(Handle);
    }

    public bool GetBoolean()
    {
        EnsureNotDisposed();
        return VariantNative.GetBoolean(Handle) != 0;
    }

    public byte[] GetBlob()
    {
        EnsureNotDisposed();
        RuntimeErrors.ThrowIfFailed(new ErrorCode(VariantNative.GetBlob(Handle, out var data, out var size, out _)));
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
            VariantNative.FreeBuffer(data);
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
        return VariantNative.Equals(Handle, other.Handle) != 0;
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
            VariantNative.Destroy(new NativeVariantHandle(_nativeVariant));
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
}
