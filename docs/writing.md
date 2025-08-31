If you want to run it on a real computer, you will need a High density 3.5" Floppy disk and a drive, make sure you have **NO** important **DATA** on it, if you do **BACK IT UP**, as **ALL** the data **WILL BE ERASED!**

To write it to a Floppy, run:
```bash
sudo dd if=cdos.img of=/path/to/fd bs=512 conv=notrunc status=progress
```
Change `/path/to/fd` with the path to your floppy drive, use `lsblk` or `sudo fdisk -l` to find that.
