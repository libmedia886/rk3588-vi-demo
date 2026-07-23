# rk3588 libmedia VI Demo

This repository is a minimal caller-facing example for the rk3588 libmedia VI API.

It shows how to:

- read the current V4L2 capture format with `MEDIA_VI_QuerySource`
- print HDMI RX connection, timing, frame-rate, and audio status when the queried node identifies itself as HDMI RX
- reject non-NV12 current formats because this minimal example only allocates NV12 buffers
- create a buffer pool sized for NV12 camera frames
- configure `MEDIA_VI_ATTR`
- show the explicit `MEDIA_SYS_Bind` call shape for a downstream module without creating that module
- start and stop VI, acquire and release frames, and release the pool and media system

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

For a regular camera, `MEDIA_VI_QuerySource` reads the current V4L2 format but may not report frame rate; this example explicitly falls back to 30 fps when `fps_milli` is zero. HDMI RX nodes can additionally report connection state, DV timing-derived frame rate, and audio state.

The example keeps the VI side explicit. It only comments the explicit bind call because it does not create a downstream module. In a real application, VI can be bound to `RESIZE_RGA`, `VPSS`, `VENC`, `OSD`, or `VO`; every successful bind must have a matching explicit `MEDIA_SYS_UnBind` during teardown.
