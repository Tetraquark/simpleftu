# Simple File Transfer Utility

## Project status

At now work only on Linux systems.

Build as **DEBUG** mode (look *BuildConfig.h*).

## Dependency list

* pthread
* libconfig (apt-get install libconfig8-dev or yum install libconfig-devel.x86_64)

## How to run program

// There is help message. Run as: **./a.out -h** 

3 program modes:

* Server (arg: -s) - start server mode which listen default port (**10888**) and save input files into **./sftu_storage** dir (which must be created).
* Client (arg: -c) - start client mode.
* Daemon (arg: -d) - start daemon mode.