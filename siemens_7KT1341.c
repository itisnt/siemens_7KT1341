//===========================================================================
// Program: siemens_7KT1341
//===========================================================================
// 02-Jan-2011 bue First Light
// 17-Jan-2011 bue Added new values
// 30-Jul-2012 bue Fixes the output
// 14-Dec-2016 bue Compiled with LibModBus 3.1.4
//                 Option handling added
//                 Fixes in printout
//===========================================================================
// Program get's the first 72 (out of 96) registers from a Siemens 7KT1341
// multicounter device
//
// Not all values will be printed out, only the uncommented below....
//===========================================================================
// Include section
//===========================================================================

#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>
#include <sys/time.h>

#include "unit-test.h"

void usage(char *);

/* Copied from modbus-private.h */
#define HEADER_LENGTH_TCP 7

enum {
    TCP,
    RTU
};

typedef struct {
    // Register 1+2
    int32_t L1_Wirkleistung;      // 1/100 Watt
    // Register 3+4 
    int32_t L1_Effektivspannung;  // mV
    // Register 5+6 
    int32_t L1_Effektivstrom;    // mA
    // Register 7+8 
    int32_t L1_Scheinleistung;    // 1/100 VA
    // Register 9+10 
    int32_t L1_cosalpha;          // 1/1000 grad
    // Register 11+12 
    int32_t L1_L2_Scheinleistung; // mV

    // Register 13+14
    int32_t L2_Wirkleistung;      // 1/100 Watt
    // Register 15+16 
    int32_t L2_Effektivspannung;  // mV
    // Register 17+18 
    int32_t L2_Effektivstrom;    // mA
    // Register 19+20 
    int32_t L2_Scheinleistung;    // 1/100 VA
    // Register 21+22 
    int32_t L2_cosalpha;          // 1/1000 grad
    // Register 23+24 
    int32_t L2_L3_Scheinleistung; // mV

    // Register 25+26 
    int32_t L3_Wirkleistung;      // 1/100 Watt
    // Register 27+28 
    int32_t L3_Effektivspannung;  // mV
    // Register 29+30 
    int32_t L3_Effektivstrom;    // mA
    // Register 31+32 
    int32_t L3_Scheinleistung;    // 1/100 VA
    // Register 33+34 
    int32_t L3_cosalpha;          // 1/1000 grad
    // Register 35+36 
    int32_t L3_L1_Scheinleistung; // mV

    // Register 37+38 
    int32_t Temperatur;           // miligrad
    // Register 39+40 
    int32_t Effektivstrom;        // mA
    // Register 41+42 
    int32_t Frequenz;             // mHz

    // Register 43+44+45+46 
    int64_t L_Summe_Wirkleistung;   // 1/100 Watt
    // Register 47+48+49+50 
    int64_t L_Summe_Scheinleistung; // 1/100 VA
    // Register 51+52
    // int32_t L_Summe_cosalpha;       // 1/1000 grad
    // Register 53+54+55+56
    // int64_t L_Summe_Blindleistung; // 1/100 VAR

    // Register 57+58+59+60
    int64_t Tarif_1_verbrauchte_Wirkenergie ; // mWh
    // Register 61+62+63+64
    // int64_t Tarif_1_gelieferte_Wirkenergie ; // mWh
    // Register 65+66+67+68
    int64_t Tarif_2_verbrauchte_Wirkenergie ; // mWh
    // Register 69+70+71+72
    // int64_t Tarif_2_gelieferte_Wirkenergie ; // mWh

    // Rest noch nicht implementiert (Register 57-96)

} s7kt1341_datagram;


//===========================================================================
// Usage 
//===========================================================================

void usage(char *prg)
{

    printf("\n------------------------------------------------------------------------------\n");
    printf("Daten via Modbus aus einem Siemens 7KT1341 Stromzaehler auslesen\n");
    printf("Zaehler muss via USB_to_RS485 Konverter am System haengen\n");
    printf("------------------------------------------------------------------------------\n");
    printf("Version 0.7\n");
    printf("------------------------------------------------------------------------------\n");
    printf("Usage  : %s -h | -d /dev/[serialport]\n",prg);
    printf("Example: %s -d /dev/ttyUSB0\n",prg);
    printf("------------------------------------------------------------------------------\n");
    printf("Output:\n");
    printf(" L1 Wirkleistung\n");
    printf(" L1 Effektivespannung\n");
    printf(" L1 Effektivestrom\n");
    printf(" L2 Wirkleistung\n");
    printf(" L2 Effektivespannung\n");
    printf(" L2 Effektivestrom\n");
    printf(" L3 Wirkleistung\n");
    printf(" L3 Effektivespannung\n");
    printf(" L3 Effektivestrom\n");
    printf(" Temperatur\n");
    printf(" Effektivstrom\n");
    printf(" Frequenz\n");
    printf(" L Summe Wirkleistung\n");
    printf(" L Summe Scheinleistung\n");
    printf(" Tarif 1 verbrauchte Wirkenergie\n");
    printf(" Tarif 2 verbrauchte Wirkenergie\n");
    printf("\n");
}
//===========================================================================
// Main section
//===========================================================================

