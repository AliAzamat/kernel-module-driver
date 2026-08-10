#ifndef DPU_STATE_H
#define DPU_STATE_H

#include <linux/types.h>

#define DPU_BUF_LEN 256

// Per-device state. In a real driver each device instance owns one of these,
// allocated on the kernel heap — not a single global buffer.
struct dpu_state {
    char   buf[DPU_BUF_LEN];
    size_t buf_len;
    int    major;
};

#endif // DPU_STATE_H
