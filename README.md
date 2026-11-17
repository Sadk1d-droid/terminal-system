# terminal-system
# terminal

```text
  +------------------------+

  |   \                    |
  |    \                   |
  |     >                  |
  |    /                   |
  |   /   ________         |
  +------------------------+
         [ terminal ]
```

**terminal** is an independent, ultra-minimalist operating system built from scratch around the Linux kernel and a custom PID 1 `init` system written in pure C (~12.3 MB ISO).

It is **not** a Linux distribution (like Ubuntu or Arch), **not** a derivative, and **not** a guide or book — it is a standalone, self-contained system designed to boot directly into a fast, distraction-free command-line environment without bloatware, display managers, or heavy background services.

---

## Minimum System Requirements

* **Architecture:** `x86_64` (64-bit processor)
* **RAM:** 64 MB minimum (128 MB+ recommended; runs entirely in RAM)
* **Boot Media:** ISO image via USB (Ventoy, Rufus) or Virtual Machine (QEMU, VirtualBox, VMware)

---

## System Architecture & Features

* **Kernel:** Linux (`vmlinuz`) — monolithic kernel handling hardware, memory, and processes.
* **Custom PID 1 (`init.c`):** Written from scratch in pure C. Directly mounts `/proc`, `/sys`, and `/dev`, initializes networking, generates credentials, and spawns the shell.
* **Userland & Utilities:** Powered by a statically linked **BusyBox** binary providing core POSIX utilities in a single lightweight executable.
* **Shell:** `/bin/sh` with a custom green prompt (`root@terminal:~#`).
* **Package Manager:** None by design. **terminal** follows a zero-bloat philosophy — software is added by compiling static binaries and packing them directly into `rootfs` during the ISO build process.
* **Automatic Security:** Generates a fresh, cryptographically random 16-character `root` password from `/dev/urandom` on every boot.

---

## Quick Start & Built-in Commands

### Booting in QEMU
```sh
qemu-system-x86_64 -cdrom terminal.iso -m 512M
```

### Networking & Wi-Fi
At boot, **terminal** automatically attempts to configure Ethernet (`eth0`) via DHCP and prompts for Wi-Fi (`wlan0`) credentials. To connect to Wi-Fi manually later:
```sh
rfkill unblock wifi
ip link set wlan0 up
wpa_passphrase "YOUR_SSID" "YOUR_PASSWORD" > /etc/wpa.conf
wpa_supplicant -B -i wlan0 -c /etc/wpa.conf
udhcpc -i wlan0
echo "nameserver 8.8.8.8" > /etc/resolv.conf
```

### Essential Commands
* **Files:** `ls -la`, `cd`, `mkdir`, `rm -rf`, `cp`, `mv`, `cat`, `vi`
* **System:** `uname -a`, `ps`, `top`, `free -m`, `df -h`, `dmesg`
* **Network:** `ip a`, `ping 8.8.8.8`, `udhcpc -i <iface>`, `wget <url>`
* **Power:** `reboot`, `poweroff`

---

## Build from Source

To compile `init.c` and generate your own `terminal.iso` on a Debian/Ubuntu host:

```sh
# 1. Install build dependencies
sudo apt update
sudo apt install -y gcc busybox-static grub-pc-bin grub-efi-amd64-bin xorriso mtools cpio gzip linux-image-virtual

# 2. Prepare root filesystem and BusyBox symlinks
mkdir -p rootfs/bin rootfs/sbin rootfs/etc rootfs/proc rootfs/sys rootfs/dev rootfs/root
cp /bin/busybox rootfs/bin/busybox
for cmd in \$(/bin/busybox --list); do ln -sf /bin/busybox rootfs/bin/\$cmd; done

# 3. Statically compile init.c as PID 1
gcc -static src/init.c -o rootfs/init
chmod +x rootfs/init

# 4. Pack initramfs
(cd rootfs && find . | cpio -H newc -o | gzip -9 > ../initrd.img)

# 5. Assemble ISO structure and generate image
mkdir -p iso_root/boot/grub
sudo cp /boot/vmlinuz-* iso_root/boot/vmlinuz
cp initrd.img iso_root/boot/initrd.img

cat << 'EOF' > iso_root/boot/grub/grub.cfg
set timeout=3
set default=0

menuentry "terminal" {
    linux /boot/vmlinuz quiet
    initrd /boot/initrd.img
}
EOF

grub-mkrescue -o terminal.iso iso_root/
```

---

## Open Source & License

**terminal** is a peaceful, free, and open-source project released into the public domain under **The Unlicense**. Anyone is free to read, copy, modify, build, and distribute the source code without restrictions or prior permission.

> **Disclaimer:** This software is provided "as is", without warranty of any kind. It is recommended to test `terminal.iso` in a virtual machine before booting on bare-metal hardware.

