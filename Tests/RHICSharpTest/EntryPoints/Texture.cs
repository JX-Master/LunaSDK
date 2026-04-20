try
{
    RhiCSharpTestApp.Run(RhiCSharpTestCase.Texture);
}
finally
{
    if (Luna.Runtime.Runtime.IsInitialized)
    {
        Luna.Runtime.Runtime.Close();
    }
}
