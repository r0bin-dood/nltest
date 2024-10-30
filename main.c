#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include "msg.h"

static int got_exit = 0;

void exit_handler(int sig, siginfo_t *sig_info, void *context)
{
    got_exit = 1;
}

void install_handlers()
{
    struct sigaction act;

    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &exit_handler;
    if (sigaction(SIGTERM, &act, NULL) == -1)
    {
        printf("%d: %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGINT, &act, NULL) == -1)
    {
        printf("%d: %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGABRT, &act, NULL) == -1)
    {
        printf("%d: %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/*
interface=lilap0
driver=nl80211
ssid=Your_SSID_Name
hw_mode=g
channel=6
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=Your_Password
wpa_key_mgmt=WPA-PSK
rsn_pairwise=CCMP
*/

int main()
{
    install_handlers();

    struct nl_sock *socket = msg_start(NETLINK_GENERIC);
    if (socket == NULL)
        return -1;
    
    const char *iface_name = "lilap0";
    const char *ssid = "lilapFree";
    if (msg_create_iface(socket, iface_name) < 0)
        return -1;
    if (msg_start_ap(socket, iface_name, ssid) < 0)
        return -1;

    if (msg_iface_up(iface_name) < 0)
        return -1;
    
    printf("waiting...\n");
    while (!got_exit)
        usleep(10000);

    if (msg_iface_down(iface_name) < 0)
        return -1;

    if (msg_delete_iface(socket, iface_name) < 0)
        return -1;

    msg_stop(socket);
    
    return 0;
}