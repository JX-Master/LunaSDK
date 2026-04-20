namespace Luna.RHI;

public readonly struct CommandQueueDesc
{
    public CommandQueueDesc(CommandQueueType type, CommandQueueFlags flags)
    {
        Type = type;
        Flags = flags;
    }

    public CommandQueueType Type { get; }

    public CommandQueueFlags Flags { get; }
}
