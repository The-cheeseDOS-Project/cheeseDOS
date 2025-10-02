# Contributing to cheeseDOS

## How Can I Contribute?

There are many ways to contribute, including:

- Reporting bugs and issues  
- Suggesting features or improvements  
- Writing and improving documentation  
- Submitting code patches or new features  
- Helping with testing and providing feedback
- Fixing code warnings (if any)
- Applying fixes from issues

## Getting Started

1. **Fork the Repository**  
   Click the "Fork" button on the top right to create your own copy of the repository.

2. **Clone Your Fork**  
   ```bash
   git clone https://github.com/The-cheeseDOS-Project/cheeseDOS.git
   cd cheeseDOS
   ```

3. **Create a Branch**
   Make a new branch:
   ```bash
   git checkout -b <feature>/<your-feature-name>
   ```

4. **Make Your Changes**
   Write code, fix bugs, or improve documentation.

5. **Commit Your Changes**
   Write clear, concise commit messages.
   ```bash
   git add .
   git commit -m "<Describe your changes>"
   ```

6. **Push Your Branch**
   ```bash
    git push origin feature/your-feature-name
    
7. **Open a Pull Request**
    Submit your changes by opening a pull request on GitHub.

## Coding Guidelines
1.  Follow the existing coding style and conventions in the project.
2.  Write clear, readable, and well-commented code.
3.  Code must work and be tested with `./build.sh run`.
4.  ONLY [.ld](https://en.wikipedia.org/wiki/Linker_(computing)#GNU), [.S](https://en.wikipedia.org/wiki/GNU_Assembler), [.h](https://en.wikipedia.org/wiki/Include_directive#C/C++), [.c](https://en.wikipedia.org/wiki/C_(programming_language)), [.md](https://en.wikipedia.org/wiki/Markdown#GitHub_Flavored_Markdown), [.json](https://en.wikipedia.org/wiki/JSON) is allowed.
5.  Features must be (mostly) working before a [PR](https://en.wikipedia.org/wiki/Distributed_version_control#Pull_requests).
6.  Use [C99](https://en.wikipedia.org/wiki/C99), no C11 or later.
7.  Uses 100% custom code and [libraries](https://en.wikipedia.org/wiki/Library_(computing)) (besides the [toolchain](https://en.wikipedia.org/wiki/Toolchain))
8.  Only needs one command to [compile](https://en.wikipedia.org/wiki/Command_(computing)) (including installing [dependencies](https://www.sonatype.com/resources/articles/what-are-software-dependencies))
9.  [Compiles](https://en.wikipedia.org/wiki/Compiler) the same exact way on every system
10. Super simple to use
11. Uses Ring 0
 
## Reporting Issues
If you find bugs or have feature requests, please submit them in the GitHub Issues section. Include detailed descriptions and steps to reproduce the issue.

## Communication
For questions or discussions, you can open an issue.
