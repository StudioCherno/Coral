using System;
using Newtonsoft.Json;

namespace Testing.Managed;

public class NuGetTest {
#pragma warning disable 0649
    struct TestObject {
        public string name;
    }
#pragma warning restore 0649

    public static void Run() {
        string json = "{ 'name': 'Hello' }";

        TestObject m = JsonConvert.DeserializeObject<TestObject>(json);

        if (m.name == "Hello")
        {
            Console.WriteLine($"NuGet packages seem to be working.");
        }
        else
        {
            Console.WriteLine($"NuGet packages seem to be broken.");
        }
    }
}
