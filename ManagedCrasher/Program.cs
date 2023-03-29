using System;
using System.Threading;

namespace ManagedCrasher
{
    internal class Program
    {
        static void TestFirstChance()
        {
            throw new Exception();
        }

        static void Main(string[] args)
        {
            Console.WriteLine("Starting");
            try
            {
                TestFirstChance();
            }
            catch
            {
                Console.WriteLine("Caught");
            }

            Console.WriteLine("Sleeping...");
            Thread.Sleep(TimeSpan.FromSeconds(10));
            Console.WriteLine("Dying:");
            throw new Exception();
        }
    }
}
