#ifndef __TELEMETRYPACKET_H__
#define __TELEMETRYPACKET_H__

#include <stdint.h>

// MSG types definition
typedef enum{
	PACKET_HEARTBEAT	= 0x00,
	PACKET_LEGACY_FULL	= 0xAA, // kppacket_payload_legacyfull_t
	PACKET_SENSORS		= 0x01, // kppacket_payload_rocket_meas_t
	PACKET_ADCS		    = 0x02,	// kppacket_payload_rocket_ADCS_t
	PACKET_TRACKER		= 0x03,	// kppacket_payload_rocket_tracker_t

	// Custom data packets
	PACKET_CUSTOM_8B	= 0xFC,	// payload = 8B
	PACKET_CUSTOM_16B	= 0xFB,	// payload = 16B
	PACKET_CUSTOM_32B	= 0xFC,	// payload = 32B
	PACKET_CUSTOM_64B	= 0xFD,	// payload = 64B
	PACKET_CUSTOM_128B	= 0xFE,	// payload = 128B
	PACKET_CUSTOM_240B	= 0xFF,	// payload = 240B
} msg_type_e;

// Packet ID definition
typedef union{
	uint16_t ID;
	struct{
		uint8_t msg_ver		: 2;	// Message version (should be 0, for future use)
		uint8_t retransmit	: 1;	// Retransmission flag (1=ON, 0=OFF)

		uint8_t encoded		: 1;	// Data encryption (1=ON, 0=OFF)
		uint8_t redu		: 4;	// Redundant bits for future use
		msg_type_e msg_type;		// Message type
	};
} packet_id_t;

// Header structure
typedef struct __attribute__((__packed__)){
    packet_id_t packet_id;
    uint16_t sender_id;
    uint16_t dest_id;
    uint16_t packet_no;
    uint32_t timestamp_ms;
    uint16_t CRC16;
} kppacket_header_t;

// Min. telemetry for trackers
typedef struct __attribute__((__packed__)){
    uint8_t retransmission_cnt;
    uint8_t vbat_10;    // Battery voltage (in decivolts [V*10])
    int32_t lat;	    // Latitude  [1e-7 deg]
    int32_t lon;	    // Longitude [1e-7 deg]
    int16_t alti_gps;	// Height above ellipsoid [x10m]
    uint8_t sats_fix;	// 6b - sat_cnt + 2b fix
} kppacket_payload_rocket_tracker_t;

// Overal packet definition (header + payload)
typedef struct __attribute__((__packed__)){
    uint8_t packet_len;
    union{
        struct{
            kppacket_header_t header;
            uint8_t  payload[255 - sizeof(kppacket_header_t)];
        };
    };
} kppacket_t;

#endif
