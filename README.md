# rk3588 libmedia VI Demo

This repository is a minimal caller-facing example for the rk3588 libmedia VI API.

It shows how to:

- query a V4L2 camera source with `MEDIA_VI_QuerySource`
- create a buffer pool sized for NV12 camera frames
- configure `MEDIA_VI_ATTR`
- bind VI output to a downstream module
- start, stop, unbind, and release resources in order

The repository intentionally does not include `libmedia.so`, `libmedia.a`, license files, or generated media.

## Build

```bash
cmake -S . -B build -DLIBMEDIA_SDK=/path/to/rk3588-libmedia-sdk
cmake --build build
```

## Run

```bash
./build/rk3588_vi_demo /dev/video-camera0
```

The example keeps the VI side explicit. In a real application, the VI output can be bound to `RESIZE_RGA`, `VPSS`, `VENC`, `OSD`, `VO`, or an application-owned frame loop.

