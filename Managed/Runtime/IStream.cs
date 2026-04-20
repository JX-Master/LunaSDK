namespace Luna.Runtime;

public interface IStream : IObject
{
    ulong Read(byte[] buffer);

    ulong Read(byte[] buffer, int offset, int count);

    ulong Write(byte[] buffer);

    ulong Write(byte[] buffer, int offset, int count);
}
