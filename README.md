# CapsWitch 🧙‍♀️⬆️
A simple, lightweight (13/78 KB) and configurable program that switches keyboard layout with the CapsLock key — just put it in&nbsp;the&nbsp;Startup&nbsp;folder and enjoy!

> **Note**: For keyboard layout switching to work in programs running with elevated privileges, CapsWitch should be ran **with administrator privileges** as well. This can be done, for example, [using Task Scheduler](https://youtu.be/jt1Eb-NaBeg). <br>It runs fine **without privileges elevation** as well, however it will only work in non-elevated programs.


## Install it!
Download an archive with the preferred version:

+ [Windows 7–11 (x64)](https://github.com/1280px/CapsWitch/releases/download/m2.1/CapsWitch-64.zip)
+ [Windows XP + (x86)](https://github.com/1280px/CapsWitch/releases/download/m2.1/CapsWitch-XP.zip)

Then, upzip it wherever you want (e.g. in ```shell:startup``` or ```shell:common startup```, which are Startup&nbsp;folder locations for current&nbsp;user and all&nbsp;users respectively), double-click the .exe and you're done!

<details><summary><b><i>Building Information</i></b></summary>

### For modern systems
1. Use Microsoft Visual Studio **2010 or newer** (I'm using VS2022)
2. Compile for x64

### For legacy systems
1. Use Microsoft Visual Studio **2010 or newer** (I'm using VS2022)
2. Install Platform Toolset with XP compilation support (the latest version is ```v141_xp```)
3. Go to **Solution Explorer**, right click on CapsWitch and open **Properties**. Then, in ```PlatformToolset```, find the one you installed (e.g. ```Visual Studio 2017 - Windows XP (v141_xp)```) and select it.
4. In the same window, go to **C/C++ > Code Creation**, find ```Runtime Library``` and change it to ```/MT```
5. Apply and compile for x86
</details>


## Use it!
CapsWitch works by emulating a language switch keystroke *(Alt+Shift by default)* whenever CapsLock key is pressed. <br>The controls are:

+ **CapsLock** to change keyboard layout  
+ **LeftShift+CapsLock / CapsLock+LeftShift** to toggle Caps Lock state
+ **Alt+CapsLock** to toggle program activity *(disabled by default)*


## Configure it!
CapsWitch allows some customization using .ini settings file. By default, it looks for ```CapsWitch.ini``` file in the same directory, however you can pass custom file path using agruments (i.e. ```.\CapsWitch.exe "C:\test.ini"```). The program can also work without .ini file, using built-in default values.

*You can see more information about each option [in default .ini file](https://github.com/1280px/CapsWitch/blob/master/CapsWitch.ini).*
