# cheeseDOS
cheeseDOS is an x86, fully GNU GPLed, custom C99 written, 1.44MB, monolithic, live, Single Address Space Diskette Operating System that loads into RAM.

> [!NOTE]
> cheeseDOS isn't "DOS-like" — it follows its own philosophy. The "DOS" in the name simply means it's a **D**isk **O**perating **S**ystem (as in, it's stored on and boots from a disk… not a punch card).

> [!WARNING]
> On real hardware, cheeseDOS has direct access to low-level components and **may cause irreversible hardware damage**. While no such incidents have occurred, cheeseDOS comes **"AS IS" WITHOUT WARRANTY OF ANY KIND TO THE EXTENT PERMITTED BY APPLICABLE LAW**. See section 15, 16, and 17 in [LICENSE.md](./LICENSE.md) for more information.

> [!NOTE]
> If you have any problems, please open an issue, you **WILL** get help!

## Getting Started

You can download the precompiled image [here](https://github.com/The-cheeseDOS-Project/cheeseDOS/releases/latest). Then write it to a floppy, learn more [here](https://github.com/The-cheeseDOS-Project/docs/writing.md).

***OR***

You can build from source, refer to the [Build and Run Guide](https://github.com/The-cheeseDOS-Project/docs/build.md).

## System Requirements

| Component   | Requirement                                      |
|-------------|--------------------------------------------------|
| Processor   | i386-class or newer x86-compatible processor     |
| Storage     | 3.5" 1.44MB Floppy Diskette and a IDE Hard Drive |
| Graphics    | VGA-compatible graphics adapter                  |
| Display     | Color Monitor or Television (640×480 or better)  |
| Audio       | PC Speaker (Piezoelectric BIOS Beeper)           |
| Keyboard    | PS/2 or USB HID QWERTY keyboard                  |
| Firmware    | CHS-capable BIOS                                 |

## Philosophy

- Fully under the [GPLv3 License](./LICENSE.md)
- Fully compatible with i386-era hardware
- Uses 100% custom code and clib
- Compiles identically across all Linux systems
- Mostly made in C99
- Runs in a single address space

## Documentation

- **The cheeseDOS wiki**: [The cheeseDOS wiki](https://github.com/The-cheeseDOS-Project/docs)

## Project Guidelines

- **Contributing Guide**: [CONTRIBUTING.md](./CONTRIBUTING.md)
- **GPLv3 License**: [LICENSE.md](./LICENSE.md)
- **Security Policy**: [SECURITY.md](./SECURITY.md)
