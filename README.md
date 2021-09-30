Free-form area light rendering
===

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br/>

[![Windows CI](https://github.com/Paul180297/BezierLightLTC/actions/workflows/windows.yaml/badge.svg)](https://github.com/Paul180297/BezierLightLTC/actions/workflows/windows.yaml)
[![MacOS CI](https://github.com/Paul180297/BezierLightLTC/actions/workflows/macos.yaml/badge.svg)](https://github.com/Paul180297/BezierLightLTC/actions/workflows/macos.yaml)
[![Ubuntu CI](https://github.com/Paul180297/BezierLightLTC/actions/workflows/ubuntu.yaml/badge.svg)](https://github.com/Paul180297/BezierLightLTC/actions/workflows/ubuntu.yaml)

This is an official implementation of the paper, 

> Kuge et al., "Real-Time Shading of Free-Form Area Lights using Linearly Transformed Cosines," Journal of Computer Graphics Techniques, No. 4, Vol. 10, 2021.

Please see the [JCGT paper](#) and [our website](https://tatsy.github.io/projects/kuge2021bezlight/) for more details.

### Requirements

* OpenGL 4.3 or higher
* GLFW3
* GLM

### Build

```shell
git clone https://github.com/Paul180297/BezierLightLTC.git
cd BezierLightLTC
mkdir build && cd build
cmake .. -D CMAKE_BUILD_TYPE=Release
cmake --build . --config Release

```

### Run

```shell
# From project root
./build/bin/bezier_ltc
```

### Screen shot

<img src="images/demo01.png" alt="demo 01" style="width:80%; max-width:512;"/>

### Reference

```bibtex
@article{kuge2021bezlight,
  author={Kuge, Takahiro and Yatagawa, Tatsuya and Morishima, Shigeo},
  title={Real-time Shading with Free-form Planar Area Lights using Linearly Transformed Cosines},
  journal={Journal of Computer Graphics Techniques},
  number={4},
  volume={10},
  page={1--16},
  year={2021}
}

```

### License

[CC BY-NC-SA 4.0](http://creativecommons.org/licenses/by-nc-sa/4.0/), 2021 (c) Takahiro Kuge and Tatsuya Yatagawa

This project has some third-party dependencies, each of which may have independent licensing:

* [Dear ImGui](https://github.com/ocornut/imgui) - Bloat-free Graphical User interface for C++ with minimal dependencies
* [stb](https://github.com/nothings/stb): single-file public domain libraries for C/C++
* [glad](https://github.com/Dav1dde/glad) - Multi-Language Vulkan/GL/GLES/EGL/GLX/WGL Loader-Generator based on the official specs.
* [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) - Tiny but powerful single file wavefront obj loader 
