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

int tunDevice;
extern bool flag_server;

int tunInit(const char *ip) {
    tunDevice = open("/dev/net/tun", O_RDWR);

    if(tunDevice < 0) {
        perror("Failed to open tun device");
        return 1;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_TUN;

    if(ioctl(tunDevice, TUNSETIFF, &ifr)) {
        perror("Failed to get options on tun file descriptor.");
        close(tunDevice);
        return 2;
    }

    printf("Host interface name: %s\n", ifr.ifr_ifrn.ifrn_name);

    // Set interface up
    int tmpSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if(ioctl(tmpSocket, SIOCGIFFLAGS, &ifr)) {
        perror("Failed to get the tun interface flags");
        close(tunDevice);
        return 3;
    }

    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;

    if(ioctl(tmpSocket, SIOCSIFFLAGS, &ifr)) {
        perror("Failed to set the tun interface up");
        close(tunDevice);
        return 4;
    }

    // Set IP address
    struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_addr;

    if(inet_pton(AF_INET, ip, &addr->sin_addr) != 1) {
        perror("Failed to parse IP address for tun interface");
        close(tunDevice);
        return 5;
    }

    addr->sin_family =  AF_INET;

    if(ioctl(tmpSocket, SIOCSIFADDR, &ifr)) {
        perror("Failed to set the IP address for the tun interface");
        close(tunDevice);
        return 6;
    }

    if(inet_pton(AF_INET, "255.255.255.0", &addr->sin_addr) != 1) {
        perror("Failed to parse subnetwork mask for the tun interface");
        close(tunDevice);
        return 7;
    }

    if(ioctl(tmpSocket, SIOCSIFNETMASK, &ifr)) {
        perror("Failed to set the subnetwork mask for the tun interface");
        close(tunDevice);
        return 8;
    }

    // Set MTU
    ifr.ifr_mtu = 1400;

    if(ioctl(tmpSocket, SIOCSIFMTU, &ifr)) {
        perror("Failed to set the MTU for the tun interface");
        close(tunDevice);
        return 1;
    }

    return 0;
}

int tunWrite(void *buffer, size_t size) {
    if(write(STDOUT_FILENO, buffer, size) < 0) {
        return -1;
    } else {
        return 0;
    }
}

void tunClose(void) {
    close(tunDevice);
}

void tunLoop(void) {
    uint8_t buffer[1504];

    while(true) {
        ssize_t packetSize = read(STDIN_FILENO, buffer, sizeof(buffer));

        printf("Received packet of size %d\n", (int)packetSize);

        if(packetSize <= 0) {
            break;
        }

        if(flag_server) {
            serverWrite(buffer, packetSize);
        } else {
            clientWrite(buffer, packetSize);
        }
    }
}
