#include "usbproto.h"
#include "pico/stdlib.h"
#include <stdbool.h>
#include "hardware/gpio.h"


void usb_proto_init(void) {
    tusb_init();
}


/**
 * Formato de los datos a enviar
 * &<tipo>,<payload_bytes>,<payload>*
 */
void sendData(void) {
 acp_response_t resp = {
        .type = ACP_RESP_DATA,
    };

    uint8_t payload[] = {0xff, 0x12, 0x32, 0xf9, 0x33, 0x26}; // limitar para que solo se puedan pasar 6 medicinoes
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

void sendString(const char* str) {
    
    if(tud_cdc_connected()) {
        tud_cdc_write_str(str);
        tud_cdc_write_flush();
    }
}

void process_commands(void) {
    uint8_t buf[64], buff_cmd[64];
    uint8_t rx_idx = 0;
    static bool fChar = false;

    if(tud_cdc_available()){
        
        uint32_t count = tud_cdc_read(buf, sizeof(buf)); // lee la información del USB y la almacena en el buffer
    
        for(uint8_t i = 0; i < count; i++) {
            char c = buf[i];

            if(c == '&') fChar = true; // start char

            if(rx_idx < sizeof(buff_cmd) - 1) {
                 buff_cmd[rx_idx++] = c; // almacena el carácter en el buffer de comando para luego procesarlo con acp_parse_command
            }

            if(c == '*' && fChar){
                acp_command_t cmd;
                 
                if(acp_parse_command(&cmd, buff_cmd, rx_idx)){
                    
                    switch (cmd.type) //   ACP_CMD_PING = 0, ACP_CMD_MEM, ACP_CMD_TOGGLE_LED, ACP_CMD_ADC, ACP_CMD_SET_LED, ACP_CMD_COUNT, ACP_CMD_INVALID
                    {
                        case ACP_CMD_PING:
                            sendString("PONG\n");
                            break;
                        case ACP_CMD_MEM:
                            break;
                        case ACP_CMD_TOGGLE_LED:
                            break;                                                    
                        case ACP_CMD_ADC:
                            sendData();
                            break;
                        case ACP_CMD_SET_LED:
                            break;
                        case ACP_CMD_COUNT:
                            break;                        
                    default:
                        //ACP_CMD_INVALID
                        sendString("ERROR :P");              
                        break;
                    }

                }

                fChar = false; // reset for next command

            }

            }
        }
    }

