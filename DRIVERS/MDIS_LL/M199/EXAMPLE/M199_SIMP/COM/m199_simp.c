/****************************************************************************
 ************                                                    ************
 ************                   M199_SIMP                        ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file m199_simp.c
 *       \author ck
 *
 *       \brief  Simple example program for the M199 driver
 *
 *               Reads and writes data at the SDRAM
 *				 Reads and writes data at the USM EEPROM
 *				 Shows the state of the LEDs
 *				 Shows the FPGA Header
 *
 *     Required: libraries: mdis_api
 *     \switches (none)
 *
 *
 *---------------------------------------------------------------------------
 * Copyright 2007-2019, MEN Mikro Elektronik GmbH
 ****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <MEN/men_typs.h>
#include <MEN/usr_oss.h>
#include <MEN/usr_utl.h>
#include <MEN/mdis_api.h>
#include <MEN/m199_drv.h>

static const char IdentString[]=MENT_XSTR(MAK_REVISION);

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   TYPDEFS                             |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   EXTERNALS                           |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void PrintError(char *info);
static void PrintLedOverview(int32 value);

/********************************* main ************************************/
/** Program main function
 *
 *  \param argc       \IN  argument counter
 *  \param argv       \IN  argument vector
 *
 *  \return	          success (0) or error (1)
 */
