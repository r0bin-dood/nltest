#include "msg.h"

static int wiphy_index = -1;
static int driver_id = -1;

unsigned char beacon_head[] = { /* frame control, address, etc. */ };
unsigned char beacon_tail[] = { };

static int helper_get_driver_id(struct nl_sock *socket)
{
    driver_id = genl_ctrl_resolve(socket, "nl80211");
    if (driver_id < 0)
        fprintf(stderr, "genl_ctrl_resolve error: %s\n", strerror(-driver_id));
    return driver_id;
}

static int handle_wiphy_info(struct nl_msg *msg, void *arg) {
    struct nlattr *attrs[NL80211_ATTR_MAX + 1];
    struct nlmsghdr *nlh = nlmsg_hdr(msg);

    if (genlmsg_parse(nlh, 0, attrs, NL80211_ATTR_MAX, NULL) < 0) {
        fprintf(stderr, "Failed to parse attributes.\n");
        return NL_SKIP;
    }

    if (attrs[NL80211_ATTR_WIPHY]) {
        wiphy_index = nla_get_u32(attrs[NL80211_ATTR_WIPHY]);
        return NL_STOP;
    }

    return NL_SKIP;
}

static int helper_get_wiphy(struct nl_sock *socket)
{
    struct nl_msg *msg = nlmsg_alloc();
    if (!msg) {
        perror("Failed to allocate netlink message");
        return -1;
    }

    if (!genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, driver_id, 0, NLM_F_DUMP, NL80211_CMD_GET_WIPHY, 0)) {
        fprintf(stderr, "Failed to setup message header.\n");
        nlmsg_free(msg);
        return -1;
    }
    if (nl_send_auto(socket, msg) < 0) {
        perror("Failed to send message");
        nlmsg_free(msg);
        return -1;
    }

    nl_socket_modify_cb(socket, NL_CB_VALID, NL_CB_CUSTOM, handle_wiphy_info, NULL);
    if (nl_recvmsgs_default(socket) < 0) {
        perror("Failed to receive messages");
        nlmsg_free(msg);
        return -1;
    }

    nlmsg_free(msg);

    return 0;
}

// NETLINK_GENERIC, NETLINK_ROUTE
struct nl_sock *msg_start(int protocol)
{
    struct nl_sock *socket = nl_socket_alloc();
    if (socket == NULL) {
        perror("nl_socket_alloc");
        return NULL;
    }

    if (nl_connect(socket, protocol) != 0) {
        perror("nl_connect");
        nl_socket_free(socket);
        return NULL;
    }

    if (driver_id < 0) {
        if (helper_get_driver_id(socket) < 0) {
            nl_socket_free(socket);
            return NULL;
        }
    }

    if (wiphy_index < 0) {
        if (helper_get_wiphy(socket) < 0) {
            nl_socket_free(socket);
            return NULL;
        }
    }

    return socket;
}

int msg_create_iface(struct nl_sock *socket, const char *iface_name)
{
    struct nl_msg *msg = nlmsg_alloc();
    if (msg == NULL) {
        perror("nlmsg_alloc");
        return -1;
    }

    genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, driver_id, 0, 0, NL80211_CMD_NEW_INTERFACE, 0);

    nla_put_u32(msg, NL80211_ATTR_WIPHY, wiphy_index);
    nla_put_string(msg, NL80211_ATTR_IFNAME, iface_name);
    nla_put_u32(msg, NL80211_ATTR_IFTYPE, NL80211_IFTYPE_AP);

    if (nl_send_auto(socket, msg) < 0) {
        fprintf(stderr, "Failed to send Netlink message.\n");
        nlmsg_free(msg);
        return -1;
    }

    struct nl_cb *cb = nl_cb_alloc(NL_CB_DEBUG);
    if (cb == NULL) {
        fprintf(stderr, "Failed to allocate netlink callback.\n");
        nlmsg_free(msg);
        return -1;
    }
    nl_recvmsgs(socket, cb);

    nlmsg_free(msg);
    nl_cb_put(cb);

    return 0;
}

int msg_iface_set_channel(struct nl_sock *socket, const char *iface_name, int channel)
{
    struct nl_msg *msg = nlmsg_alloc();
    if (!msg) {
        perror("nlmsg_alloc");
        return -1;
    }

    int if_index = if_nametoindex(iface_name);
    if (if_index == 0) {
        fprintf(stderr, "Interface %s does not exist.\n", iface_name);
        nlmsg_free(msg);
        return -1;
    }

    genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, driver_id, 0, 0, NL80211_CMD_SET_CHANNEL, 0);
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, if_index);
    nla_put_u32(msg, NL80211_ATTR_WIPHY_FREQ, channel);

    if (nl_send_auto(socket, msg) < 0) {
        fprintf(stderr, "Failed to set mode and channel.\n");
        nlmsg_free(msg);
        return -1;
    }
    struct nl_cb *cb = nl_cb_alloc(NL_CB_DEBUG);
    if (cb == NULL) {
        fprintf(stderr, "Failed to allocate netlink callback.\n");
        nlmsg_free(msg);
        return -1;
    }
    nl_recvmsgs(socket, cb);

    nlmsg_free(msg);
    nl_cb_put(cb);

    return 0;
}

