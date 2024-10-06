
# jaylib

Shader toy built with raylib 

The "jay" stands for "J"

## Prerequisites

I don't even know what all I had to install.  Just run it, google the errors,
and `sudo apt install -y` until it works.

- linux (wsl is fine)
- ffmpeg
- opengl
- various x11 and related libraries
- cmake

Raylib at least is automatically fetched via cmake

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

To run the top-level source `main.c` (and `shader.glsl` and its other dependencies), set
`SRC_DIR` to `"."`:
```
set(SRC_DIR ".")
```

