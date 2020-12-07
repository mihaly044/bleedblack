using System.Runtime.InteropServices;
using KdlCore.Native.Commons.Win32Types;

namespace bleedblack.net.Native.Wrappers
{
	internal unsafe class MouseClNative
	{
		[DllImport("bleedblack.dll", CharSet = CharSet.Unicode, SetLastError = true)]
		internal static extern void* MouCL_Create();

		[DllImport("bleedblack.dll", CharSet = CharSet.Unicode, SetLastError = true)]
		internal static extern NtStatus MouCL_Init(
			void* pHandle
		);

		[DllImport("bleedblack.dll", CharSet = CharSet.Unicode, SetLastError = true)]
		[return: MarshalAs(UnmanagedType.Bool)]
		internal static extern bool MouCL_IsReady(
			void* pHandle
		);

		[DllImport("bleedblack.dll", CharSet = CharSet.Unicode, SetLastError = true)]
		internal static extern NtStatus MouCL_Move(
			void* pHandle,
			[MarshalAs(UnmanagedType.U8)] ulong ulPid,
			long x,
			long y
		);

        [DllImport("bleedblack.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        internal static extern NtStatus MouCL_Click(
            void* pHandle,
            [MarshalAs(UnmanagedType.U8)] ulong ulPid,
            ushort buttonFlags,
            [MarshalAs(UnmanagedType.U8)] ulong releaseDelayInMilliseconds

		);

		[DllImport("bleedblack.dll", CharSet = CharSet.Unicode, SetLastError = true)]
		internal static extern void MouCL_Destroy(
			void* pHandle
		);
	}
}
