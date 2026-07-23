#include "media_api.h"

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VI_DEV 0
#define VI_POOL_ID 2
#define DEFAULT_CAMERA "/dev/video-camera0"

static volatile sig_atomic_t g_running = 1;

static void on_signal(int sig) {
    (void)sig;
    g_running = 0;
}

static int align_up(int value, int align) {
    return (value + align - 1) & ~(align - 1);
}

static int fill_vi_attr(const char *device, MEDIA_VI_ATTR *attr, size_t *frame_size) {
    MEDIA_VI_SOURCE_INFO info;
    int query_ok;
    memset(&info, 0, sizeof(info));
    memset(attr, 0, sizeof(*attr));

    query_ok = MEDIA_VI_QuerySource(device, &info) == 0 &&
               info.width > 0 && info.height > 0;
    if (!query_ok) {
        fprintf(stderr,
                "Could not read a valid V4L2 current format; "
                "using the known demo fallback 3840x2160 NV12 at 30 fps\n");
        info.width = 3840;
        info.height = 2160;
        info.stride = 3840;
        info.fps_milli = 30000;
        info.format = MEDIA_FORMAT_NV12;
    } else {
        printf("V4L2 current format: device=%s %dx%d stride=%d fourcc=%s media_format=%d\n",
               device, info.width, info.height, info.stride,
               info.fourcc_name[0] ? info.fourcc_name : "unknown", info.format);

        if (info.fps_milli <= 0) {
            printf("Camera FPS is not reported by MEDIA_VI_QuerySource; "
                   "this demo falls back to 30 fps\n");
        }

        if (info.format != MEDIA_FORMAT_NV12) {
            fflush(stdout);
            fprintf(stderr,
                    "Unsupported V4L2 current format %s (media_format=%d): "
                    "this demo supports NV12 only\n",
                    info.fourcc_name[0] ? info.fourcc_name : "unknown", info.format);
            return -1;
        }
    }

    attr->device = device;
    attr->width = info.width;
    attr->height = info.height;
    attr->stride = align_up(info.stride > 0 ? info.stride : info.width, 16);
    attr->fps = info.fps_milli > 0 ? info.fps_milli / 1000 : 30;
    attr->buf_cnt = 4;
    attr->pool_id = VI_POOL_ID;
    attr->format = MEDIA_FORMAT_NV12;

    *frame_size = (size_t)attr->stride * (size_t)attr->height * 3u / 2u;
    return 0;
}

int main(int argc, char **argv) {
    const char *device = argc > 1 ? argv[1] : DEFAULT_CAMERA;
    MEDIA_VI_ATTR vi;
    size_t frame_size = 0;
    int pool_created = 0;
    int vi_attr_set = 0;
    int vi_enabled = 0;

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    if (MEDIA_SYS_Init() != 0) {
        fprintf(stderr, "MEDIA_SYS_Init failed\n");
        return 1;
    }

    if (fill_vi_attr(device, &vi, &frame_size) != 0) goto cleanup;

    if (MEDIA_POOL_Create(VI_POOL_ID, frame_size, vi.buf_cnt + 2) != 0) {
        fprintf(stderr, "MEDIA_POOL_Create failed\n");
        goto cleanup;
    }
    pool_created = 1;

    if (MEDIA_VI_SetAttr(VI_DEV, &vi) != 0) {
        fprintf(stderr, "MEDIA_VI_SetAttr failed\n");
        goto cleanup;
    }
    vi_attr_set = 1;

    /*
     * Bind VI output to a real downstream module in production:
     * MEDIA_SYS_Bind("VI", VI_DEV, "output0", "RESIZE_RGA", 0, "input0");
     */

    if (MEDIA_VI_Enable(VI_DEV) != 0) {
        fprintf(stderr, "MEDIA_VI_Enable failed\n");
        goto cleanup;
    }
    vi_enabled = 1;

    printf("VI enabled: %s %dx%d stride=%d fps=%d\n",
           device, vi.width, vi.height, vi.stride, vi.fps);

    while (g_running) {
        MEDIA_BUFFER frame = {-1, -1};
        if (MEDIA_VI_GetFrame(VI_DEV, &frame, 1000) == 0) {
            printf("captured frame pool=%d index=%d\n", frame.pool_id, frame.index);
            MEDIA_VI_ReleaseFrame(VI_DEV, frame);
        }
    }

cleanup:
    if (vi_enabled) MEDIA_VI_Disable(VI_DEV);
    if (vi_attr_set) {
        /* No destroy call is required for VI; disabling stops capture. */
    }
    if (pool_created) MEDIA_POOL_Destroy(VI_POOL_ID);
    MEDIA_SYS_Exit();
    return vi_enabled ? 0 : 1;
}
