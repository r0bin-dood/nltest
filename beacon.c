#include "beacon.h"

#pragma pack(push, 1)
struct tag {
    uint8_t num;
    uint8_t len;
    uint8_t data[];
};
struct beacon_frame {
    union {
        struct {
            uint8_t version:    2;
            uint8_t type:       2;
            uint8_t subtype:    4;
            struct {
                uint8_t ds_status:  2;
                uint8_t more_frags: 1;
                uint8_t retry:      1;
                uint8_t power_mgmt: 1;
                uint8_t more_data:  1;
                uint8_t protected:  1;
                uint8_t ordered:    1;
            } flags;
        };
        uint16_t data;
    } frame_control_field;
    uint16_t duration;
    uint8_t dest_addr[6];
    uint8_t src_addr[6];
    uint8_t bssid[6];
    union {
        struct {
            uint8_t frag_num:   4;
            uint16_t seq_num:   12;
        };
        uint16_t data;
    } frag_seq_num;
    uint64_t timestamp;
    uint16_t beacon_interval;
    union {
        struct {
            uint8_t ess_cap:                1;
            uint8_t ibss_status:            1;
            uint8_t reserved:               1;
            uint8_t reserved2:              1;
            uint8_t privacy:                1;
            uint8_t short_preamble:         1;
            uint8_t critical_update:        1;
            uint8_t nontransmitted_bssids:  1;
            uint8_t spectrum_mgmt:          1;
            uint8_t qos:                    1;
            uint8_t short_slot_time:        1;
            uint8_t auto_pwr_save_delivery: 1;
            uint8_t radio_measurement:      1;
            uint8_t epd:                    1;
            uint8_t reserved3:              1;
        };
        uint16_t data;
    } capabilities;
    uint8_t tags[];
};
#pragma pack(pop)

bf_t *create_bf(uint8_t *dst_addr, uint8_t *src_addr, uint8_t *bssid, const char *ssid, uint8_t channel)
{
    size_t len = sizeof(struct beacon_frame) + (2 * sizeof(struct tag)) + (strlen(ssid)) + sizeof(channel);
    uint8_t *buf = calloc(1, len);
    struct beacon_frame *bf = (struct beacon_frame *)buf;

    bf->frame_control_field.subtype = 8;

    memcpy(bf->dest_addr, dst_addr, 6);
    memcpy(bf->src_addr, src_addr, 6);
    memcpy(bf->bssid, bssid, 6);

    bf->beacon_interval = 100;

    struct tag *ssid_tag = (struct tag *)&bf->tags[0];
    ssid_tag->num = 0;
    ssid_tag->len = strlen(ssid);
    memcpy(ssid_tag->data, ssid, ssid_tag->len);

    struct tag *chnl_tag = (struct tag *)&bf->tags[sizeof(struct tag) + ssid_tag->len];
    chnl_tag->num = 3;
    chnl_tag->len = sizeof(channel);
    memcpy(chnl_tag->data, &channel, chnl_tag->len);

    bf_t *ret = malloc(sizeof(bf_t));
    ret->buf = buf;
    ret->len = len;

    return ret;
}