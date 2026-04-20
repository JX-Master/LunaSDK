namespace Luna.Image;

public sealed class ImageData
{
    public ImageData(byte[] data, ImageDesc desc)
    {
        Data = data;
        Desc = desc;
    }

    public byte[] Data { get; }

    public ImageDesc Desc { get; }
}
