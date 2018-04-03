# newbos

newbos is strictly a hobby kernel.

## Build
You will need to build a cross compiler. You can use the setup script on Ubuntu.
```
$ source setup.sh
```

Next you can build the kernel and run in an emulator
```
$ make
$ qemu-system-i386 -kernel newbos.bin
```

## Debugging Tips [FIXME: broken by high-half kernel commit b7ab2ae790c15f1f9ceceaee2881cdc25cee16be]
You can attach a debugger after setting up symbols and launching in freeze mode.
```
$ objcopy --only-keep-debug newbos.bin newbos.sym
$ qemu-system-i386 -kernel newbos.bin -s -S
```

Now you can attach gdb.
```
$ gdb
(gdb) target remote localhost:1234
(gdb) symbol-file newbos.bin
(gdb) break kernel_main
(gdb) continue
```
