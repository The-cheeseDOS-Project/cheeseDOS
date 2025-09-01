![cdos-logo](./images/cdos-logo.svg)

## Thank you for visiting the cheeseDOS GitHub page!

> [!NOTE]
> cheeseDOS isn't "DOS-like" — it follows its own philosophy. The "DOS" in the name simply means it's a **D**isk **O**perating **S**ystem (as in, it's stored on and boots from a disk… not a punch card).

![Lines of Code](https://tokei.rs/b1/github/The-cheeseDOS-Project/cheeseDOS?style=for-the-badge)
[![Repo Size](https://img.shields.io/github/repo-size/The-cheeseDOS-Project/cheeseDOS?style=for-the-badge)](https://github.com/The-cheeseDOS-Project/cheeseDOS)
[![Last Commit](https://img.shields.io/github/last-commit/The-cheeseDOS-Project/cheeseDOS?style=for-the-badge)](https://github.com/The-cheeseDOS-Project/cheeseDOS/commits/main)
[![Open Issues](https://img.shields.io/github/issues/The-cheeseDOS-Project/cheeseDOS?style=for-the-badge)](https://github.com/The-cheeseDOS-Project/cheeseDOS/issues)
[![Open PRs](https://img.shields.io/github/issues-pr/The-cheeseDOS-Project/cheeseDOS?style=for-the-badge)](https://github.com/The-cheeseDOS-Project/cheeseDOS/pulls)
[![Contributors](https://img.shields.io/github/contributors/The-cheeseDOS-Project/cheeseDOS?style=for-the-badge)](https://github.com/The-cheeseDOS-Project/cheeseDOS/graphs/contributors)
[![Stargazers](https://img.shields.io/github/stars/The-cheeseDOS-Project/cheeseDOS.svg?style=for-the-badge)](https://github.com/The-cheeseDOS-Project/cheeseDOS/stargazers)
[![Forks](https://img.shields.io/github/forks/The-cheeseDOS-Project/cheeseDOS.svg?style=for-the-badge)](https://github.com/The-cheeseDOS-Project/cheeseDOS/network/members)
[![Build](https://img.shields.io/github/actions/workflow/status/The-cheeseDOS-Project/cheeseDOS/release.yml?style=for-the-badge)](https://github.com/The-cheeseDOS-Project/cheeseDOS/actions)

cheeseDOS is an x86, fully GNU GPLed, custom C99 written, 1.44MB, monolithic, live, Single Address Space Diskette Operating System that loads into RAM.

![`ban` command](./images/ban-command.png)

> [!WARNING]
> On real hardware, cheeseDOS has direct access to low-level components and **may cause irreversible damage**. While no such incidents have occurred, this is experimental software—**USE AT YOUR OWN RISK!**

> [!IMPORTANT]
> I have **only** gotten this to **work** with the **floppy image** on **QEMU**, when I tried this on my **real computer**, it **froze** and **rebooted** right **after booting**.

> [!NOTE]
> If you have any problems, please open an issue, you **WILL** get help!

## Getting Started

You can download the precompiled image [here](https://github.com/The-cheeseDOS-Project/cheeseDOS/releases/latest). Then write it to a floppy, learn more [here](./docs/writing.md).

## System Requirements

| Component   | Requirement                                                         |
|-------------|---------------------------------------------------------------------|
| Processor   | i386 class or newer x86-compatible processor                        |
| Storage     | 3.5" 1.44MB Floppy Diskette on slot 0 or 1 (nothing else will work) |
| Memory      | 96KB RAM or better (640KB is all cDOS can use)                      |
| Graphics    | VGA 80x25 Graphics Adapter                                          |
| Display     | Color Monitor or Television (640×400 or better)                     |
| Audio       | PC Speaker (Piezoelectric BIOS Beeper)                              |
| Keyboard    | PS/2 or USB HID QWERTY keyboard                                     |
| Firmware    | CHS-capable BIOS                                                    |

## Philosophy

- Fully under the GNU GPL v3
- Works on vintage hardware
- Runs as fast as possible
- Uses 100% custom code and libraries (besides the toolchain)
- Only needs one command to compile (including installing dependencies)
- Compiles the same exact way on every system
- Always has a precompiled image of its latest version
- Super simple to use
- Uses C99
- Uses Ring 0
- Super simple to use

## Building

For instructions on building from source, refer to the [Build and Run Guide](./docs/build-and-run.md).

## Documentation

- **The cheeseDOS wiki**: [The cheeseDOS wiki](./docs)

## Project Guidelines

- **Contributing Guide**: [CONTRIBUTING.md](./CONTRIBUTING.md)
- **GPL v3 License**: [LICENSE](./LICENSE)
- **Security Policy**: [SECURITY.md](./SECURITY.md)

### Shoutouts
- Shout out to [rkapl](https://github.com/rkapl) for cheeseLDR!
- Shout out to [anchored-archer](https://github.com/anchored-archer) for just being a cool person!
- Shout out to [ImNotJahan](https://github.com/ImNotJahan) for fixing a typo!
- Shout out to [m4kyu](https://github.com/m4kyu) for adding `pix`!
- and Shout out to **YOU!** for visiting the cheeseDOS GitHub page!

<!--
#### Here's a random word I made up
##### Pentoims (noun) 
###### Pronounced: /ˈpen-ti-oms/

##### Meaning

- random dandified intel pentuim CPU

##### Usage

- My **pentoim** is so fast and overclocked.
-->
