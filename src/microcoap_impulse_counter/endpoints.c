#include <stdbool.h>
#include <string.h>
#include "microcoap.h"
#include "connections.h"

static char light = '0';
static uint32_t tick = 0;
// FIXME: magic numbers
#define mac_len 6
#define str_len 32
#define pass_len 64
#define rsplen 1500

// 74:E5:43:DC:B7:E6
static uint8_t mac[mac_len] = {0x74,0xE5,0x43,0xDC,0xB7,0xE6};
static char model[str_len] = "MRCR";
static char serial[str_len] = "SMT1";
static char ap_ssid[str_len];
static char sta_ssid[str_len];
static char ap_pass[pass_len];
static char sta_pass[pass_len];
static char rsp[rsplen] = "";
void build_rsp(void);

#ifdef ARDUINO
#include "Arduino.h"
#else
#include <stdio.h>
#endif
void endpoint_setup(void)
{
#ifdef ARDUINO
    pinMode(LED, OUTPUT); // TODO: check if exist, etc
#endif
    build_rsp();
}

void increment_tick(void)
{
    // TODO: check overflow and stuff
    tick++;
}

static const coap_endpoint_path_t path_well_known_core = {2, {".well-known", "core"}};
static int handle_get_well_known_core(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    return coap_make_response(scratch, outpkt, (const uint8_t *)rsp, strlen(rsp), id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_APPLICATION_LINKFORMAT);
}

static const coap_endpoint_path_t path_light = {1, {"light"}};
static int handle_get_light(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    // FIXME: magic size?
    return coap_make_response(scratch, outpkt, (const uint8_t *)&light, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_put_light(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    if (inpkt->payload.len == 0)
        return coap_make_response(scratch, outpkt, NULL, 0, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_BAD_REQUEST, COAP_CONTENTTYPE_TEXT_PLAIN);
    if (inpkt->payload.p[0] == '1')
    {
        light = '1';
#ifdef ARDUINO
        digitalWrite(LED, DEBUG_LED_LIGHT);
#else
        printf("ON\n");
#endif
        return coap_make_response(scratch, outpkt, (const uint8_t *)&light, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_TEXT_PLAIN);
    }
    else
    {
        light = '0';
#ifdef ARDUINO
        digitalWrite(LED, DEBUG_LED_DARK);
#else
        printf("OFF\n");
#endif
        return coap_make_response(scratch, outpkt, (const uint8_t *)&light, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_TEXT_PLAIN);
    }
}

//////////////////////////////////////
//////////////////////////////////////
static const coap_endpoint_path_t path_tick = {1, {"tick"}};
static int handle_get_tick(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    tick++; // FIXME
    // FIXME: magic size?
    return coap_make_response(scratch, outpkt, (const uint32_t *)&tick, 4, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_put_tick(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    if (inpkt->payload.len == 0)
        return coap_make_response(scratch, outpkt, NULL, 0, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_BAD_REQUEST, COAP_CONTENTTYPE_TEXT_PLAIN);
    // FIXME: magic size?
    memcpy(&tick, inpkt->payload.p, 4);
    return coap_make_response(scratch, outpkt, (const uint16_t *)&tick, 4, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_TEXT_PLAIN);
}
//////////////////////////////////////
//////////////////////////////////////
static const coap_endpoint_path_t path_model = {1, {"model"}};
int handle_get_model(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    return coap_make_response(scratch, outpkt, (const char *)&model, str_len, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

int handle_put_model(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{

}

//////////////////////////////////////
//////////////////////////////////////
static const coap_endpoint_path_t path_serial = {1, {"serial"}};
int handle_get_serial(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    return coap_make_response(scratch, outpkt, (const char *)&serial, str_len, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

int handle_put_serial(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{

}

//////////////////////////////////////
//////////////////////////////////////
static const coap_endpoint_path_t path_mac = {1, {"mac"}};
int handle_get_mac(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    return coap_make_response(scratch, outpkt, (const uint8_t *)&mac, mac_len, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

//////////////////////////////////////
//////////////////////////////////////
static const coap_endpoint_path_t path_ap_ssid = {1, {"ap_ssid"}};
int handle_get_ap_ssid(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    return coap_make_response(scratch, outpkt, (const char *)&ap_ssid, str_len, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

int handle_put_ap_ssid(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{

}

//////////////////////////////////////
//////////////////////////////////////

int handle_get_ap_pass(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{

}

int handle_put_ap_pass(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{

}

//////////////////////////////////////
//////////////////////////////////////
static const coap_endpoint_path_t path_sta_ssid = {1, {"sta_ssid"}};
int handle_get_sta_ssid(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    return coap_make_response(scratch, outpkt, (const char *)&sta_ssid, str_len, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

int handle_put_sta_ssid(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{

}

//////////////////////////////////////
//////////////////////////////////////

int handle_get_sta_pass(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{

}

int handle_put_sta_pass(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{

}


//////////////////////////////////////
//////////////////////////////////////

const coap_endpoint_t endpoints[] =
{
    {COAP_METHOD_GET, handle_get_well_known_core, &path_well_known_core, "ct=40"},
    {COAP_METHOD_GET, handle_get_tick, &path_tick, "ct=42"},
    {COAP_METHOD_PUT, handle_put_tick, &path_tick, NULL},
    {COAP_METHOD_GET, handle_get_mac, &path_mac, "ct=42"},
    {COAP_METHOD_GET, handle_get_model, &path_model, "ct=0"},
    {COAP_METHOD_PUT, handle_put_model, &path_model, NULL},
    {COAP_METHOD_GET, handle_get_serial, &path_serial, "ct=0"},
    {COAP_METHOD_PUT, handle_put_serial, &path_serial, NULL},
    {COAP_METHOD_GET, handle_get_light, &path_light, "ct=0"},
    {COAP_METHOD_PUT, handle_put_light, &path_light, NULL},
    {(coap_method_t)0, NULL, NULL, NULL}
};

void build_rsp(void)
{
    uint16_t len = rsplen;
    const coap_endpoint_t *ep = endpoints;
    int i;

    len--; // Null-terminated string

    while(NULL != ep->handler)
    {
        if (NULL == ep->core_attr) {
            ep++;
            continue;
        }

        if (0 < strlen(rsp)) {
            strncat(rsp, ",", len);
            len--;
        }

        strncat(rsp, "<", len);
        len--;

        for (i = 0; i < ep->path->count; i++) {
            strncat(rsp, "/", len);
            len--;

            strncat(rsp, ep->path->elems[i], len);
            len -= strlen(ep->path->elems[i]);
        }

        strncat(rsp, ">;", len);
        len -= 2;

        strncat(rsp, ep->core_attr, len);
        len -= strlen(ep->core_attr);

        ep++;
    }
}


uint32_t get_tick()
{
    return tick;
}

int set_tick(uint32_t value)
{
    tick = value;
}

int set_model()
{
    // TODO:
    // memcpy(&tick, inpkt->payload.p, 4);
}
