Realtek Driver-based Card Reader
================================

Supported chips:
RTS5209
RTS5229

Contact Email:
pclinux@realsil.com.cn


Description
-----------

Realtek driver-based card reader supports access to many types of memory cards,
such as Memory Stick, Memory Stick Pro, Secure Digital and MultiMediaCard.


Helper Scripts
--------------

1. scripts/prebuild.sh
Update header files of your kernel header tree. This script only need to be
executed at the first time.

2. scripts/insert.sh
Insmod the modules

3. scripts/remove.sh
Rmmod the modules

4. scripts/install.sh
Install the modules to kernel module tree


Usage
-----

$ ./scripts/prebuild.sh    # Only once
$ make
$ ./scripts/install.sh

If you just test these modules temporarily, you can simply execute
scripts/insert.sh after make, and execute scripts/remove.sh to remove them.


About Debug
-----------

The default Makefile disables debug message. If you need it, please
modify the top-level Makefile and uncomment the below line:

#export CONFIG_CR_DEBUG = y

