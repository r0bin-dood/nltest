#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/attr.h>
#include <netlink/msg.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <linux/nl80211.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

struct nl_sock *msg_start(int protocol);
int msg_create_iface(struct nl_sock *socket, const char *iface_name);
int msg_start_ap(struct nl_sock *socket, const char *iface_name, const char *ssid);
int msg_iface_up(const char *iface_name);
int msg_iface_down(const char *iface_name);
int msg_delete_iface(struct nl_sock *socket, const char *iface_name);
void msg_stop(struct nl_sock *socket);