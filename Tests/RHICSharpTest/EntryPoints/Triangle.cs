try
{
    RhiCSharpTestApp.Run(RhiCSharpTestCase.Triangle);
}
finally
{
    if (Luna.Runtime.Runtime.IsInitialized)
    {
        Luna.Runtime.Runtime.Close();
    }
}
