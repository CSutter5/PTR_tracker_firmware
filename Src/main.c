/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include "trackerHw.h"
#include "gps.h"
#include "telemetryPacket.h"
#include "config.h"
#include "flightState.h"

enum FlightState state;


volatile char uartReceivedByte = '\0';
DataPackageRF_t telemetryPacket;

uint8_t vbat;

void read_analogSensors(uint8_t *voltage) {
	*voltage = (uint32_t)((HW_readADC(3)*1000 / 4095 * 33 * 3) / 1000); //Returns battery voltage * 10
}

void pack_data() {
	telemetryPacket.state = state;
	read_analogSensors(&vbat);
	telemetryPacket.vbat_10 = vbat;
	telemetryPacket.packet_id = 0x00AA; //We specify what type of frame we're sending, in this case the big 48 byte struct
	telemetryPacket.lat = GPS_lat;
	telemetryPacket.lon = GPS_lon;
	telemetryPacket.alti_gps = GPS_alt * 1000; //To mm
	telemetryPacket.sats_fix = ((GPS_fix & 3) << 6) | (GPS_sat_count & 0x3F);
}

void send_data_lora(uint8_t* data) {
	HW_writeLED(1);
	RADIO_sendPacketLoRa(data, sizeof(DataPackageRF_t), 500);
	HW_DelayMs(50);
	HW_writeLED(0);
}

void blink_GPS_startup() {
	HW_writeLED(1);
	HW_DelayMs(500);
	HW_writeLED(0);
	HW_DelayMs(500);
	for(int i=0;i<GPS_sat_count;i++) {
		HW_writeLED(1);
		HW_DelayMs(50);
		HW_writeLED(0);
		HW_DelayMs(300);
	}
	HW_DelayMs(1000);
}

int main(void)
{
	state = STARTUP;
	HW_trackerHwInit();
	RADIO_init();
	RADIO_modeLORA(TRACKER_FREQUENCY_0, TRACKER_TXPOWER_LOW);
    __disable_irq();
    GPS_sendCmd(PMTK_RESET);
	GPS_sendCmd(PMTK_SET_GPGGA);
	__enable_irq();

	state = WAIT_FOR_FIX;
	int i;
	while(GPS_sat_count < 5) {
		blink_GPS_startup();
	}
	state = OPERATION;
	while(state = OPERATION) {
		pack_data();
		send_data_lora(&telemetryPacket);
		HW_DelayMs(TRACKER_TRANSMISSION_SPACING);
	}
}

void USART1_IRQHandler(void) {
	if(USART1->ISR & USART_ISR_RXNE_RXFNE) {
		uartReceivedByte = USART1->RDR;
		GPS_parse(uartReceivedByte);
	}
}
