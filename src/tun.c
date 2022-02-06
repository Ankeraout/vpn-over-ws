#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "client.h"
#include "server.h"
#include "tun.h"

#define C_CONFIG_TUN_MTU 1400
#define C_CONFIG_TUN_SUBNET_MASK "255.255.255.0"
#define C_READ_BUFFER_SIZE 1504

static int s_tunDeviceFd;

int tunInit(const char *ip) {
    // Initialize TUN device
    s_tunDeviceFd = open("/dev/net/tun", O_RDWR);

    if(s_tunDeviceFd < 0) {
        perror("Failed to open tun device");
        return 1;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_TUN;

    if(ioctl(s_tunDeviceFd, TUNSETIFF, &ifr)) {
        perror("Failed to get options on tun file descriptor.");
        close(s_tunDeviceFd);
        return 2;
    }

    printf("Host interface name: %s\n", ifr.ifr_ifrn.ifrn_name);

    // Set interface up
    int tmpSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if(ioctl(tmpSocket, SIOCGIFFLAGS, &ifr)) {
        perror("Failed to get the tun interface flags");
        close(s_tunDeviceFd);
        return 3;
    }

    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;

    if(ioctl(tmpSocket, SIOCSIFFLAGS, &ifr)) {
        perror("Failed to set the tun interface up");
        close(s_tunDeviceFd);
        return 4;
    }

    // Set IP address
    struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_addr;

    if(inet_pton(AF_INET, ip, &addr->sin_addr) != 1) {
        perror("Failed to parse IP address for tun interface");
        close(s_tunDeviceFd);
        return 5;
    }

    addr->sin_family = AF_INET;

    if(ioctl(tmpSocket, SIOCSIFADDR, &ifr)) {
        perror("Failed to set the IP address for the tun interface");
        close(s_tunDeviceFd);
        return 6;
    }

    if(inet_pton(AF_INET, C_CONFIG_TUN_SUBNET_MASK, &addr->sin_addr) != 1) {
        perror("Failed to parse subnetwork mask for the tun interface");
        close(s_tunDeviceFd);
        return 7;
    }

    if(ioctl(tmpSocket, SIOCSIFNETMASK, &ifr)) {
        perror("Failed to set the subnetwork mask for the tun interface");
        close(s_tunDeviceFd);
        return 8;
    }

    // Set MTU
    ifr.ifr_mtu = C_CONFIG_TUN_MTU;

    if(ioctl(tmpSocket, SIOCSIFMTU, &ifr)) {
        perror("Failed to set the MTU for the tun interface");
        close(s_tunDeviceFd);
        return 1;
    }

    sleep(1);

    return 0;
}

int tunWrite(void *buffer, size_t size) {
    if(write(s_tunDeviceFd, buffer, size) < 0) {
        return -1;
    } else {
        return 0;
    }
}

int tunClose(void) {
    return close(s_tunDeviceFd);
}

ssize_t tunRead(void *p_buffer) {
    return read(s_tunDeviceFd, p_buffer, C_READ_BUFFER_SIZE);
}
