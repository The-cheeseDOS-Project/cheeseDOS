![csh logo](https://raw.githubusercontent.com/The-cheeseDOS-Project/storage/a66fa5efd6723d345f7f266410878973cf4f6376/logos/csh-logo.svg)

So I have made a scripting "language" for cheeseDOS, its called `csh` or **C**heese **SH**ell its very very simple, here's how to use it.

## Scripting

First make a file, we will call it `hello`, so to open the new file in the text editor `txt`, run:
```
txt hello
```

Then when you get into `txt` you can type out the script, here's how to make one.

So if i want to make a program that slowly prints out "Welcome to csh!" I would just put this in the csh file,
```
say Welcome
dly 500
say to
dly 500
say csh!
```

and that would do

Welcome
*waits 500ms*
to
*waits 500ms*
csh!

## Running the script
You can run any script by just typing `run` and in front of that put the filename like:
```
run hello
```
