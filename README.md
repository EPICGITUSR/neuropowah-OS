# neuropowah-OS
a custom made os i built personally 
# NeuroPowah Meme OS 

A custom 32-bit Protected Mode operating system built from scratch for the memes. Run on hardware-accelerated emulation.

## Repository Contents

* `myos.iso` - The compiled, bootable operating system image.
* `kernel.c` - The core 32-bit kernel code written in C.

## How to Test and Run

To launch this OS inside a Linux environment with hardware acceleration, make sure you have QEMU installed and run:

```bash
qemu-system-i386 -enable-kvm -vga std -boot d -cdrom myos.iso
```

## License
Licensed under the GNU General Public License v3.0 (Copyleft). Turn your inner code out into the public!
