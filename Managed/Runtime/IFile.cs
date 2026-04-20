namespace Luna.Runtime;

public interface IFile : ISeekableStream
{
    void Flush();
}
