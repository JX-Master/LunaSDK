namespace Luna.Runtime;

public interface IFileIterator : IObject
{
    bool IsValid { get; }

    string? FileName { get; }

    FileAttributeFlags Attributes { get; }

    bool MoveNext();
}