int main(int argc, char *argv[])
{
	MDIS_PATH  path;
	int32      runninglights, value;
	char       *device;
	u_int32    index_i;
	u_int8 	   error;
    char       *errstr, errbuf[40];

	error = 0;

	if ((errstr = UTL_ILLIOPT("flrwueh?", errbuf)))
	{
		printf( "*** ERROR: %s\n", errstr);
		error = 1;
	}
	if (argc < 2 || (UTL_TSTOPT("?")) || (UTL_TSTOPT("h")) || error == 1) {
		printf("Syntax: m199_simp <device> [<options>]\n");
		printf("Function: M199 example for read / write access\n");
		printf("Options:\n");
		printf("  device       device name\n");
		printf("  -f           show fpga header................ [no]      \n");
		printf("  -l           running lights.................. [no]      \n");
		printf("  -r           sdram read access............... [no]      \n");
		printf("  -w           sdram write defiend values ..... [no]      \n");
		printf("  -u           show usm eeprom................. [no]      \n");
		printf("  -e           set usm eeprom to defined values [no]      \n");
		printf("  -? / -h      prints this help................ [no]      \n");
		printf("%s\n", IdentString );
		printf("\n");
		return(1);
	}

	device = argv[1];

	/*--------------------+
    |  open path          |
    +--------------------*/
	if ((path = M_open(device)) < 0) {
		PrintError("open");
		return(1);
	}

	/*--------------------+
    |  switch leds        |
    +--------------------*/
	value = 0;
	if ((M_getstat(path, M199_LED, &value)) < 0) {
			PrintError("getstat");
			goto abort;
	}
	PrintLedOverview(value);
	if ((M_setstat(path, M199_LED, 0x55)) < 0) {
			PrintError("setstat");
			goto abort;
	}
	value = 0;
	if ((M_getstat(path, M199_LED, &value)) < 0) {
			PrintError("getstat");
			goto abort;
	}
	PrintLedOverview(value);

	/*--------------------+
    |  usm eeprom         |
    +--------------------*/
     if(UTL_TSTOPT("u"))	/* read eeprom data */
     {
		M_SG_BLOCK msgblk;
		u_int16 eepromcontent[128];
		msgblk.data = eepromcontent;
		msgblk.size = sizeof(eepromcontent);

		for (index_i = 0; index_i < 128; index_i++){	/* clear buffer */
			eepromcontent[index_i]= 0;
		}
		if ((M_getstat(path, M199_BLK_USM_MODULE, (int32*)&msgblk)) < 0) {
				PrintError("getstat");
				goto abort;
		}
		printf("\nUSM EEPROM content: \n");
		for (index_i = 0; index_i < 128; index_i++)
		{
			if(!(index_i % 8)){
				printf("\n %02lx: ",index_i/8);
			}
			printf(" 0x%04x",eepromcontent[index_i]);
		}
		printf("\n");
	 }

	 if(UTL_TSTOPT("e"))	/* write eeprom data */
     {
		M_SG_BLOCK msgblk;
		u_int16 eepromcontent[128];

		eepromcontent[0]= 0x5553;
		eepromcontent[1]= 0x0000;
		for (index_i = 2; index_i < 128; index_i++)
		{
			eepromcontent[index_i] = 0xffff;
		}

		msgblk.data = eepromcontent;
		msgblk.size = sizeof(eepromcontent);

		if ((M_setstat(path, M199_BLK_USM_MODULE, (U_INT32_OR_64)&msgblk)) < 0) {
				PrintError("setstat");
				goto abort;
		}
		printf("\nUSM-EEPROM has been set to predefined values \n");
	}
	/*--------------------+
    |  fpga header        |
    +--------------------*/
   	if(UTL_TSTOPT("f"))
	{
		M_SG_BLOCK msgblk;
		u_int16 fpgaheadercontent[128];
		msgblk.data = fpgaheadercontent;
		msgblk.size = sizeof(fpgaheadercontent);
		value = 0;
		if ((M_getstat(path, M199_BLK_FPGA_HEADER, (int32*)&msgblk)) < 0) {
				PrintError("getstat");
				goto abort;
		}
		printf("\nFPGA Header content: \n");
		for (index_i = 0; index_i < 128; index_i++)
		{
			if(!(index_i %8)){
				printf("\n %02lx: ",index_i/8);
			}
			printf(" 0x%04x",fpgaheadercontent[index_i]);
		}
		printf("\n");
		printf("\n Filename: ");
    	for (index_i = 0; index_i < 14; index_i++)
    	{
    		printf("%c%c",fpgaheadercontent[index_i+2]>>8,fpgaheadercontent[index_i+2]);
    	}
    	printf("\n\n");
	}
	/*---------------------------------+
    |  read / write data to the SDRAM  |
    +---------------------------------*/
	if(UTL_TSTOPT("r"))			/* read data from SDRAM */
	{
		M_SG_BLOCK msgblk;
		M199_SDRAM_ACCESS blksdram;

		blksdram.offset = 0x00F00000;
		blksdram.size   = M199_SDRAM_BUFFER_SIZE *2; /* Size in bytes */

		/* clear the buffer */
		for (index_i = 0; index_i < M199_SDRAM_BUFFER_SIZE; index_i++) {
			blksdram.buf[index_i] = 0;
		}

		msgblk.size 	= (sizeof(blksdram) - (M199_SDRAM_BUFFER_SIZE*2) + blksdram.size);
		msgblk.data 	= (u_int16*)&blksdram;

		M_getstat(path, M199_BLK_SDRAM,(int32*) &msgblk);

		printf("\nRead content of the SDRAM at the address 0x%08lx\n",blksdram.offset);
		for (index_i = 0; index_i < M199_SDRAM_BUFFER_SIZE; index_i++)
		{
			if(!(index_i %8)){
				printf("\n %08lx: ",index_i / 8);
			}
			printf(" %04x",blksdram.buf[index_i]);
		}
		printf("\n\n");
	}

	/* write data to SDRAM */
	if(UTL_TSTOPT("w"))
	{
		M_SG_BLOCK msgblk;
		M199_SDRAM_ACCESS blksdram;

		blksdram.offset = 0x00F00000;
		blksdram.size   = M199_SDRAM_BUFFER_SIZE *2 ; /* Size in bytes */

		/* fill the buffer for the SDRAM content */
		for (index_i = 0; index_i < M199_SDRAM_BUFFER_SIZE; index_i++)
		{
			blksdram.buf[index_i] = (u_int16)(index_i * 4);
		}
		msgblk.size 	= (sizeof(blksdram) - (M199_SDRAM_BUFFER_SIZE*2) + blksdram.size);
		msgblk.data 	= (u_int16*)&blksdram;

		printf("Write predefined values to SDRAM at the offset 0x%08lx\n",blksdram.offset);
		M_setstat(path, M199_BLK_SDRAM, (U_INT32_OR_64)&msgblk);

		/* clear the buffer */
		for (index_i = 0; index_i < M199_SDRAM_BUFFER_SIZE; index_i++){
			blksdram.buf[index_i] = 0;
		}

		blksdram.size = M199_SDRAM_BUFFER_SIZE * 2;
		M_getstat(path, M199_BLK_SDRAM,(int32*)&msgblk);

		for (index_i = 0; index_i < M199_SDRAM_BUFFER_SIZE; index_i++)
		{
			if(!(index_i %8)){
				printf("\n %08lx: ",index_i / 8);
			}
			printf(" %04x",blksdram.buf[index_i]);
		}
		printf("\n");
	}

	/*--------------------+
    | running lights      |
    +--------------------*/
    runninglights = (UTL_TSTOPT("l") ? 1 : 0);
    if (runninglights)
    {
    	value = 1;
		printf("Running Lights could be stopped by pressing a key \n");
		do {
			if ((M_setstat(path,M199_LED,~(u_int8)value)) < 0) {
				PrintError("setstat");
				goto abort;
			}

			if (value > 127){
				value = 1;
			}
			else {
				value = value << 1;
			}
			UOS_Delay(100);
		} while(runninglights && UOS_KeyPressed() == -1);
	}

	/*--------------------+
    |  cleanup            |
    +--------------------*/
	abort:
	if (M_close(path) < 0)
		PrintError("close");

	return(0);
}

/********************************* PrintError ******************************/
/** Print MDIS error message
 *
 *  \param info       \IN  info string
*/
static void PrintError(char *info)
{
	printf("*** can't %s: %s\n", info, M_errstring(UOS_ErrnoGet()));
}

/********************************* PrintLedOverview ***********************/
/** Print LED states
 *
 *  \param value       \IN  value of the LED register
*/
static void PrintLedOverview(int32 value)
{
	printf("LEDs are switched \n  D1  D2  D3  D4  D5  D6  D7 \n"
		   " %s %s %s %s %s %s %s \n",
		   ((value>>3) & 1)? "OFF":" ON",
		   ((value>>2) & 1)? "OFF":" ON",
		   ((value>>1) & 1)? "OFF":" ON",
		   ((value>>0) & 1)? "OFF":" ON",
		   ((value>>4) & 1)? "OFF":" ON",
		   ((value>>5) & 1)? "OFF":" ON",
		   ((value>>6) & 1)? "OFF":" ON");
}


