# cheeseDOS

![Visitors](https://api.visitorbadge.io/api/daily?path=https%3A%2F%2Fgithub.com%2FThe-cheeseDOS-Project%2FcheeseDOS&countColor=%23263759?style=for-the-badge)
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

> [!NOTE]
> cheeseDOS isn't "DOS-like" — it follows its own philosophy. The "DOS" in the name simply means it's a **D**isk **O**perating **S**ystem (as in, it's stored on and boots from a disk… not a punch card).

> [!WARNING]
> On real hardware, cheeseDOS has direct access to low-level components and **may cause irreversible hardware damage**. While no such incidents have occurred, cheeseDOS comes **"AS IS" WITHOUT WARRANTY OF ANY KIND TO THE EXTENT PERMITTED BY APPLICABLE LAW**. See section 15 and 16 in [LICENSE](./LICENSE) for more information.

> [!NOTE]
> If you have any problems, please open an issue, you **WILL** get help!

## Getting Started

You can download the precompiled image [here](https://github.com/The-cheeseDOS-Project/cheeseDOS/releases/latest). Then write it to a floppy, learn more [here](https://github.com/The-cheeseDOS-Project/docs/writing.md).

***OR***

You can build from source, refer to the [Build and Run Guide](https://github.com/The-cheeseDOS-Project/docs/build.md).

## System Requirements

| Component   | Requirement                                     |
|-------------|-------------------------------------------------|
| Processor   | i386 class or newer x86-compatible processor    |
| Storage     | 3.5" 1.44MB Floppy Diskette on slot 0 or 1      |
| Graphics    | VGA 80x25 Graphics Adapter                      |
| Display     | Color Monitor or Television (640×400 or better) |
| Audio       | PC Speaker (Piezoelectric BIOS Beeper)          |
| Keyboard    | PS/2 or USB HID QWERTY keyboard                 |
| Firmware    | CHS-capable BIOS                                |

## Philosophy

- Fully under the [GPLv3 License](./LICENSE)
- Fully compatible with i386-era hardware
- Uses 100% custom code and clib
- Compiles identically across all Linux systems
- Mostly made in C99
- Runs in a single address space

## Documentation

- **The cheeseDOS wiki**: [The cheeseDOS wiki](https://github.com/The-cheeseDOS-Project/docs)

## Project Guidelines

- **Contributing Guide**: [CONTRIBUTING.md](./CONTRIBUTING.md)
- **GPLv3 License**: [LICENSE](./LICENSE)
- **Security Policy**: [SECURITY.md](./SECURITY.md)
