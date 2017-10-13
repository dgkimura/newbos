# newbos

## Build
You will need to build a cross compiler. You can use the setup script on Ubuntu.
```
$ sh setup.sh
```

Next you can build the kernel and run in an emulator
```
$ make
$ qemu-system-i386 -kernel newbos.bin
```
