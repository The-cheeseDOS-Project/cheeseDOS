# Contributing to cheeseDOS

## How Can I Contribute?

There are many ways to contribute, including:

- Reporting bugs and issues  
- Suggesting features or improvements  
- Writing and improving documentation  
- Submitting code patches or new features  
- Helping with testing and providing feedback
- Fixing code warnings (if any)

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
4.  ONLY `.ld`, `.S`, `.h`, `.c`, `.md` is allowed.
5.  Features must be mostly working before a PR.
6.  Use C99, no C11 or later.
7.  Uses 100% custom code and libraries (besides the toolchain)
8.  Only needs one command to compile (including installing dependencies)
9.  Compiles the same exact way on every system
10. Super simple to use
11. Uses Ring 0
 
## Reporting Issues
If you find bugs or have feature requests, please submit them in the GitHub Issues section. Include detailed descriptions and steps to reproduce the issue.

## Communication
For questions or discussions, you can open an issue.
