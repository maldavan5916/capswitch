# CapsWitch 🧙‍♀️⬆️
A simple and lightweight program that switches keyboard layout with the CapsLock key — just put it in the Startup folder (press **Win+R** and type <code>shell:startup</code> to open) and enjoy!

> **Note**: For keyboard layout switching to work in programs running with administrator privileges, CapsWitch must also be run with administrator privileges. This can be done, in example, [using Task Scheduler](https://youtu.be/jt1Eb-NaBeg). <br>CapsWitch can also run without privileges elevation, however it will only work in other non-elevated programs.


## Install it!
+ [Windows 7–11 (x64)](https://github.com/1280px/CapsWitch/releases/)
+ [Windows XP–? (x86)](https://github.com/1280px/CapsWitch/releases/)

<details><summary>Building Information</summary>

### For modern systems
1. Use Microsoft Visual Studio 2017 or newer
2. Compile for x64 or x86

### For legacy systems
1. Use Microsoft Visual Studio 2017 or newer
2. Install ```Platform Toolset v141_xp``` (the last version that supports compilation for XP)
3. Go to Solution Explorer, right click on CapsWitch and open Properties. <br>Then, find ```PlatformToolset``` and select ```Visual Studio 2017 - Windows XP (v141_xp)```
4. In the same window, go to ```C/C++ > Code Creation```, find ```Runtime Library``` and change it to ```/MT```
5. Apply and compile for x86
</details>


## Use it!
The program works by emulating specific language switch combination (Alt+Shift by default, but can be configured in settings) whenever Caps Lock key is pressed. The controls are:

+ **CapsLock** to change keyboard layout  
+ **Shift+CapsLock / CapsLock+Shift** to toggle CapsLock state
+ **Alt+CapsLock** to enable/disable *(only works if enabled in settings)*


## Configure it!
CapsWitch allows some customization using .ini settings file. By default, it looks for ```CapsWitch.ini``` file in the same directory, however you can pass custom file (with path) using agruments (e.g. ```.\CapsWitch.exe "C:\test.ini"```). The program can also work without .ini file, simply using default settings values.

*You can see more information about each option [in default .ini file](https://github.com/1280px/CapsWitch/blob/master/CapsWitch.ini).*
