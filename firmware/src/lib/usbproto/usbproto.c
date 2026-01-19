#include "usbproto.h"
#include "pico/stdlib.h"



void usb_proto_init(void) {
    tusb_init();
}

void sendData(void) {
 acp_response_t resp = {
        .type = ACP_RESP_DATA,
    };

    uint8_t payload[] = {0xff, 0x12, 0x32, 0xf9, 0x33, 0x26};
    size_t payload_length = sizeof(payload);
    resp.len_bytes = payload_length;
    memcpy(resp.payload, payload, resp.len_bytes);
    
    uint8_t buffer[256];
    //bool result = acp_create_response(buffer, &resp);
    int result = acp_create_response(buffer, &resp);
    if(tud_cdc_connected() && 0 < result) {
        tud_cdc_write(buffer, (uint32_t)result);
        tud_cdc_write_flush();
    }
}

void process_commands(void) {

    if(tud_cdc_available()){
        uint8_t buf[64];
        uint32_t count = tud_cdc_read(buf, sizeof(buf));
        static bool fChar = false;

        for(uint32_t i = 0; i < count; i++) {
            if(buf[i] == '&'){
                fChar = true;
            }
            if(buf[i] == '*' && fChar){
                sendData();
                fChar = false;
            }
        }
    }
}
