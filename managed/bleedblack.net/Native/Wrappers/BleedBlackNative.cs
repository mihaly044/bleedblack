using System.Runtime.InteropServices;
using KdlCore.Native.Commons.Win32Types;

namespace bleedblack.net.Native.Wrappers
{
	internal unsafe class BleedBlackNative
	{
		[DllImport("bleedblack.dll", CharSet = CharSet.Unicode, SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
		internal static extern void* Bleedblack_Create();

		[DllImport("bleedblack.dll", CharSet = CharSet.Unicode, SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
		internal static extern NtStatus Bleedblack_Init(
			void* pHandle
		);

		[DllImport("bleedblack.dll", CharSet = CharSet.Unicode, SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
		[return: MarshalAs(UnmanagedType.Bool)]
		internal static extern bool Bleedblack_IsReady(
			void* pHandle
		);

		[DllImport("bleedblack.dll", CharSet = CharSet.Unicode, SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
		internal static extern NtStatus Bleedblack_Move(
			void* pHandle,
			[MarshalAs(UnmanagedType.U8)] ulong ulPid,
			long x,
			long y
		);

        [DllImport("bleedblack.dll", CharSet = CharSet.Unicode, SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        internal static extern NtStatus Bleedblack_Click(
            void* pHandle,
            [MarshalAs(UnmanagedType.U8)] ulong ulPid,
            ushort buttonFlags,
            [MarshalAs(UnmanagedType.U8)] ulong releaseDelayInMilliseconds

		);

		[DllImport("bleedblack.dll", CharSet = CharSet.Unicode, SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
		internal static extern void Bleedblack_Destroy(
			void* pHandle
		);
	}
}
