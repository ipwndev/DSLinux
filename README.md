DSLinux-Mirror
==============

UClinux for the DS console.  Mirror of the SVN.
Compiling instructions:
Operating System

DSLinux can be compiled on any modern Linux distribution.

DSLinux can also be built on FreeBSD, NetBSD, and OpenBSD. See this page for details.

If you are running Windows on your box you can try LiveDVDandCDs

If you are running Windows on your box you can get the VMWare Player and a preconfigured Debian or Ubuntu image to do the build. Note that the build may take quite a long time this way! The VMWare Player can be downloaded at http://www.vmware.com/products/player/ and a Ubuntu appliance can be found at http://www.vmware.com/appliances/.

Tools and Libraries

You must install all of the tools and libraries mentioned below before you start to compile DSLinux.

You need the following standard development tools installed:

make (GNU make)
gcc (C compiler)
binutils (assembler and linker)
bison (parser generator)
They should be part of any major Linux distribution, and are often installed by default.

You also need ncurses development libraries on your system. They should be packaged with any major Linux distribution. The package is usually called either of

ncurses-dev
ncurses-devel
libncurses-dev
libncurses-devel
or something similar. It is called libncurses5-dev on Debian.

You also need zlib development libraries on your system. This should also be packaged with your distribution, and should be called

zlib-dev
zlib-devel
or something similar. It is called zlib1g-dev on Debian.

You also need gettext, more exactly the msgfmt utility.

Some other packages that may be required are

libperl-dev
texinfo
flex
this what they are called on Debian.

Also, make sure your system uses the GNU implementation of awk (commonly called gawk).

If you are going to compile applications that use libncurses in dslinux you may need to install mawk as it might not be a part of the awk package (depending on linux distribution)

Getting the DSLinux Source Tree

The DSLinux sources are stored in a Subversion repository hosted by IN-Berlin.

DSLinux contains cryptographic software. You may therefore not be allowed to re-distribute it to other countries, depending on the country you are living in and the country you want to distribute to. See http://rechten.uvt.nl/koops/cryptolaw/ for information about export restrictions in your country. Since the DSLinux source code is now hosted in Germany, no export restrictions currently apply to the main repository.

Getting the source tree for the first time

To get the DSLinux source tree from anonymous SVN run the following command:

  svn checkout http://svn.dslinux.org/svn/trunk dslinux
Updating your source tree

To update the DSLinux sources from anonymous SVN, simply run:


 cd dslinux
 svn update
Getting the toolchain

Downloading a pre-built toolchain

Note to AMD-64 Users, download the i386/i686 toolchain, or build a toolchain using i386 tools. The AMD-64 tools will segfault during compile.

Also note that the toolchain currently will not compile with GCC 4.3. Use a precompiled toolchain if this will affect you (if you are running Fedora 9, for example)

You can download pre-built toolchains for several architectures running Linux here or here. Toolchain distribution archives are named like this:


 dslinux-toolchain-<version>-<architecture>.tbz
where <version> is the version of the toolchain and <architecture> is the architecture it was built for. If you are not sure what architecture you have, run the command


 uname -m
to find out.

Note that the toolchain is currently still a bit in development, and there is no stable release of it yet. For this reason, version numbers are still just date stamps.

For example, let's say you've downloaded a toolchain archive called dslinux-toolchain-2006-11-04-i686.tbz. Extract the toolchain into /usr/local. Since /usr/local is usually only writable by root, you may need to run this command as root user.


 tar -C /usr/local -jxf dslinux-toolchain-2006-11-04-i686.tbz
Extract the toolchain somewhere else if you don't have root on the machine you are working on:


 tar -C /some/where/else -jxf dslinux-toolchain-2006-11-04-i686.tbz
Now make sure your user (not root) has the toolchain binaries in PATH:


  export PATH=/usr/local/dslinux-toolchain-2006-11-04-i686/bin:$PATH
You can put this line into the file ~/.bashrc to make the setting permanent. Of course, if you didn't extract the toolchain into /usr/local, you need to adjust the PATH accordingly.

Compiling the toolchain

If you want to (or have to) build the toolchain yourself, see the file


 toolchain/README
in the dslinux source tree for instructions. Please note that as the toolchain is currently in the state of development it may not build correctly. If this is the case please use the prebuilt toolchain.

Configuring DSLinux

Starting the configuration menu

Run the following commands:


  cd dslinux
  make menuconfig
You can navigate the menu with the arrow keys. Press the tab key to select an action from the bottom of the screen (Select, Exit or Help). Press return to perform an action.

Selecting the build type

Press return on this entry to enter the build type selection menu:


 DSLinux build type selection  --->
Follow the on-screen instructions. At present only the DLDI and NDS builds are being actively developed.

Customizing DSLinux

Note: Please try and build with the defaults FIRST before messing around with configuration options Also please bear in mind that all the applications listed in the menus are not available due to DSLinux's build system being based on uClinux.

Customizing the kernel

If you want to customize your kernel settings (not recommended unless you know what you are doing), select:


 Kernel/Library/Defaults Selection ---> [*] Customize Kernel Settings
and exit the configuration menu. Save the configuration when you are asked if you want to save it. Another menu will pop up where you can configure the kernel.

Configuring a larger font

There are three fonts available. The smallest one is 4x6, and it is the default. A bigger one is 4x9. It does not sacrifice display width. The largest usable font is 6x6. It sacrifices both display width and height.

In the kernel configuration menu, select one of the following entries:

For the 4x9 font:


  Device Drivers  --->
    Graphics support  --->
       Console display driver support  ---> [*]   Mini 4x9 font
For the 6x6 font:


  Device Drivers  --->
    Graphics support  --->
       Console display driver support  ---> [*]   Mini 6x6 font
Also, make sure you disable the Mini 4x6 font, which is enabled by default:


       Console display driver support  ---> [ ]   Mini 4x6 font
Swapping the L and R hardware buttons

By default the L button is shift and the R button is control. If you are left handed or your L button is broken it is more convenient the other way around. To swap them, set the following in the kernel configuration menu:


  System Type  --->
    Nintendo DS Options  --->
      [*] Swap L and R buttons
Resetting to default configuration

If you would like to go back to the default configuration, you can do so from the menu by selecting the option


 Kernel/Library/Defaults Selection ---> [*] Default all settings (lose changes)
and saving the configuration while exiting.

Adding another virtual terminal

This information is now redundant as DSLinux will automatically activate 3 virtual terminals if extra RAM is detected

You can add another virtual terminal (VT) to DSLinux. This means you can run two applications in different windows at the same time. Note that a second VT uses a bit of memory. Thus applications are more likely to crash with out of memory errors if you enable this!

Edit this file before compiling:


 vendors/Nintendo/<build>/inittab
where <build> is the build type you are using (for example, GBAMP). The file usually only contains one line:


 tty1::linux:/usr/bin/agetty 38400 tty1
To add another VT, add this line below the first:


 tty2::linux:/usr/bin/agetty 38400 tty2
To switch to the second VT, press Alt and then F2. To switch back to the first VT, press Alt and then F1.

Compiling DSLinux

Now run


   make
and go keep yourself busy with something else for a while. When the compile is done, you will find installation files in the dslinux/images directory. Unpack the tarball onto the root of your media card, including all directories inside the tarball.
