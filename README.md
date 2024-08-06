| <img src="logo/logo.svg" alt="img" width="50px"> | <h1 style="margin: 0;">Just Build It</h1> |
|:-----------------------------------------------:|:---------------------------------------------------:|

jbi is a highly minimal and tiny build system written in C99 that takes the fustration and bloat that CMake or meson could bring to creating/managing a project. The entire project is around 700 lines and compiles into a 17kb executable.

jbi requires a `justbuild.it` file to be present in the root of a project and requires the following to inside it at a minimum

```
exe = projectname
cc = compiler
src = /path/to/src/ # will search through all subfolders if any
```
More configuration is optional, as shown below
```
include = /path/to/headers/ # will search through all subfolders if any
libs = sdl2
cflags = [compiler args]
ldflags = [compiler args]
pre = ./runbeforecomp
post = ./runaftercomp
```
When ready, simply run `jbi` or `jbi <path to root of project>` to compile your project. Run `jbi -c` to clean your project.

An example project using SDL2 is avalible in the example-project/ folder of this repository.

# Dependencies

Requires pkg-config installed to run. TCC is required to build.

# Building

Make sure you have tcc installed, and simply run `make`. To install onto your system, run `sudo make install`
