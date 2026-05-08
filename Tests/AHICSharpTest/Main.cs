using System;
using System.Threading;
using Luna.AHI;
using Luna.Runtime;

Runtime.Init();

try
{
    Module.Init();

    Module.GetAdapters(out var playbackAdapters, out var captureAdapters);
    Console.WriteLine($"Playback adapters: {playbackAdapters.Length}");
    Console.WriteLine($"Capture adapters: {captureAdapters.Length}");

    foreach (var adapter in playbackAdapters)
    {
        Console.WriteLine($"Playback: {adapter.Name} (primary={adapter.IsPrimary}) formats={adapter.GetNativeWaveFormats().Length}");
    }

    foreach (var adapter in captureAdapters)
    {
        Console.WriteLine($"Capture: {adapter.Name} (primary={adapter.IsPrimary}) formats={adapter.GetNativeWaveFormats().Length}");
    }

    if (playbackAdapters.Length == 0)
    {
        throw new InvalidOperationException("No playback audio adapter is available.");
    }

    using (var device = Module.CreateDevice(new DeviceDesc(
        new DeviceIoDesc(playbackAdapters[0], 2, BitDepth.Unspecified),
        default,
        0,
        DeviceFlag.Playback)))
    {
        long callbackCount = 0;
        var callbackHandle = device.AddPlaybackDataCallback((_, _, _) =>
        {
            Interlocked.Increment(ref callbackCount);
            return 0;
        });

        try
        {
            var deadline = System.DateTime.UtcNow.AddSeconds(2);
            while (Interlocked.Read(ref callbackCount) == 0 && System.DateTime.UtcNow < deadline)
            {
                Thread.Sleep(10);
            }

            if (Interlocked.Read(ref callbackCount) == 0)
            {
                throw new InvalidOperationException("The playback callback was not invoked within the timeout.");
            }

            Console.WriteLine($"Playback callback count: {callbackCount}");
            Console.WriteLine($"Device sample rate: {device.SampleRate}");
            Console.WriteLine($"Playback format: {device.PlaybackBitDepth}, channels={device.PlaybackNumChannels}");
            Console.WriteLine("AHICSharpTest passed.");
        }
        finally
        {
            device.RemovePlaybackDataCallback(callbackHandle);
        }
    }

    foreach (var adapter in playbackAdapters)
    {
        adapter.Dispose();
    }
    foreach (var adapter in captureAdapters)
    {
        adapter.Dispose();
    }
}
finally
{
    Runtime.Close();
}
