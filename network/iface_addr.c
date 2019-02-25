#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

void query_addr_by_ioctl(const char *iface);
void query_addr_by_getifaddr(const char *iface);

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Usage: %s <iface name>\n", argv[0]);
        return 0;
    }

    query_addr_by_ioctl(argv[1]);
    query_addr_by_getifaddr(argv[1]);

    return 0;
}

void
query_addr_by_ioctl(const char *iface)
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        printf("Failed to open socket\n");
        return ;
    }

    // must reset ifr, otherwise the ifr_addr cloud be random if disconnected
    memset(&ifr, 0, sizeof(ifr));
    //Type of address to retrieve - IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;
    //Copy the interface name in the ifreq structure
    strncpy(ifr.ifr_name, iface, IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);

    printf("[ioctl]\t%s - %s\n", iface, inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr));
}

void
query_addr_by_getifaddr(const char *iface)
{
    struct ifaddrs *iter = NULL;
    struct ifaddrs *iter_tmp = NULL;
    int result = 0;
    char addr[13] = {0};

    result = getifaddrs(&iter);
    if (result != 0) {
        printf("Failed to get iface addrs\n");
        return;
    }

    iter_tmp = iter;
    while(iter != NULL) {
        if (iter->ifa_addr->sa_family == AF_INET &&
            strcmp(iter->ifa_name, iface) == 0 ) {
            strncpy(addr, inet_ntoa(((struct sockaddr_in*)iter->ifa_addr)->sin_addr), 12);
            break;
        }
        iter = iter->ifa_next;
        continue;
    }
    freeifaddrs(iter_tmp);

    printf("[getifaddrs]\t%s - %s\n", iface, addr);
}
