#ifndef __KPLORA_H
#define __KPLORA_H

#define KPLORA_RELAYBUFFER_SIZE 5
#define KPLORA_LBT1_TIMEOUT 5000

#include "dataPacket.h"
#include "lora.h"
#include "trackerHw.h"
#include "config.h"

extern int KPLORA_LBTCounter;

void KPLORA_pack_data_standard(int _state, uint32_t time_ms, uint8_t _vbat, uint32_t _lat, uint32_t _lon, uint32_t _alt, uint8_t _fix, uint8_t _sats);
void KPLORA_send_data_lora();
void KPLORA_fillRelayBuffer(kppacket_t newData, kppacket_t* buffer);
void KPLORA_listenForPackets();
int KPLORA_listenBeforeTalk();
void KPLORA_transmitRelayBuffer();

#endif
