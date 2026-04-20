namespace Luna.Runtime;

public interface ISeekableStream : IStream
{
    ulong Position { get; }

    ulong Size { get; set; }

    void Seek(long offset, SeekMode mode);
}
