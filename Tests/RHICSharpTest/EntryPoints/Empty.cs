try
{
    RhiCSharpTestApp.Run(RhiCSharpTestCase.Empty);
}
finally
{
    if (Luna.Runtime.Runtime.IsInitialized)
    {
        Luna.Runtime.Runtime.Close();
    }
}
