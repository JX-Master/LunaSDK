try
{
    RhiCSharpTestApp.Run(RhiCSharpTestCase.Box);
}
finally
{
    if (Luna.Runtime.Runtime.IsInitialized)
    {
        Luna.Runtime.Runtime.Close();
    }
}
