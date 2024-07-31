//This file contains functions specific to the KP-LORA protocol
#include "kplora.h"
#include <string.h>

kppacket_t KPLORA_selfTelemetryPacket;
kppacket_t KPLORA_relayBuffer[KPLORA_RELAYBUFFER_SIZE];
kppacket_t KPLORA_receivedPacket;
//int KPLORA_LBTCounter;
uint16_t KPLORA_packetCounter = 0;

void KPLORA_pack_data_standard(int _state, uint32_t time_ms, uint8_t _vbat, uint32_t _lat, uint32_t _lon, uint32_t _alt, uint8_t _fix, uint8_t _sats) {
	kppacket_header_t header = {
		.packet_id.msg_type = PACKET_TRACKER,
		.packet_id.msg_ver = 0,
		.packet_id.retransmit = 1,
		.packet_id.encoded = 0,
		.packet_id.redu = 0,
		.sender_id = TRACKER_ID,
		.dest_id = 0,
		.packet_no = KPLORA_packetCounter++,
		.timestamp_ms = time_ms,
		.CRC16 = 0
	};

	kppacket_payload_rocket_tracker_t payload = {
		.retransmission_cnt = 0,
		.vbat_10 = _vbat,
		.lat = _lat,
		.lon = _lon,
		.alti_gps = _alt * 1000,
		.sats_fix = ((_fix & 3) << 6) | (_sats & 0x3F)
	};

	KPLORA_selfTelemetryPacket.packet_len = sizeof(kppacket_header_t) + sizeof(kppacket_payload_rocket_tracker_t);
	memcpy(&(KPLORA_selfTelemetryPacket.header), &header, sizeof(kppacket_header_t));
	memcpy(&(KPLORA_selfTelemetryPacket.payload), &payload, sizeof(kppacket_payload_rocket_tracker_t));

}

void KPLORA_send_data_lora() {
	HW_writeLED(1);
	RADIO_sendPacketLoRa((uint8_t*)&(KPLORA_selfTelemetryPacket), KPLORA_selfTelemetryPacket.packet_len, 500);
	HW_writeLED(0);
	RADIO_clearIrqStatus();
	HW_DelayMs(5);
}

void KPLORA_fillRelayBuffer(kppacket_t newData, kppacket_t* buffer) {
	int i;
	for(i = 0;i<KPLORA_RELAYBUFFER_SIZE;i++) {
		if(buffer[i].header.packet_id.ID == 0) {
			buffer[i] = newData;
			return;
		}
	}
}

void KPLORA_listenForPackets() {
	RADIO_clearIrqStatus();
	HW_DelayMs(5);
	RADIO_setRxSingle();
	HW_DelayMs(5);
	HW_resetTimer3();
	while(HW_getTimer3() < TRACKER_TRANSMISSION_SPACING) {
		if((RADIO_readIrqStatus() & 0x2) == 0x2) {
			if(RADIO_getCRC() == 0) {
				if(RADIO_getRxPayloadSize() == KPLORA_selfTelemetryPacket.packet_len) {
					RADIO_getRxPayload((uint8_t*)&KPLORA_receivedPacket);
					if(KPLORA_receivedPacket.header.packet_id.msg_type == PACKET_TRACKER) {
						KPLORA_fillRelayBuffer(KPLORA_receivedPacket, KPLORA_relayBuffer);
					}
				}
			}
			RADIO_setBufferBaseAddress(0, 100);
			RADIO_clearIrqStatus();
			HW_DelayMs(5);
		}
	}
}

int KPLORA_listenBeforeTalk() {
	HW_resetTimer3();
	while(HW_getTimer3() < KPLORA_LBT1_TIMEOUT) { //If the interference doesn't stop after 5s, transmit anyway
		HW_DelayMs((uint8_t)(RADIO_getRandInt(1)));
		int flag = 1;
		int i;
		for(i=0;i<10;i++) {
			if((RADIO_get_CAD()) || (RADIO_get_rssi() > -90)) {
				flag = 0;
				break;
			}
		}
		if(flag == 1) {
			break;
		}
	}
	return 1;
}

void KPLORA_transmitRelayBuffer() {
	int i;
	for(i=0;i<KPLORA_RELAYBUFFER_SIZE;i++) {
		kppacket_payload_rocket_tracker_t payload;
		memcpy(&payload, &(KPLORA_relayBuffer[i].payload), sizeof(kppacket_payload_rocket_tracker_t));
		if(payload.retransmission_cnt == 0) {
			payload.retransmission_cnt++;
			memcpy(&(KPLORA_relayBuffer[i].payload), &payload, sizeof(kppacket_payload_rocket_tracker_t));
			KPLORA_listenBeforeTalk();
			HW_writeLED(1);
			RADIO_sendPacketLoRa((uint8_t*)&(KPLORA_relayBuffer[i]), KPLORA_relayBuffer[i].packet_len, 500);
			HW_writeLED(0);
			RADIO_clearIrqStatus();
			HW_DelayMs(5);
		}
	}
	for(i=0;i<KPLORA_RELAYBUFFER_SIZE;i++) {
		KPLORA_relayBuffer[i].header.packet_id.ID = 0;
	}
}
