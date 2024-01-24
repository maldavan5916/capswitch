# CapsWitch 🧙‍♀️⬆️
A simple and lightweight (13/78 KB) program that switches keyboard layout with the CapsLock key — just put it in&nbsp;the&nbsp;Startup&nbsp;folder and enjoy!

> **Note**:  Even though it's not necessary, for keyboard layout switching to work in programs running with administrator privileges, CapsWitch should be ran **with administrator privileges** as well. <br>This can be done, for example, [using Task Scheduler](https://youtu.be/jt1Eb-NaBeg). <br>It can also run **without privileges elevation**, however it will only work in other non-elevated programs.


## Install it!
Download an archive with the preferred version:

+ [Windows 7–11 (x64)](https://github.com/1280px/CapsWitch/releases/download/m2.0/CapsWitch-64.zip)
+ [Windows XP–? (x86)](https://github.com/1280px/CapsWitch/releases/download/m2.0/CapsWitch-XP.zip)

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
The program works by emulating specific language switch key combination *(Alt+Shift by default, can be configured in settings)* whenever CapsLock key is pressed. The controls are:

+ **CapsLock** to change keyboard layout  
+ **Shift+CapsLock / CapsLock+Shift** to toggle CapsLock state
+ **Alt+CapsLock** to enable/disable *(disabled by default, can be re-enabled in settings)*


## Configure it!
CapsWitch allows some customization using .ini settings file. By default, it looks for ```CapsWitch.ini``` file in the same directory, however you can pass custom file path using agruments (e.g. ```.\CapsWitch.exe "C:\test.ini"```). The program can also work without .ini file, using built-in default values (which are just **1** for every option).

*You can see more information about each option [in default .ini file](https://github.com/1280px/CapsWitch/blob/master/CapsWitch.ini).*
