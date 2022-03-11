// Copyright (c) 2014, Robert Harris.

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <RFM70.h>
#include <RFM70_impl.h>
#include <radio.h>
#include <unistd.h>
#include <strings.h>


#define	RADIO_ENABLE	7
#define	RADIO_IRQ	0

#define RADIO_CHANNEL          23


#define	SPI_CHANNEL	0
#define	SPI_SPEED	500000

RFM70 radio(0, RADIO_ENABLE, RADIO_IRQ);

char *node_string[7] = {
        "Unknown",
        "Loft",
        "Lounge",
        "Outside",
        "Node 4",
	"Node 5",
	"Node 6"
};

char *type_string[4] = {
        "T_NULL",
        "T_TEMPERATURE",
        "T_PRESSURE",
        "T_VOLTAGE"
};

void
report(int nodeid, unsigned short type, float value, unsigned short seqno)
{
	printf("%3d %s %s %5.2f\n", seqno, node_string[nodeid],
	    type_string[type], value);
	fflush(stdout);
}

void
radio_int(void)
{
        byte status;
        byte rxlen;
        byte pipe_n;
        byte buffer[MAX_PACKET_LEN];
	int i;

	
unsigned short type;
float value;
unsigned short seqno;

        if (((status = radio.read(R_STATUS)) & B_STATUS_RX_DR) == 0) {
                printf("Help: wierdness!\n");
                radio.dump_reg(R_STATUS, status);
        }

        while ((radio.read(R_FIFO_STATUS) & B_FIFO_STATUS_RX_EMPTY) == 0) {

                pipe_n = (radio.read(R_STATUS) & 0xe) >> 1;
                (void) radio.command(C_R_RX_PL_WID, 0, &rxlen, 1);
                (void) radio.command(C_R_RX_PAYLOAD, 0, buffer, rxlen);

/*
		printf("Received %d bytes from node %d: ", rxlen, pipe_n + 1);
		for (i = 0; i < rxlen; i++)
			printf("0x%hhx ", buffer[i]);
		printf("\n");
*/
		
		bcopy(&buffer[0], &type, sizeof (type));
		bcopy(&buffer[2], &value, sizeof (value));
		bcopy(&buffer[6], &seqno, sizeof (seqno));
/*
		printf("type = %hu, value = %f, seqno = %hu\n", type, value, seqno);
		
*/
                if (rxlen != 8)
                        continue;

                //blink();
                report(pipe_n + 1, type, value, seqno);

        }

        radio.write(R_STATUS, status);
}

int
main(int argc, char **argv)
{
	int fd;
	int c;


	if (wiringPiSetup() == -1) {
		perror("wiringPiSetup");
		exit(1);
	}

	fprintf(stdout, "Initialisation successful; board revision %d\n",
	    piBoardRev());

	fflush(stdout);

	if ((fd = wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED)) == -1) {
		perror("wiringPiSPISetup");
		exit(1);
	}

	pinMode(RADIO_ENABLE, OUTPUT);
	pinMode(RADIO_IRQ, INPUT);

	wiringPiISR(RADIO_IRQ, INT_EDGE_FALLING, radio_int);


	radio.begin();

        // Initialise the radio in a power-down state; its SPI bus and
        // registers will still be available. Enable CRC encoding with a
        // one-byte result.
        radio.write(R_CONFIG, B_CONFIG_EN_CRC | B_CONFIG_CRCO);

        // We will configure and enable all data pipes, using an address width
        // of five bytes in order to decrease the probability of bogus matches.
        radio.write(R_SETUP_AW, V_SETUP_AW_5);
        radio.write(R_RX_ADDR_P0, 0, ADDR3, ADDR2, ADDR1, ADDR0);
        radio.write(R_RX_ADDR_P1, 1, ADDR3, ADDR2, ADDR1, ADDR0);
        radio.write(R_RX_ADDR_P2, 2);
        radio.write(R_RX_ADDR_P3, 3);
        radio.write(R_RX_ADDR_P4, 4);
        radio.write(R_RX_ADDR_P5, 5);
        radio.write(R_EN_AA, V_EN_AA_ALL);
        radio.write(R_EN_RXADDR, V_EN_RXADDR_ALL);

        // We'll never send so retrying is meaningless.
        radio.write(R_SETUP_RETR, V_SETUP_RETR_ARC_DIS);
//radio.write(R_SETUP_RETR, V_SETUP_RETR_ARD_1750 | V_SETUP_RETR_ARC_15);

        radio.write(R_RF_CH, RADIO_CHANNEL);

        // Use a data rate of only 1 Mbs in the (untested) expectation that
        // this will increase robustness of the connection. Default to the
        // highest power available.
        radio.write(R_RF_SETUP, V_RF_SETUP_MAGIC | V_RF_SETUP_DR_1MBPS |
            V_RF_SETUP_PWR_5DBM | V_RF_SETUP_LNA_HIGH);

#ifndef nRF24L01
        // We'll be requiring various special features that are set in the
        // FEATURE register, but this is read-only and zero unless enabled.
        // We therefore test its status and toggle it if necessary.
        if (radio.read(R_FEATURE) == 0) {
                radio.write(R_FEATURE, 1);
                if (radio.read(R_FEATURE) == 0) {
                        byte activate = ACTIVATE_FEATURES;
                        (void) radio.command(C_ACTIVATE, 0, &activate, 1);
                }
	}
#endif

        // We'd like dynamic payload length.
        radio.write(R_FEATURE, B_EN_DPL);
        radio.write(R_DYNPD, V_DYNPD_DPL_ALL);

#ifndef nRF24L01
        // Initialise the manufacturer's secret settings.
        radio.config_magic();
#endif

	radio.set_mode(MODE_STANDBY_ONE);
        delay(250);

        // Prepare the radio to receive.
        (void) radio.command(C_FLUSH_RX, 0, NULL, 0);
        radio.set_mode(MODE_RX);

	for (;;)
		sleep(10000);

	return (0);
}

