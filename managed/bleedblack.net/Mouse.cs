using System;
using bleedblack.net.Enums;
using bleedblack.net.Native.Wrappers;
using KdlCore.Native.Commons.Win32Types;
using KdlCore.Native.Commons.Wrappers;

namespace bleedblack.net
{
    public unsafe class Mouse : IDisposable
    {
        private readonly void* _handle;

        public bool IsReady
        {
            get
            {
                if (_handle == null)
                    throw new InvalidOperationException();

                return MouseClNative.MouCL_IsReady(_handle);
            }
        }

        public Mouse(string dllPath = "")
        {
            if (!string.IsNullOrEmpty(dllPath))
            {
                Kernel32.SetDllDirectory(dllPath);
            }

            _handle = MouseClNative.MouCL_Create();
        }

        public NtStatus Move(ulong pid, long x, long y)
        {
            if (_handle == null)
                throw new InvalidOperationException();

            return MouseClNative.MouCL_Move(_handle, pid, x, y);
        }

        public NtStatus Click(ulong pid, ushort buttonFlags, ulong releaseDelayInSeconds)
        {
            if (_handle == null)
                throw new InvalidOperationException();

            return MouseClNative.MouCL_Click(_handle, pid, buttonFlags, releaseDelayInSeconds);
        }

        public NtStatus Click(ulong pid, MouseButton button, ulong releaseDelayInSeconds)
        {
            return Click(pid, (ushort) button, releaseDelayInSeconds);
        }

        private void ReleaseUnmanagedResources()
        {
            if (_handle != default)
                MouseClNative.MouCL_Destroy(_handle);
        }

        public void Dispose()
        {
            ReleaseUnmanagedResources();
            GC.SuppressFinalize(this);
        }

        ~Mouse()
        {
            ReleaseUnmanagedResources();
        }
    }
}
