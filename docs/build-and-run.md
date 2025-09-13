# Build and Run

Here's how to get cheeseDOS up and running on your computer or on QEMU

> [!WARNING]
> On real hardware, cheeseDOS has direct access to low-level components and **may cause irreversible damage**. While no such incidents have occurred, this is experimental software—**USE AT YOUR OWN RISK!**

> [!IMPORTANT]
> Only 1.44MB floppy's with a CHS capable BIOS have a chance of working, use QEMU if you don't have those.

> [!NOTE]
> If you have any problems, please open an issue, you **WILL** get help!

## Prerequisites
You will need to be using `apt`, `dnf`, `zypper`, `pacman`, or `portage` based Linux distributions.

You also will need to have `git` and `bash` installed.

## Cloning
You will need to get the latest version of cheeseDOS: 
```bash
git clone https://github.com/The-cheeseDOS-Project/cheeseDOS.git && \
cd cheeseDOS
```

## Configuring

You need to configure cheeseDOS before compiling, you can just run:

```
./configure.sh
```

by itself and that will use the default options, but you can use diffrent options then the default one.

### Configure flags

Here are all the configure flags

- `--march=1` 
- `--bits= 1`
- `--optimize=`
- `--debug-info=`
- `--strip=`
- `--floppy=`

## Compilation
Then you want to compile cheeseDOS, this also will auto check and install (if not found) dependencies so if it asks for sudo that's why and this only should take <1 second to 5 seconds maximum to compile:
```bash
./build.sh
```

## Testing
After that'sone, you now have cheeseDOS compiled, if you just want to use it in QEMU, you can run:
```bash
./build.sh run
```

## Writing
Page moved over [here](./writing.md).

## Miscellaneous
For all build.sh arguments, refer to [this](./build.sh-arguments.md).
