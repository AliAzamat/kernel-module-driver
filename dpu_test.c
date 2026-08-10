// User-space exerciser for /dev/dpu0. Builds with plain gcc (it's userland) and
// drives the driver through its real syscall interface: write, ioctl, read.
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define DPU_IOC_MAGIC 'D'
#define DPU_GET_LEN   _IOR(DPU_IOC_MAGIC, 1, int)
#define DPU_CLEAR     _IO(DPU_IOC_MAGIC, 2)

int main(void) {
    int fd = open("/dev/dpu0", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "open /dev/dpu0: %s\n", strerror(errno));
        return 1;
    }

    // 1. write some data.
    const char *msg = "link up";
    if (write(fd, msg, strlen(msg)) < 0) { perror("write"); return 1; }

    // 2. ioctl: how many bytes are stored?
    int len = -1;
    if (ioctl(fd, DPU_GET_LEN, &len) == 0) {
        printf("DPU_GET_LEN -> %d\n", len);
    }

    // 3. read it back.
    char buf[64] = {0};
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        printf("read -> \"%s\"\n", buf);
    }

    // 4. ioctl: clear, then confirm length is 0.
    ioctl(fd, DPU_CLEAR, 0);
    ioctl(fd, DPU_GET_LEN, &len);
    printf("after CLEAR, len = %d\n", len);

    close(fd);
    return 0;
}
