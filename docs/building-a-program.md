# Building a Program
> [!IMPORTANT]
> This guide is **W.I.P.!**

## 1.0.) Introduction
So you want to build a program? If so, then you are at the right place!

Building a program on cheeseDOS is a great way to learn low-level programming in C. Be aware that cheeseDOS is non-standard. You will only learn the basics of low-level programming.

## 2.0.) Naming your program
First of all, to build your own program you need to name your program.

### 2.1.) Explaining your program in one word
First step in naming your program for cheeseDOS is explaining what your program does in one word, For example:

| Type                                                         | Word      |
|--------------------------------------------------------------|-----------|
| A program that lets you paint a picture                      | Paint     |
| A program that lets you print out a picture into the console | Photo     |
| A program that lets you see raw keyboard scancodes           | Scancodes |

You want them to be direct.

For this demo, we will use Scancodes.

### 2.2) Shortening the name to three or fewer letters long
For simplicity, cheeseDOS has all the commands three or fewer letters long. Commands **CANNOT** have numbers or special characters, only alphabetical. Here are some examples:

| Word      | Command |
|-----------|---------|
| Paint     | `pnt`   |
| Photo     | `pto`   |
| Scancodes | `scn`   |

That will turn our command from Scancodes to `scn`.

Now that we have our name we can move onto the next step.

## 3.0.) Building cheeseDOS
If you already have the source cheeseDOS, you can safely skip this step, for those who have not, please do not skip this.

### 3.1.) Prerequisites
You will need a working GNU toolchain, `git`, and standard UNIX utilities.

### 3.2.) Cloning
You will need to get the latest version of cheeseDOS: 
```sh
git clone --depth=1 https://github.com/The-cheeseDOS-Project/cheeseDOS.git && \
cd cheeseDOS
```

### 3.3.) Configuring
You need to configure cheeseDOS before compiling, you can just run:

```sh
./configure.sh
```

by itself and that will use the default options (`i386`, `32`, `s`, `cheeseDOS.img`, `sudo`.), but you can use different options than the default one.

#### 3.3.1.) Configure flags

| Flag          | Purpose                          | Applicable values               |
|---------------|----------------------------------|---------------------------------|
| `--march=`    | Target CPU architecture          | Varies                          |
| `--bits=`     | Target bit width                 | `32`, `64`                      |
| `--optimize=` | Optimization level               | `0`, `1`, `2`, `3`, `s`, `fast` |
| `--floppy=`   | Target filename                  | Any                             |
| `--sudo=`     | **S**uper**U**ser **DO** command | `sudo`, `doas`                  |

### 3.4.) Compilation
Then you want to compile cheeseDOS:
```sh
./build.sh
```

## 4.0.) Registering your soon-to-be-made program
For the shell to find your program you will need to register it into cheeseDOS.

### 4.1.) programs.c
#### 4.1.1.) Opening in nano
We will need to open `programs.c`. I will assume you are using the nano text editor and that you are in the root of the cheeseDOS folder:
```sh
nano src/programs/programs.c
```
You should now see some source code in C.

#### 4.1.2.) Adding your soon-to-be-made header file
You should see a bunch of `#include`s, go to the bottom of the includes and put
```
#include "name.h"
```
Replace `name` with your program.

It should look something like this:
```
#include "scn.h"
```

#### 4.1.3.) Adding it to the array
Adding your program to the array is what makes cheeseDOS find it.

Scroll down and you should see:
```
static shell_command_t commands[] = {
    {"hlp", hlp},
    {"ver", ver},
    {"hi", hi},
    .............
    {"snk", snk},
    {"key", key},
    {"rep", rep},
    {NULL, NULL}
};
```

You want to add your program to the bottom of it (just before `{NULL, NULL}`) just add:
```
{"name", name},
```
Replace `name` with your program's name

So it should look something like:
```
static shell_command_t commands[] = {
    {"hlp", hlp},
    {"ver", ver},
    {"hi", hi},
    .............
    {"key", key},
    {"rep", rep},
    {"scn", scn},
    {NULL, NULL}
};
```
You now have your program registered in the shell, do not try to compile yet as it will fail due to it not existing yet.

You may now close nano with Ctrl + X

### 4.3.) Registering in hlp
`hlp` is a program that lists all programs

#### 4.3.1.) Opening in nano
```
nano src/programs/hlp/hlp.c
```
You should now see source code in C

### 4.3.2.) Adding it
There will be a line that has the `print()` function on it
```
print("Commands: hlp, cls, say, ... snk, key, rep.\n");
```

You want to add your program at the end of it so it looks something like this:
```
print("Commands: hlp, cls, say, ... key, rep, scn.\n");
```

You now may press Ctrl + X to exit nano.

### 5.0.) Making the program directory
You now will need to make the directory that will store your program's source files:
```
mkdir src/programs/scn
```

Make sure to use your own name.

#### 5.0.1.) Change directory into the newly made directory
We will do this so we don't have to keep using such long paths:
```
cd src/programs/scn
```

Make sure to use the path from when you made it.

### 6.0.) Making the header file
You need to make your header file so the shell knows where to look:
```
nano scn.h
```
Remember to use your own name.

#### 6.1.) Occupying the header file
Here is a sample header file:
