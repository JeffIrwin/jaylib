
# jaylib

Shader toy built with raylib 

The "jay" stands for "J"

## Prerequisites

I don't even know what all I had to install.  Just run it, google the errors,
and `sudo apt install -y` until it works.

- linux (wsl is fine)
- ffmpeg (`4.4.2-0ubuntu0.22.04.1` works for me)
- opengl
- various x11 and related libraries
- cmake

Raylib is included from source as a submodule.  Either clone recursively or
update submodules after cloning.

ffmpeg is only needed to save videos.  You should still be able to render to
your screen by deleting any ffmpeg references in the code.

## Run

```
run.sh
```

### Running an archived shader

To run one of the archived shaders, set `SRC_DIR` in
[CMakeLists.txt](CMakeLists.txt):
```
set(SRC_DIR "./archive/2024-10-05/")
```
Either set this at the end of the block, or comment-out any re-setting of
`SRC_DIR` below.

To run the top-level source `main.c` (and the shader `*.glsl` and its other dependencies), set
`SRC_DIR` to `"."`:
```
set(SRC_DIR ".")
```

