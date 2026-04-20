using System;
using System.IO;
using System.Text;
using Luna.Runtime;

Runtime.Init();

try
{
    if (!Runtime.IsInitialized)
    {
        throw new InvalidOperationException("Luna runtime should be initialized.");
    }

    var failure = RuntimeErrors.GetCodeByName("BasicError", "failure");
    if (failure.Succeeded)
    {
        throw new InvalidOperationException("BasicError::failure should not be success.");
    }
    if (RuntimeErrors.GetCodeName(failure) != "failure")
    {
        throw new InvalidOperationException("BasicError::failure name lookup failed.");
    }
    var category = RuntimeErrors.GetCodeCategory(failure);
    if (RuntimeErrors.GetCategoryName(category) != "BasicError")
    {
        throw new InvalidOperationException("BasicError category lookup failed.");
    }
    if (RuntimeErrors.Explain(failure) != "failure")
    {
        throw new InvalidOperationException("BasicError::failure explanation lookup failed.");
    }

    if (RuntimeTime.TicksPerSecond <= 0)
    {
        throw new InvalidOperationException("Runtime tick frequency should be positive.");
    }
    var utcTimestamp = RuntimeTime.UtcTimestamp;
    var utcDateTime = RuntimeTime.TimestampToDateTime(utcTimestamp);
    if (utcDateTime.Year < 2020)
    {
        throw new InvalidOperationException("UTC timestamp to date-time conversion returned an unexpected year.");
    }
    var roundtripTimestamp = RuntimeTime.DateTimeToTimestamp(utcDateTime);
    if (Math.Abs(roundtripTimestamp - utcTimestamp) > 1)
    {
        throw new InvalidOperationException("Date-time timestamp roundtrip drifted too far.");
    }

    var currentDirectory = RuntimePaths.CurrentDirectory;
    if (string.IsNullOrWhiteSpace(currentDirectory))
    {
        throw new InvalidOperationException("Current directory should not be empty.");
    }
    RuntimePaths.CurrentDirectory = currentDirectory;

    if (string.IsNullOrWhiteSpace(RuntimePaths.ProcessPath))
    {
        throw new InvalidOperationException("Process path should not be empty.");
    }

    RuntimeLog.Info("RuntimeCSharpTest", "managed runtime log smoke");

    var testDirectory = Path.Combine(Path.GetTempPath(), $"LunaRuntimeCSharpTest-{System.Guid.NewGuid():N}");
    var filePath = Path.Combine(testDirectory, "data.txt");
    var copyPath = Path.Combine(testDirectory, "copy.txt");
    var movedPath = Path.Combine(testDirectory, "moved.txt");
    RuntimeFile.CreateDirectory(testDirectory);
    try
    {
        var payload = Encoding.UTF8.GetBytes("managed file smoke");
        using (var file = RuntimeFile.Open(filePath, FileOpenFlags.Read | FileOpenFlags.Write, FileCreationMode.CreateAlways))
        {
            if (file.Write(payload) != (ulong)payload.Length)
            {
                throw new InvalidOperationException("File write byte count mismatch.");
            }
            file.Flush();
            if (file.Size != (ulong)payload.Length)
            {
                throw new InvalidOperationException("File size mismatch after write.");
            }
            file.Seek(0, SeekMode.Begin);

            var readBuffer = new byte[payload.Length + 4];
            if (file.Read(readBuffer, 2, payload.Length) != (ulong)payload.Length)
            {
                throw new InvalidOperationException("File read byte count mismatch.");
            }
            var readText = Encoding.UTF8.GetString(readBuffer, 2, payload.Length);
            if (readText != "managed file smoke")
            {
                throw new InvalidOperationException("File read content mismatch.");
            }
        }

        var attribute = RuntimeFile.GetAttribute(filePath);
        if (attribute.Size != (ulong)payload.Length)
        {
            throw new InvalidOperationException("File attribute size mismatch.");
        }

        var loadedPayload = RuntimeFile.LoadData(filePath);
        if (Encoding.UTF8.GetString(loadedPayload) != "managed file smoke")
        {
            throw new InvalidOperationException("LoadData path content mismatch.");
        }

        using (var file = RuntimeFile.Open(filePath, FileOpenFlags.Read | FileOpenFlags.UserBuffering, FileCreationMode.OpenExisting))
        {
            file.Seek(7, SeekMode.Begin);
            loadedPayload = RuntimeFile.LoadData(file);
            if (Encoding.UTF8.GetString(loadedPayload) != "managed file smoke")
            {
                throw new InvalidOperationException("LoadData file content mismatch.");
            }
            if (file.Position != 7)
            {
                throw new InvalidOperationException("LoadData should restore the original file position.");
            }
        }

        var foundDataFile = false;
        using (var iterator = RuntimeFile.OpenDirectory(testDirectory))
        {
            while (iterator.IsValid)
            {
                if (iterator.FileName == "data.txt")
                {
                    foundDataFile = true;
                    break;
                }
                iterator.MoveNext();
            }
        }
        if (!foundDataFile)
        {
            throw new InvalidOperationException("Directory iterator did not find the test file.");
        }

        RuntimeFile.Copy(filePath, copyPath);
        RuntimeFile.Move(copyPath, movedPath);
        RuntimeFile.Delete(movedPath);
        RuntimeFile.Delete(filePath);
    }
    finally
    {
        if (Directory.Exists(testDirectory))
        {
            foreach (var path in Directory.EnumerateFiles(testDirectory))
            {
                RuntimeFile.Delete(path);
            }
            RuntimeFile.Delete(testDirectory);
        }
    }

    Console.WriteLine("RuntimeCSharpTest passed.");
}
finally
{
    Runtime.Close();
}
