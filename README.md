# CrystalGrowth
## Overview
![demo](docs/images/demo.gif)

CrystalGrowth is a simulation of dendritic crystal growth. Here, we implement the Kobayashi model<sup>[1](#footnote_1)</sup><sup>[2](#footnote_2)</sup> that introduces phase-field evolution for dendrite patterns. This simulation is rendered using <A href="https://github.com/frostsim/DXViewer">DXViewer</A>.

## Build
This repo was developed in the following environment:
* Windows 10 64-bit
* Microsoft Visual Studio 2019 on x64 platform (C++14)
* CMake 3.19.0
* DXViewer 3.1.0

You should update submodules before creating a project with cmake.

```bash
git submodule update --progress --init -- "ext/DXViewer"
```

## Gallery
![gallery1](docs/images/gallery1.jpg)|![gallery2](docs/images/gallery2.jpg)
:---:|:---:
![gallery3](docs/images/gallery3.jpg)|![gallery4](docs/images/gallery4.jpg)

## Reference
* <a name="footnote_1">[1]</a> Kobayashi, Ryo. "Modeling and numerical simulations of dendritic crystal growth." _Physica D: Nonlinear Phenomena_ 63.3-4 (1993): 410-423.
* <a name="footnote_2">[2]</a> Sanal, Rahul. "Numerical simulation of dendritic crystal growth using phase field method and investigating the effects of different physical parameter on the growth of the dendrite." _arXiv preprint arXiv:1412.3197_ (2014). (2014).

