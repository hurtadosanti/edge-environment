#include "wifi_connect.hpp"
#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_DECLARE(app, LOG_LEVEL_INF);

static K_SEM_DEFINE(wifi_connected_sem, 0, 1);
static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;

static void wifi_mgmt_handler(struct net_mgmt_event_callback *cb,
                             uint32_t mgmt_event, struct net_if *iface)
{
    if (mgmt_event == NET_EVENT_WIFI_CONNECT_RESULT) {
        const struct wifi_status *status = (const struct wifi_status *)cb->info;
        if (status->status) {
            LOG_ERR("WiFi Connection failed (%d)", status->status);
        } else {
            LOG_INF("WiFi WLAN connected");
        }
    }
}

static void ipv4_mgmt_handler(struct net_mgmt_event_callback *cb,
                             uint32_t mgmt_event, struct net_if *iface)
{
    if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD) {
        LOG_INF("IPv4 address added successfully");
        k_sem_give(&wifi_connected_sem);
    }
}

bool wifi_connect()
{
    struct net_if *iface = net_if_get_default();
    if (!iface) {
        LOG_ERR("No default network interface found");
        return false;
    }

    net_mgmt_init_event_callback(&wifi_cb, wifi_mgmt_handler,
                                 NET_EVENT_WIFI_CONNECT_RESULT);
    net_mgmt_add_event_callback(&wifi_cb);

    net_mgmt_init_event_callback(&ipv4_cb, ipv4_mgmt_handler,
                                 NET_EVENT_IPV4_ADDR_ADD);
    net_mgmt_add_event_callback(&ipv4_cb);

    struct wifi_connect_req_params params = {0};
    params.ssid = (const uint8_t *)CONFIG_APP_WIFI_SSID;
    params.ssid_length = strlen(CONFIG_APP_WIFI_SSID);
    params.psk = (const uint8_t *)CONFIG_APP_WIFI_PASSWORD;
    params.psk_length = strlen(CONFIG_APP_WIFI_PASSWORD);
    params.channel = WIFI_CHANNEL_ANY;
    params.security = WIFI_SECURITY_TYPE_PSK;

    LOG_INF("Connecting to SSID: %s...", CONFIG_APP_WIFI_SSID);
    
    if (net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &params, sizeof(params))) {
        LOG_ERR("WiFi connection request failed immediately");
        return false;
    }

    // Wait up to 30 seconds for connection and IP assignment
    if (k_sem_take(&wifi_connected_sem, K_SECONDS(30)) != 0) {
        LOG_ERR("WiFi connection or DHCP timeout");
        return false;
    }

    return true;
}
