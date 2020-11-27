# kbdcnt-kernel
Keyboard press counter kernel module

In order to use build and load the module into the kernel using following commands in module directory (sudo might be necessary):

    make
    sudo insmod ./kbdcnt

Then you can build the application in app directory and run it

    make
    sudo ./kbcntret
    sudo ./kbcntret -t
    sudo ./kbcntret -c
    sudo ./kbcntret -r

