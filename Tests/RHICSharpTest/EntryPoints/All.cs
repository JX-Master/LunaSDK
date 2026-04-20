try
{
    RhiCSharpTestApp.Run();
}
finally
{
    if (Luna.Runtime.Runtime.IsInitialized)
    {
        Luna.Runtime.Runtime.Close();
    }
}