int msg_iface_set_open_auth(struct nl_sock *socket, const char *iface_name)
{
    struct nl_msg *msg = nlmsg_alloc();
    if (!msg) {
        perror("nlmsg_alloc");
        return -1;
    }

    int if_index = if_nametoindex(iface_name);
    if (if_index == 0) {
        fprintf(stderr, "Interface %s does not exist.\n", iface_name);
        nlmsg_free(msg);
        return -1;
    }

    genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, driver_id, 0, 0, NL80211_CMD_SET_INTERFACE, 0);
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, if_index);
    nla_put_u32(msg, NL80211_ATTR_IFTYPE, NL80211_IFTYPE_AP);
    nla_put_u32(msg, NL80211_ATTR_AUTH_TYPE, NL80211_AUTHTYPE_OPEN_SYSTEM);

    if (nl_send_auto(socket, msg) < 0) {
        fprintf(stderr, "Failed to set open authentication.\n");
        nlmsg_free(msg);
        return -1;
    }
    struct nl_cb *cb = nl_cb_alloc(NL_CB_DEBUG);
    if (cb == NULL) {
        fprintf(stderr, "Failed to allocate netlink callback.\n");
        nlmsg_free(msg);
        return -1;
    }
    nl_recvmsgs(socket, cb);

    nlmsg_free(msg);
    nl_cb_put(cb);

    return 0;
}

int msg_start_ap(struct nl_sock *socket, const char *iface_name, const char *ssid)
{
    struct nl_msg *msg = nlmsg_alloc();
    if (!msg) {
        perror("Failed to allocate netlink message");
        return -1;
    }

    genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, driver_id, 0, 0, NL80211_CMD_START_AP, 0);

    // 412 @NL80211_CMD_START_AP
    // required:
    nla_put_u32(msg, NL80211_ATTR_DTIM_PERIOD, 1);
    nla_put_u32(msg, NL80211_ATTR_BEACON_INTERVAL, 100);
    nla_put(msg, NL80211_ATTR_BEACON_HEAD, sizeof(beacon_head), beacon_head);
    nla_put(msg, NL80211_ATTR_BEACON_TAIL, sizeof(beacon_tail), beacon_tail);
    // now let's set the add-ons
    nla_put_string(msg, NL80211_ATTR_IFNAME, iface_name);
    nla_put(msg, NL80211_ATTR_SSID, strlen(ssid), ssid);

    if (nl_send_auto(socket, msg) < 0) {
        fprintf(stderr, "Failed to start AP.\n");
        nlmsg_free(msg);
        return -1;
    }
    struct nl_cb *cb = nl_cb_alloc(NL_CB_DEBUG);
    if (cb == NULL) {
        fprintf(stderr, "Failed to allocate netlink callback.\n");
        nlmsg_free(msg);
        return -1;
    }
    nl_recvmsgs(socket, cb);

    nlmsg_free(msg);
    nl_cb_put(cb);

    return 0;
}

int msg_iface_up(const char *iface_name)
{
    int sockfd;
    struct ifreq ifr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    strncpy(ifr.ifr_name, iface_name, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
        perror("ioctl SIOCGIFFLAGS");
        close(sockfd);
        return -1;
    }

    ifr.ifr_flags |= IFF_UP;

    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) {
        perror("ioctl SIOCSIFFLAGS");
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0;
}

int msg_iface_down(const char *iface_name)
{
    int sockfd;
    struct ifreq ifr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    strncpy(ifr.ifr_name, iface_name, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
        perror("ioctl SIOCGIFFLAGS");
        close(sockfd);
        return -1;
    }

    ifr.ifr_flags &= ~IFF_UP;

    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) {
        perror("ioctl SIOCSIFFLAGS");
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0;
}

int msg_delete_iface(struct nl_sock *socket, const char *iface_name)
{
    struct nl_msg *msg = nlmsg_alloc();
    if (msg == NULL) {
        perror("nlmsg_alloc");
        return -1;
    }

    int if_index = if_nametoindex(iface_name);
    if (if_index == 0) {
        fprintf(stderr, "Interface %s does not exist.\n", iface_name);
        nlmsg_free(msg);
        return -1;
    }

    genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, driver_id, 0, 0, NL80211_CMD_DEL_INTERFACE, 0);
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, if_index);

    if (nl_send_auto(socket, msg) < 0) {
        fprintf(stderr, "Failed to send Netlink message.\n");
        nlmsg_free(msg);
        return -1;
    }

    struct nl_cb *cb = nl_cb_alloc(NL_CB_DEBUG);
    if (cb == NULL) {
        fprintf(stderr, "Failed to allocate netlink callback.\n");
        nlmsg_free(msg);
        return -1;
    }
    nl_recvmsgs(socket, cb);

    nlmsg_free(msg);
    nl_cb_put(cb);

    return 0;
}

void msg_stop(struct nl_sock *socket)
{
    if (socket != NULL)
        nl_socket_free(socket);
}
