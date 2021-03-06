using System;
using System.Diagnostics;
using System.Threading;
using KdlCore.Native.Commons.Win32Types;
using bleedblack.net;
using bleedblack.net.Enums;

namespace BleedBlackCSharp
{
    class Program
    {
        static void Main()
        {
            using var mouse = new Mouse();
            var status = mouse.Init();

            if (!status.IsSuccess())
            {
                Console.WriteLine("Failed to initialize. Is the driver loaded?");
                return;
            }

            //
            // Move the cursor in a circle
            //
            Console.WriteLine(@"Watch the cursor do a full circle.");

            //
            // For simplicity, mouse events will execute in the context of our own process.
            //
            var myPid = (ulong)(Process.GetCurrentProcess().Id);
            const int radius = 15;
            for (var i = 0d; i < 2 * Math.PI; i += 0.1d)
            {
                mouse.Move(pid: myPid,
                    x: (long) (radius * Math.Cos(i)),
                    y: (long) (radius * Math.Sin(i)));
                Thread.Sleep(5);
            }

            //
            // Left click
            //
            Console.WriteLine("Left click in 3 seconds ...");
            Thread.Sleep(3000);
            mouse.Click(myPid, MouseButton.LEFT, 30);

            //
            // Right click
            //
            Console.WriteLine("Right click in 3 seconds ...");
            Thread.Sleep(3000);
            mouse.Click(myPid, MouseButton.RIGHT, 30);

            //
            // Wait for the bleedblack driver to finish up before quitting
            // TODO: Use an event for this
            //
            Thread.Sleep(300);
        }
    }
}
