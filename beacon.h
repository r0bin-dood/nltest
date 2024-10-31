#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>

typedef struct ieee80211_beacon_frame {
    uint8_t *buf;
    size_t len;
} bf_t;

bf_t *create_bf(uint8_t *dst_addr, uint8_t *src_addr, uint8_t *bssid, const char *ssid, uint8_t channel);