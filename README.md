# Mathline

## Compiling from source

### Linux

The process for Linux is very straightforward.
The required packages are: `clang`, `raylib`, `make`, and a text editor.
Clone the project: `git clone https://github.com/kimizdevv/mathline`
Run `run.sh` to compile and run the game.

### Windows

Windows hates its developers.
It is recommended to install MSYS to avoid some of the hassle. It is a collection of tools and libraries gifted with a unix-like terminal. Then, you will install some packages to allow for compilation. Experience with command lines is recommended to not feel lost following the steps below.
1. Clone the project from GitHub into your system. GitHub Desktop is recommended if you do not know how to deal with commands.
2. Install MSYS2 into the root of your system drive (`C:\\msys64`): https://www.msys2.org/
3. After a successful installation a UCRT terminal should appear on the screen. Close it.
4. Navigate to the MSYS installation path and launch MINGW64.exe
5. Update the system: `pacman -Syuu`. **Keep running that command until it has no more upgrades to perform,** usually twice is enough.
6. From this point onwards, **make sure you are in the MINGW64 terminal.** Navigate to the project's directory. For instance: `cd /c/Users/user/mathline`
7. Run the setup script, which will take care of the rest (`./msyswinsetup.sh`)
8. You may now run `run.sh` to compile and run the game.

