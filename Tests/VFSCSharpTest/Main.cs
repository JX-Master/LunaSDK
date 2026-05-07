using System;
using System.IO;
using System.Text;
using Luna.Runtime;
using Luna.VFS;

Runtime.Init();

try
{
    VfsModule.Init();

    if (RuntimeErrors.GetCategoryName(Errors.Category) != "VFSError")
    {
        throw new InvalidOperationException("VFS error category lookup failed.");
    }
    if (RuntimeErrors.GetCodeName(Errors.DriverNotFound) != "driver_not_found")
    {
        throw new InvalidOperationException("VFS driver_not_found lookup failed.");
    }

    var driver = VfsModule.PlatformFilesystemDriver;
    if (string.IsNullOrWhiteSpace(driver))
    {
        throw new InvalidOperationException("Platform filesystem driver should not be empty.");
    }

    var nativeRoot = Path.Combine(Path.GetTempPath(), $"LunaVFSCSharpTest-{System.Guid.NewGuid():N}");
    var mountPath = $"/managed-vfs-{System.Guid.NewGuid():N}";
    var remountedPath = $"{mountPath}-remounted";
    var currentMountPath = mountPath;
    Directory.CreateDirectory(nativeRoot);

    try
    {
        VfsModule.Mount(driver, nativeRoot, mountPath);
        try
        {
            VfsModule.CreateDirectory($"{mountPath}/nested");

            var payload = Encoding.UTF8.GetBytes("managed vfs smoke");
            var filePath = $"{mountPath}/nested/data.txt";
            using (var file = VfsModule.OpenFile(filePath, FileOpenFlags.Read | FileOpenFlags.Write | FileOpenFlags.UserBuffering, FileCreationMode.CreateAlways))
            {
                if (file.Write(payload) != (ulong)payload.Length)
                {
                    throw new InvalidOperationException("VFS write byte count mismatch.");
                }
                file.Flush();
                if (file.Size != (ulong)payload.Length)
                {
                    throw new InvalidOperationException("VFS file size mismatch after write.");
                }
                file.Seek(0, SeekMode.Begin);

                var readback = new byte[payload.Length];
                if (file.Read(readback) != (ulong)readback.Length)
                {
                    throw new InvalidOperationException("VFS read byte count mismatch.");
                }
                if (!readback.AsSpan().SequenceEqual(payload))
                {
                    throw new InvalidOperationException("VFS readback content mismatch.");
                }
            }

            var attribute = VfsModule.GetFileAttribute(filePath);
            if (attribute.Size != (ulong)payload.Length)
            {
                throw new InvalidOperationException("VFS file attribute size mismatch.");
            }

            var nativeFilePath = VfsModule.GetNativePath(filePath);
            var expectedNativeFilePath = Path.Combine(nativeRoot, "nested", "data.txt");
            if (Path.GetFullPath(nativeFilePath) != Path.GetFullPath(expectedNativeFilePath))
            {
                throw new InvalidOperationException($"VFS native path translation mismatch: expected {expectedNativeFilePath}, got {nativeFilePath}.");
            }

            var foundNestedDir = false;
            using (var iterator = VfsModule.OpenDirectory(mountPath))
            {
                while (iterator.IsValid)
                {
                    if (iterator.FileName == "nested")
                    {
                        foundNestedDir = true;
                        break;
                    }
                    iterator.MoveNext();
                }
            }
            if (!foundNestedDir)
            {
                throw new InvalidOperationException("VFS directory iterator did not find the nested directory.");
            }

            var copyPath = $"{mountPath}/copy.txt";
            var movedPath = $"{mountPath}/moved.txt";
            VfsModule.CopyFile(filePath, copyPath);
            VfsModule.MoveFile(copyPath, movedPath);

            using (var movedFile = VfsModule.OpenFile(movedPath, FileOpenFlags.Read | FileOpenFlags.UserBuffering, FileCreationMode.OpenExisting))
            {
                var movedData = RuntimeFile.LoadData(movedFile);
                if (!movedData.AsSpan().SequenceEqual(payload))
                {
                    throw new InvalidOperationException("VFS moved file content mismatch.");
                }
            }

            VfsModule.Remount(mountPath, remountedPath);
            currentMountPath = remountedPath;

            using (var remountedFile = VfsModule.OpenFile($"{remountedPath}/nested/data.txt", FileOpenFlags.Read | FileOpenFlags.UserBuffering, FileCreationMode.OpenExisting))
            {
                var remountedData = RuntimeFile.LoadData(remountedFile);
                if (!remountedData.AsSpan().SequenceEqual(payload))
                {
                    throw new InvalidOperationException("VFS remounted file content mismatch.");
                }
            }

            VfsModule.DeleteFile($"{remountedPath}/moved.txt");
            VfsModule.DeleteFile($"{remountedPath}/nested/data.txt");
            VfsModule.DeleteFile($"{remountedPath}/nested");
        }
        finally
        {
            VfsModule.Unmount(currentMountPath);
        }
    }
    finally
    {
        if (Directory.Exists(nativeRoot))
        {
            Directory.Delete(nativeRoot, recursive: true);
        }
    }

    Console.WriteLine("VFSCSharpTest passed.");
}
finally
{
    Runtime.Close();
}
