try
{
    RhiCSharpTestApp.Run(RhiCSharpTestCase.Clear);
}
finally
{
    if (Luna.Runtime.Runtime.IsInitialized)
    {
        Luna.Runtime.Runtime.Close();
    }
}