int main(int argc, char*argv[])
{
    modbus_t *ctx;
    int rc;
    int i;
    uint16_t tab_reg[128];
    struct tm *local;
    time_t t;
    int64_t temp;
    int option = 0;
    char *dev_name = "";
    uint32_t to_sec;
    uint32_t to_usec;

    t = time(NULL);
    local = localtime(&t);

    s7kt1341_datagram my_data;

    // fprintf(stderr, "ARGC: %d\n", argc);

    while ((option = getopt(argc,argv,"hd:")) != -1)
    {
        switch (option) {
            case 'd' : dev_name = optarg;
                break;
            case 'h' : usage(argv[0]);
                exit(EXIT_FAILURE);
            default : usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (dev_name == "")
    {
        printf("Missing Filename\n");
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    ctx = modbus_new_rtu(dev_name, 19200, 'N', 8, 1);

    modbus_set_debug(ctx, FALSE);
    // modbus_set_debug(ctx, TRUE);

    // Response Timeout seems to be to short by default
    //modbus_get_response_timeout(ctx, &to_sec, &to_usec);
    //printf("Timeouts: sec: %d usec: %d\n",to_sec, to_usec);
    // Timeouts: sec: 0 usec: 500000 
    // Set it to 1s and not 500ms
    modbus_set_response_timeout(ctx, 1, 0);

    modbus_set_error_recovery(ctx, TRUE);

    //
    // Connect to the MODBUS Slave
    //

    modbus_set_slave(ctx, 1);
    rc = modbus_connect(ctx);
    if (rc == -1) {
       fprintf(stderr, "%04d-%02d-%02d %02d:%02d:%02d Unable to connect Reason: %s\n",
         local->tm_year+1900,
         local->tm_mon+1,
         local->tm_mday,
         local->tm_hour,
         local->tm_min,
         local->tm_sec,
         modbus_strerror(errno)
        );
       modbus_close(ctx);
       modbus_free(ctx);
       return -1;
    }

    // 
    // Read the Registers
    // 

    rc = modbus_flush(ctx);
    if (rc == -1) {
       fprintf(stderr, "%04d-%02d-%02d %02d:%02d:%02d Unable to flush input buffer Reason: %s\n",
         local->tm_year+1900,
         local->tm_mon+1,
         local->tm_mday,
         local->tm_hour,
         local->tm_min,
         local->tm_sec,
         modbus_strerror(errno)
        );
       modbus_close(ctx);
       modbus_free(ctx);
       return -1;
    }

    rc = modbus_read_registers(ctx, 0, 72, tab_reg);
    if (rc == -1) {
       fprintf(stderr, "%04d-%02d-%02d %02d:%02d:%02d Unable to read register: %s\n",
         local->tm_year+1900,
         local->tm_mon+1,
         local->tm_mday,
         local->tm_hour,
         local->tm_min,
         local->tm_sec,
         modbus_strerror(errno)
        );
       modbus_close(ctx);
       modbus_free(ctx);
       return -1;
    }

    modbus_close(ctx);
    modbus_free(ctx);

    // Debug 
    //for (i=0;i<73;i++) {
    //   printf("%d: int: %d hex: %x\n",i, tab_reg[i],tab_reg[i]);
    //}

    // Nur die nicht auskommentierten Werte werden berechnet und ausgegeben.
    
    my_data.L1_Wirkleistung      = (tab_reg[0] * 65536) + tab_reg[1];
    my_data.L1_Effektivspannung  = (tab_reg[2] * 65536) + tab_reg[3];
    my_data.L1_Effektivstrom    = (tab_reg[4] * 65536) + tab_reg[5];
    // my_data.L1_Scheinleistung    = (tab_reg[6] * 65536) + tab_reg[7];
    // my_data.L1_cosalpha          = (tab_reg[8] * 65536) + tab_reg[9];
    // my_data.L1_L2_Scheinleistung = (tab_reg[10] * 65536) + tab_reg[11];
    my_data.L2_Wirkleistung      = (tab_reg[12] * 65536) + tab_reg[13];
    my_data.L2_Effektivspannung  = (tab_reg[14] * 65536) + tab_reg[15];
    my_data.L2_Effektivstrom    = (tab_reg[16] * 65536) + tab_reg[17];
    // my_data.L2_Scheinleistung    = (tab_reg[18] * 65536) + tab_reg[19];
    // my_data.L2_cosalpha          = (tab_reg[20] * 65536) + tab_reg[21];
    // my_data.L2_L3_Scheinleistung = (tab_reg[22] * 65536) + tab_reg[23];
    my_data.L3_Wirkleistung      = (tab_reg[24] * 65536) + tab_reg[25];
    my_data.L3_Effektivspannung  = (tab_reg[26] * 65536) + tab_reg[27];
    my_data.L3_Effektivstrom    = (tab_reg[28] * 65536) + tab_reg[29];
    // my_data.L3_Scheinleistung    = (tab_reg[30] * 65536) + tab_reg[31];
    // my_data.L3_cosalpha          = (tab_reg[32] * 65536) + tab_reg[33];
    // my_data.L3_L1_Scheinleistung = (tab_reg[34] * 65536) + tab_reg[35];
    my_data.Temperatur           = (tab_reg[36] * 65536) + tab_reg[37];
    my_data.Effektivstrom        = (tab_reg[38] * 65536) + tab_reg[39];
    my_data.Frequenz             = (tab_reg[40] * 65536) + tab_reg[41];

    my_data.L_Summe_Wirkleistung = (uint64_t)((tab_reg[42] * 65536) + tab_reg[43] ) * 1000000000L;
    my_data.L_Summe_Wirkleistung += ((tab_reg[44] * 65536) + tab_reg[45]);

    my_data.L_Summe_Scheinleistung = (uint64_t)((tab_reg[46] * 65536) + tab_reg[47] ) * 1000000000L;
    my_data.L_Summe_Scheinleistung = ((tab_reg[48] * 65536) + tab_reg[49]);

    my_data.Tarif_1_verbrauchte_Wirkenergie = (uint64_t)((tab_reg[56] * 65536) + tab_reg[57]) * 1000000000L;
    my_data.Tarif_1_verbrauchte_Wirkenergie += ((tab_reg[58] * 65536) + tab_reg[59]);

    my_data.Tarif_2_verbrauchte_Wirkenergie = (uint64_t)((tab_reg[64] * 65536) + tab_reg[65]) * 1000000000L;
    my_data.Tarif_2_verbrauchte_Wirkenergie += ((tab_reg[66] * 65536) + tab_reg[67]);

    // We print the CSV line in parts.

    //
    // No timestamp anymore, will be inserted by the perl script who uses
    // this program
    //
    // printf("\"%04d-%02d-%02d %02d:%02d:%02d\";",
    //   local->tm_year+1900,
    //   local->tm_mon+1,
    //   local->tm_mday,
    //   local->tm_hour,
    //   local->tm_min,
    //   local->tm_sec
    // );

    printf("\"%d\";\"%d\";\"%d\";",
      my_data.L1_Wirkleistung, 
      my_data.L1_Effektivspannung, 
      my_data.L1_Effektivstrom
    );

    printf("\"%d\";\"%d\";\"%d\";",
      my_data.L2_Wirkleistung, 
      my_data.L2_Effektivspannung, 
      my_data.L2_Effektivstrom
    );

    printf("\"%d\";\"%d\";\"%d\";",
      my_data.L3_Wirkleistung, 
      my_data.L3_Effektivspannung, 
      my_data.L3_Effektivstrom
    );

    printf("\"%d\";\"%d\";\"%d\";\"%ld\";\"%ld\";\"%ld\";\"%ld\"",
      my_data.Temperatur, 
      my_data.Effektivstrom, 
      my_data.Frequenz,
      my_data.L_Summe_Wirkleistung,
      my_data.L_Summe_Scheinleistung,
      my_data.Tarif_1_verbrauchte_Wirkenergie,
      my_data.Tarif_2_verbrauchte_Wirkenergie
    );

    printf("\n");

    return 0;
}
