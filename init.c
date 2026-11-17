#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

int main() {
    // 1. mount core/kernel
    mount("proc", "/proc", "proc", 0, "");
    mount("sysfs", "/sys", "sysfs", 0, "");
    mount("devtmpfs", "/dev", "devtmpfs", 0, "");

    // 2. generation random password root from /dev/urandom
    const char charset[] = "abcdefghijklmnopqrstuvwxyz"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "0123456789!@#$%^&*_-";
    int password_length = 16;
    char password[17];

    FILE *urandom = fopen("/dev/urandom", "rb");
    if (urandom == NULL) {
        perror("coverage error /dev/urandom");
        return 1;
    }

    for (int i = 0; i < password_length; i++) {
        unsigned char random_byte;
        fread(&random_byte, 1, 1, urandom);
        password[i] = charset[random_byte % (sizeof(charset) - 1)];
    }
    password[password_length] = '\0';
    fclose(urandom);

    // 3.(Ethernet)
    system("ip link set eth0 up");
    sleep(2);
    system("udhcpc -i eth0");
    system("echo nameserver 8.8.8.8 > /etc/resolv.conf");

    // 4.  Wi-Fi
    char wifi_name[64];
    char wifi_pass[64];
    char cmd[256];

    printf("Wi-Fi name: ");
    scanf("%63s", wifi_name);

    printf("Wi-Fi pass: ");
    scanf("%63s", wifi_pass);

    system("rfkill unblock wifi");
    system("ip link set wlan0 up");

    snprintf(cmd, sizeof(cmd), "wpa_passphrase \"%s\" \"%s\" > /etc/wpa.conf", wifi_name, wifi_pass);
    system(cmd);

    system("wpa_supplicant -B -i wlan0 -c /etc/wpa.conf");
    system("udhcpc -i wlan0");
    system("echo nameserver 8.8.8.8 > /etc/resolv.conf");

    // 5. terminal
    printf("\033[2J\033[H");
    printf("\033[32m");
    printf("  +------------------------+\n");
    printf("  |   \\                    |\n");
    printf("  |    \\                   |\n");
    printf("  |     >                  |\n");
    printf("  |    /                   |\n");
    printf("  |   /   ________         |\n");
    printf("  +------------------------+\n");
    printf("         [ terminal ]\n\n");
    printf("  user: root\n");
    printf("  pass: %s\n\n", password);
    printf("\033[0m");
    fflush(stdout);

    // 6. // Set up the environment and launch the /bin/sh shell with a persistent green prompt

    char *args[] = {"/bin/sh", NULL};
    char *envp[] = {
        "PATH=/bin:/sbin:/usr/bin:/usr/sbin",
        "HOME=/root",
        "TERM=linux",
        "PS1=\\033[32mroot@terminal:\\w# \\033[0m",
        NULL
    };

    execve("/bin/sh", args, envp);

    return 0;
}
