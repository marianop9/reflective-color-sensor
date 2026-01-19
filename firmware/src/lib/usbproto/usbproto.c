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
//     static uint32_t rx_idx = 0;

//     if(tud_cdc_available()) {
//         uint8_t c;
//         uint32_t count = tud_cdc_read(&c, sizeof(c));

//         if(count != 0 && rx_idx < MAX_COMMAND_LEN) {
//             rx_buffer[rx_idx] = c;
//             rx_idx++;
//         }

//         if(c == ACP_END_CHAR) {
//             acp_command_t cmd;
//             bool result = acp_parse_command(&cmd, (char*)rx_buffer, rx_idx);

//             if(result) {
//                 if(cmd.type == ACP_CMD_ADC) {
//                     sendData();
//                 }
//             }

//             // Reset buffer index for next command
//             rx_idx = 0;
//             memset(rx_buffer, 0, MAX_COMMAND_LEN);
//         }
//     }
//     else{
//         char buffer[] = "HOLIS :D";
//         if(tud_cdc_connected()) {
//             tud_cdc_write(buffer, sizeof(buffer)+1);
//             tud_cdc_write_flush();
//         }
//         sleep_ms(1000);
//     }

// }