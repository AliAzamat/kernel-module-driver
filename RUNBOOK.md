# dpu0 driver runbook

## Health
- `lsmod | grep dpu_chardev` — module loaded.
- `ls /sys/class/dpu/dpu0`   — device registered in the model.
- `dmesg | grep dpu0`        — register/remove lines and any -EFAULT.

## Common incidents
- **insmod "Invalid module format"**: built against a different kernel. Rebuild
  against the running kernel's headers (`uname -r`).
- **/dev/dpu0 missing after insmod**: udev didn't run, or class/device_create
  failed — check dmesg for the unwind path and a PTR_ERR code.
- **rmmod "Device or resource busy"**: a process holds /dev/dpu0 open. Find it
  with `fuser /dev/dpu0` and close it first.
- **A read hangs**: that's blocking I/O working — it's waiting for a write. Send
  data, or Ctrl-C (the read is interruptible).
- **The whole device freezes, every caller stuck**: a handler returned while
  holding `dpu_lock` (a missed unlock = deadlock). Audit the goto-out paths.

## Golden rule
NEVER `rmmod -f` a driver bound to live hardware — it can leave the device in an
undefined state. Quiesce first.
