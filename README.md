# 
Bing wallpaper of the day application for Gnome desktop

#  - Little utility to browse/download/setting bing's wallpaper of the day to Gnome desktop

by [@keshavbhatt](https://github.com/keshavbhatt) of [ktechpit.com](http://ktechpit.com) and [others]
(https://github.com/keshavbhatt//graphs/contributors)
﻿

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

﻿**Nightly and stable Build on any [snapd](https://docs.snapcraft.io/installing-snapd) enabled Linux Distribution can be installed using:**

﻿[![bing-wall](https://snapcraft.io//bing-wall/badge.svg)](https://snapcraft.io/bing-wall) [![bing-wall](https://snapcraft.io//bing-wall/trending.svg?name=0)](https://snapcraft.io/bing-wall)

[![Get it from the Snap Store](https://snapcraft.io/static/images/badges/en/snap-store-black.svg)](https://snapcraft.io/bing-wall)

    snap install bing-wall
    
[AUR for Arch based distributions](https://aur.archlinux.org/packages/bing-wall)    

**Features**
-   Allows list bing wallpaper of the day for 10 Countries (All supported by bing API).
-   Simple to use neat interface.
-   Allow Downloading of wallpapers and saving them to a browsable location.
-   Downloaded wallpapers are accessible from with the application.
-   Set wallpaper of the day as you LogIn to your system.
-   Option to show Info Watermark on wallpaper.
-   Dark Theme.

**Command Line Mode:**

Set wallpaper of the day without opening the GUI with single command. `bing-wall --set`


﻿**Consider Donating if you want this music player grow further**

[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://paypal.me/keshavnrj/5)

[![Buy Me A Coffee](https://bmc-cdn.nyc3.digitaloceanspaces.com/BMC-button-images/custom_images/orange_img.png)](https://www.buymeacoffee.com/keshavnrj)

** utilises power of the following tools and technologies :**
- Qt GUI Framework 5.5.1 
- Gnome Desktop 'gsettings'
- Qt
- C++11
- JSON 
- peapix.com API
- snapcraft.io/build for continuous build and delivery through snapcraft.io/store for all major Linux distributions supporting [snapd](https://snapcraft.io/docs/installing-snapd)

﻿**Build requirement**

    Qt >=5.5.1
    
**Build instructions**
With all build requirements in place go to project root and execute:

Build:

    qmake (or qmake-qt5, depending on your distro)
    make
    
Execute :

    ./bing-wall
     
﻿
﻿**Or build a snap package**
Copy snap directory from project root and paste it somewhere else (so the build will not mess with source code)
Run :

    snapcraft
Try snap with :

    snap try
Install snap with

    snap install --dangerous name_of_snap_file

**ScreenShots:** (could be old)

![BingWall for Linux Desktop](https://github.com/keshavbhatt/BingWall/blob/master/screenshots/1.png?raw=true)
![BingWall for Linux Desktop](https://github.com/keshavbhatt/BingWall/blob/master/screenshots/2.png?raw=true)
![BingWall for Linux Desktop](https://github.com/keshavbhatt/BingWall/blob/master/screenshots/3.png?raw=true)
