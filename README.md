# JIT compiler
This is an educational project, aim is to to understand how JIT compilers work.

Function, provided here, is simple: it multiplies two numbers (255 and 4 by default). 

Architecture: x86-64.

## Supported commands
* `execute <arg1> <arg2>` - executes function with specified arguments, it's possible to specify both, one or none of them;
* `exit` - close program;
* `help` | `-help` | `--help` - print help message.

## Build
```
$ mkdir build
$ cd build
$ cmake ..
$ make
```
Requires C++14 compiler.

## Test
Tested manually on macOS Mojave 10.14.3 and Linux 4.12.

## Copyright
Pavel Ponomarev, 2019 (pavponn@gmail.com)

MIT License.
