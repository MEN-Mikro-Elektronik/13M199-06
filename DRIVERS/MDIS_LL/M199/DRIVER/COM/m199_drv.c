/*********************  P r o g r a m  -  M o d u l e *************************/
/*!
 *        \file  m199_drv.c
 *
 *      \author  ck
 *
 *      \brief   Low-level driver for M199 module
 *
 *     Required: OSS, DESC, DBG, ID libraries
 *
 *     \switches _ONE_NAMESPACE_PER_DRIVER_
 *
 *
 *---------------------------------------------------------------------------
 * Copyright 2007-2019, MEN Mikro Elektronik GmbH
 ******************************************************************************/

#define _NO_LL_HANDLE		/* ll_defs.h: don't define LL_HANDLE struct */

#include <MEN/men_typs.h>   /* system dependent definitions   */
#include <MEN/maccess.h>    /* hw access macros and types     */
#include <MEN/dbg.h>        /* debug functions                */
#include <MEN/oss.h>        /* oss functions                  */
#include <MEN/desc.h>       /* descriptor functions           */
#include <MEN/modcom.h>     /* ID PROM functions              */
#include <MEN/mdis_api.h>   /* MDIS global defs               */
#include <MEN/mdis_com.h>   /* MDIS common defs               */
#include <MEN/mdis_err.h>   /* MDIS error codes               */
#include <MEN/ll_defs.h>    /* low-level driver definitions   */

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/* general defines */
#define CH_NUMBER			1			/**< Number of device channels */
#define USE_IRQ				FALSE		/**< Interrupt required  */
#define ADDRSPACE_COUNT		1			/**< Number of required address spaces */
#define ADDRSPACEA08_SIZE	256			/**< Size of A08 address space */
#define ADDRSPACEA24_SIZE	0x1000000	/**< Size of A24 address space */

#define MOD_ID_MAGIC		0x5346   	/**< ID PROM magic word */
#define MOD_ID_SIZE			128			/**< ID PROM size [bytes] */
#define MOD_ID				199			/**< ID PROM module ID */

/*	offset in the address space */
#define M199_USER_MODULE	0x00		/**< User Module */
#define M199_IRQ_IRR		0xD0		/**< Interrupt Request Register */
#define M199_IRQ_IER		0xD8		/**< Interrupt Enable Register */
#define M199_LED_REG		0xE0		/**< LED GPIO register*/
#define	M199_FLASH_ADDR		0xF0		/**< Flash address offset */
#define M199_FLASH_DATA		0xF4		/**< Flash data offset */
#define M199_SDRAM_ADDR		0xE8		/**< Indexed SDRAM address register
											 for A08 access mode*/
#define M199_SDRAM_DATA		0xEC		/**< Indexed SDRAM data register
											 for A08 access mode */

/* debug defines */
#define DBG_MYLEVEL			llHdl->dbgLevel   /**< Debug level */
#define DBH					llHdl->dbgHdl     /**< Debug handle */

#define M199_MWRITE_D16(addr,offs,val)  (MWRITE_D16(addr,offs,OSS_SWAP16((val))))
#define M199_MREAD_D16(addr,offs)  		(OSS_SWAP16(MREAD_D16(addr,offs)))
#define M199_MWRITE_D32(addr,offs,val)	{M199_MWRITE_D16(addr,offs, (val & 0xffff)); \
										M199_MWRITE_D16(addr,offs + 2,(val>>16) & 0xffff);}
#define M199_MREAD_D32(addr,offs)		(((u_int32)M199_MREAD_D16(addr,offs))\
									 	+ (((u_int32)M199_MREAD_D16(addr,offs + 2 ))<<16))

/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/
/** low-level handle */
typedef struct {
	/* general */
    int32           memAlloc;		/**< Size allocated for the handle */
    OSS_HANDLE      *osHdl;         /**< OSS handle */
    OSS_IRQ_HANDLE  *irqHdl;        /**< IRQ handle */
    DESC_HANDLE     *descHdl;       /**< DESC handle */
    MACCESS         ma;             /**< HW access handle */
	MDIS_IDENT_FUNCT_TBL idFuncTbl;	/**< ID function table */
	/* debug */
    u_int32         dbgLevel;		/**< Debug level */
	DBG_HANDLE      *dbgHdl;        /**< Debug handle */
	/* misc */
    u_int32         irqCount;       /**< Interrupt counter */
    u_int32         idCheck;		/**< ID check enabled */
} LL_HANDLE;

/* include files which need LL_HANDLE */
#include <MEN/ll_entry.h>   /* low-level driver jump table  */
#include <MEN/m199_drv.h>   /* M199 driver header file */

static const char IdentString[]=MENT_XSTR(MAK_REVISION);

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
static int32 M199_Init(DESC_SPEC *descSpec, OSS_HANDLE *osHdl,
					   MACCESS *ma, OSS_SEM_HANDLE *devSemHdl,
					   OSS_IRQ_HANDLE *irqHdl, LL_HANDLE **llHdlP);
static int32 M199_Exit(LL_HANDLE **llHdlP );
static int32 M199_Read(LL_HANDLE *llHdl, int32 ch, int32 *value);
static int32 M199_Write(LL_HANDLE *llHdl, int32 ch, int32 value);
static int32 M199_SetStat(LL_HANDLE *llHdl,int32 ch, int32 code, INT32_OR_64 value32_or_64);
static int32 M199_GetStat(LL_HANDLE *llHdl, int32 ch, int32 code, INT32_OR_64 *value32_or_64P);
static int32 M199_BlockRead(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
							int32 *nbrRdBytesP);
static int32 M199_BlockWrite(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
							 int32 *nbrWrBytesP);
static int32 M199_Irq(LL_HANDLE *llHdl );
static int32 M199_Info(int32 infoType, ... );

static char* M199_Ident( void );
static int32 M199_Cleanup(LL_HANDLE *llHdl, int32 retCode);

/****************************** M199_GetEntry *********************************/
/** Initialize driver's jump table
 *
 *  \param drvP     \OUT Pointer to the initialized jump table structure
 ******************************************************************************/
extern void M199_GetEntry( LL_ENTRY* drvP )
{
    drvP->init        = M199_Init;
    drvP->exit        = M199_Exit;
    drvP->read        = M199_Read;
    drvP->write       = M199_Write;
    drvP->blockRead   = M199_BlockRead;
    drvP->blockWrite  = M199_BlockWrite;
    drvP->setStat     = M199_SetStat;
    drvP->getStat     = M199_GetStat;
    drvP->irq         = M199_Irq;
    drvP->info        = M199_Info;
}

/******************************** M199_Init ***********************************/
/** Allocate and return low-level handle, initialize hardware
 *
 * The function initializes with the definitions made
 * in the descriptor. The interrupt is disabled.
 *
 * \code
 * Descriptor key        Default          Range
 * --------------------  ---------------  -------------
 * DEBUG_LEVEL_DESC      OSS_DBG_DEFAULT  see dbg.h
 * DEBUG_LEVEL_MBUF      OSS_DBG_DEFAULT  see dbg.h
 * DEBUG_LEVEL           OSS_DBG_DEFAULT  see dbg.h
 * ID_CHECK              TRUE             TRUE/FALSE
 * LED                   127              0..127
 * \endcode
 *
 * The function decodes \ref descriptor_entries "these descriptor entries"
 * in addition to the general descriptor keys.
 *
 *  \param descP      \IN  Pointer to descriptor data
 *  \param osHdl      \IN  OSS handle
 *  \param ma         \IN  HW access handle
 *  \param devSemHdl  \IN  Device semaphore handle
 *  \param irqHdl     \IN  IRQ handle
 *  \param llHdlP     \OUT Pointer to low-level driver handle
 *
 *  \return           \c 0 On success or error code
 ******************************************************************************/
static int32 M199_Init(
    DESC_SPEC       *descP,
    OSS_HANDLE      *osHdl,
    MACCESS         *ma,
    OSS_SEM_HANDLE  *devSemHdl,
    OSS_IRQ_HANDLE  *irqHdl,
    LL_HANDLE       **llHdlP
)
{
    LL_HANDLE *llHdl = NULL;
    u_int32 gotsize;
    int32 retCode;
    u_int32 value;

    /*------------------------------+
    |  prepare the handle           |
    +------------------------------*/
	*llHdlP = NULL;		/* set low-level driver handle to NULL */

	/* alloc */
    llHdl = (LL_HANDLE*) OSS_MemGet(osHdl, sizeof(LL_HANDLE), &gotsize);

    if (llHdl == NULL)
       return(ERR_OSS_MEM_ALLOC);

	/* clear */
    OSS_MemFill(osHdl, gotsize, (char*)llHdl, 0x00);

	/* init */
    llHdl->memAlloc   = gotsize;
    llHdl->osHdl      = osHdl;
    llHdl->irqHdl     = irqHdl;
    llHdl->ma		  = *ma;

    /*------------------------------+
    |  init id function table       |
    +------------------------------*/
	/* driver's ident function */
	llHdl->idFuncTbl.idCall[0].identCall = M199_Ident;
	/* library's ident functions */
	llHdl->idFuncTbl.idCall[1].identCall = DESC_Ident;
	llHdl->idFuncTbl.idCall[2].identCall = OSS_Ident;
	/* terminator */
	llHdl->idFuncTbl.idCall[3].identCall = NULL;

    /*------------------------------+
    |  prepare debugging            |
    +------------------------------*/
	DBG_MYLEVEL = OSS_DBG_DEFAULT;	/* set OS specific debug level */
	DBGINIT((NULL,&DBH));

    /*------------------------------+
    |  scan descriptor              |
    +------------------------------*/
	/* prepare access */
    retCode = DESC_Init(descP, osHdl, &llHdl->descHdl);
    if (retCode ){
		return( M199_Cleanup(llHdl,retCode) );
	}

    /* DEBUG_LEVEL_DESC */
    retCode = DESC_GetUInt32(llHdl->descHdl,
    						 OSS_DBG_DEFAULT,
							 &value,
							 "DEBUG_LEVEL_DESC");
    if (retCode != 0 &&	retCode != ERR_DESC_KEY_NOTFOUND){
		return( M199_Cleanup(llHdl,retCode) );
	}
	DESC_DbgLevelSet(llHdl->descHdl, value);	/* set level */

    /* DEBUG_LEVEL */
    retCode = DESC_GetUInt32(llHdl->descHdl,
    						 OSS_DBG_DEFAULT,
							 &llHdl->dbgLevel,
							 "DEBUG_LEVEL");
    if (retCode != 0 && retCode != ERR_DESC_KEY_NOTFOUND){
		DBGWRT_ERR(( DBH, " *** M199_Init: DESC Error \"DEBUG_LEVEL\" = 0x%08lx\n",retCode));
		return( M199_Cleanup(llHdl,retCode) );
	}
	DBGWRT_1((DBH, "LL - M199_Init\n"));
	DBGWRT_3((DBH,"LL - M199_Init: physAddress is 0x%08p\n", (void*)(llHdl->ma)));

    /* ID_CHECK */
    retCode = DESC_GetUInt32(llHdl->descHdl,
    						 TRUE,
					 		 &llHdl->idCheck,
					 		 "ID_CHECK");
    if (retCode != 0 && retCode != ERR_DESC_KEY_NOTFOUND){
		DBGWRT_ERR(( DBH, " *** M199_Init: DESC Error \"ID_CHECK\" = 0x%08lx\n",retCode));
		return( M199_Cleanup(llHdl,retCode) );
	}

    /*------------------------------+
    |  check module ID              |
    +------------------------------*/
	if (llHdl->idCheck) {
		u_int16 modIdMagic = (u_int16)m_read((U_INT32_OR_64)llHdl->ma, 0);
		u_int16 modId      = (u_int16)m_read((U_INT32_OR_64)llHdl->ma, 1);

		if (modIdMagic != MOD_ID_MAGIC) {
			DBGWRT_ERR((DBH," *** M199_Init: illegal magic=0x%04x\n",modIdMagic));
			retCode = ERR_LL_ILL_ID;
			return( M199_Cleanup(llHdl,retCode) );
		}
		if (modId != MOD_ID) {
			DBGWRT_ERR((DBH," *** M199_Init: illegal id=%d\n",modId));
			retCode = ERR_LL_ILL_ID;
			return( M199_Cleanup(llHdl,retCode) );
		}
	}

    /*------------------------------+
    |  init hardware                |
    +------------------------------*/
	/* LEDs */
    retCode = DESC_GetUInt32(llHdl->descHdl,
    						 0x7F,
					 		 &value,
					 		 "LED");
    if (retCode != 0 && retCode != ERR_DESC_KEY_NOTFOUND){
		DBGWRT_ERR(( DBH, " *** M199_Init: DESC Error \"LED\" = 0x%08lx\n",retCode));
		return( M199_Cleanup(llHdl,retCode) );
	}
	M199_MWRITE_D16( llHdl->ma, M199_LED_REG , (u_int16)(value & 0x7F) );

	*llHdlP = llHdl;	/* set low-level driver handle */

	return(ERR_SUCCESS);
} /* M199_Init */

/****************************** M199_Exit *************************************/
/** De-initialize hardware and clean up memory
 *
 *  The function deinitializes the device.
 *  The interrupt is disabled.
 *
 *  \param llHdlP      \IN  Pointer to low-level driver handle
 *
 *  \return           \c 0 On success or error code
 ******************************************************************************/
static int32 M199_Exit(
   LL_HANDLE    **llHdlP
)
{
    LL_HANDLE *llHdl = *llHdlP;
	int32 error = 0;

    DBGWRT_1((DBH, "LL - M199_Exit\n"));

    /*------------------------------+
    |  de-init hardware             |
    +------------------------------*/
	/* ... */

    /*------------------------------+
    |  clean up memory              |
    +------------------------------*/
	*llHdlP = NULL;		/* set low-level driver handle to NULL */
	error = M199_Cleanup(llHdl,error);

	return(error);
} /* M199_Exit */

/****************************** M199_Read *************************************/
/** Read a value from the device is not supported
 *
 *  The function reads a value out of the SDRAM.
 *
 *  \param llHdl      \IN  Low-level handle
 *  \param ch         \IN  Current channel
 *  \param valueP     \OUT Read value
 *
 *  \return           \c 0 On success or error code
 ******************************************************************************/
static int32 M199_Read(
    LL_HANDLE *llHdl,
    int32 ch,
    int32 *valueP
)
{
    DBGWRT_ERR((DBH, "LL - M199_Read: Not supported"));

	return(ERR_SUCCESS);
}/* M199_Read */

/****************************** M199_Write ************************************/
/** Write a value to the device is not supported
 *
 *  The function writes a value to the SDRAM.
 *
 *  \param llHdl      \IN  Low-level handle
 *  \param ch         \IN  Current channel
 *  \param value      \IN  Read value
 *
 *  \return           \c 0 on success or error code
 ******************************************************************************/
static int32 M199_Write(
    LL_HANDLE *llHdl,
    int32 ch,
    int32 value
)
{
    DBGWRT_ERR((DBH, "LL - M199_Write: Not supported"));

	return(ERR_SUCCESS);
} /* M199_Write */

/****************************** M199_SetStat **********************************/
/** Set the driver status
 *
 *  The driver supports \ref getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *
 *  \param llHdl  	  \IN  Low-level handle
 *  \param code       \IN  \ref getstat_setstat_codes "status code"
 *  \param ch         \IN  Current channel
 *  \param value32_or_64  \IN  Data or
 *                         pointer to block data structure (M_SG_BLOCK) for
 *                         block status codes
 *  \return           \c 0 On success or error code
 ******************************************************************************/
static int32 M199_SetStat(
    LL_HANDLE *llHdl,
    int32  code,
    int32  ch,
    INT32_OR_64  value32_or_64
)
{
    int32	error  = ERR_SUCCESS;
    int32	value  = (int32)value32_or_64;	/* 32bit value		      */
    INT32_OR_64	valueP = value32_or_64;	        /* stores 32/64bit pointer    */
        
    DBGWRT_1((DBH, "LL - M199_SetStat: ch=%d code=0x%04x value=0x%x\n",
			  ch,code,value));

    switch(code) {
        /*--------------------------+
        |  debug level              |
        +--------------------------*/
        case M_LL_DEBUG_LEVEL:
            llHdl->dbgLevel = value;
            break;
        /*--------------------------+
        |  enable interrupts        |
        +--------------------------*/
        case M_MK_IRQ_ENABLE:
			M199_MWRITE_D32(llHdl->ma,M199_IRQ_IER,value);
			break;
        /*--------------------------+
        |  set irq counter          |
        +--------------------------*/
        case M_LL_IRQ_COUNT:
            llHdl->irqCount = value;
            break;
        /*--------------------------+
        |  set usm-module data      |
        +--------------------------*/
        case M199_BLK_USM_MODULE:
        {
            u_int8 n;
            M_SG_BLOCK *blk = (M_SG_BLOCK*)valueP;
            u_int16 *dataP = (u_int16*)blk->data;

			if (blk->size < 256)		/* check buf size */
				return(ERR_LL_USERBUF);
			for(n = 0; n < 128; n++)
			{
	            usm_write((u_int8 *)llHdl->ma,n,(u_int16)(*(dataP+n)));
	            OSS_Delay(llHdl->osHdl, 12);

			}
			break;
		}
        /*--------------------------+
        |  set LEDs                 |
        +--------------------------*/
        case M199_LED:
        {
        	M199_MWRITE_D16( llHdl->ma, M199_LED_REG , (u_int16)(value & 0x7F) );
        	break;
        }
        /*--------------------------+
        |  set sdram data           |
        +--------------------------*/
        case M199_BLK_SDRAM:
        {
        	u_int32 n;
			M_SG_BLOCK *blk = (M_SG_BLOCK*)valueP;
			M199_SDRAM_ACCESS *blksd = (M199_SDRAM_ACCESS*)blk->data;
			if( blk->size < M199_SDRAM_ACCESS_MINSIZE ){
				DBGWRT_ERR(( DBH, " *** M199_Setstat: Block size too small\n"));
				return(ERR_LL_USERBUF);
			}

#ifdef M199_A24
			for (n=0; n < (blksd->size/2); n++){	/* write blksd->size/2 words */
				M199_MWRITE_D16(llHdl->ma, ((blksd->offset) + (n*2)), ((blksd->buf[n])));
			}
#else	/* M199_A24 */
			/* The written address will be autoincremented in A08 mode          *
			*  at the M199_SDRAM_ADDR. The data will be sent to next address    *
			*  if continued writing  											*/
  			M199_MWRITE_D32(llHdl->ma,M199_SDRAM_ADDR,blksd->offset);
			for (n=0; n < (blksd->size/2); n++){	/* write blk->size/2 words */
				M199_MWRITE_D16(llHdl->ma,M199_SDRAM_DATA,(blksd->buf[n]));
			}
#endif /* M199_A24 */
        	break;
        }
        /*--------------------------+
        |  (unknown)                |
        +--------------------------*/
        default:
            error = ERR_LL_UNK_CODE;
    }

	return(error);
} /*M199_SetStat */

/****************************** M199_GetStat **********************************/
/** Get the driver status
 *
 *  The driver supports \ref getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *
 *  \param llHdl      	   \IN  Low-level handle
 *  \param code       	   \IN  \ref getstat_setstat_codes "status code"
 *  \param ch         	   \IN  Current channel
 *  \param value32_or_64P  \IN  Pointer to block data structure (M_SG_BLOCK) for
 *                         	block status codes
 *  \param value32_or_64P  \OUT Data pointer or pointer to block data structure
 *                         	(M_SG_BLOCK) for block status codes
 *
 *  \return           \c 0 On success or error code
 ******************************************************************************/
static int32 M199_GetStat(
    LL_HANDLE *llHdl,
    int32  code,
    int32  ch,
    INT32_OR_64  *value32_or_64P
)
{
    int32	*valueP       = (int32*)value32_or_64P;	/* pointer to 32bit value      */
    INT32_OR_64	*value64P     = value32_or_64P;		/* stores 32/64bit pointer     */
    M_SG_BLOCK	*blk          = (M_SG_BLOCK*)value32_or_64P; /* stores block struct pointer */

    int32 error = ERR_SUCCESS;

    DBGWRT_1((DBH, "LL - M199_GetStat: ch=%d code=0x%04x\n",
			  ch,code));

    switch(code)
    {
        /*--------------------------+
        |  debug level              |
        +--------------------------*/
        case M_LL_DEBUG_LEVEL:
            *valueP = llHdl->dbgLevel;
            break;
        /*--------------------------+
        |  usm-module               |
        +--------------------------*/
        case M199_BLK_USM_MODULE:
        {
        	u_int8 n;
        	u_int16 *dataP = (u_int16*)blk->data;
			if (blk->size < 256)		/* check buf size */
				return(ERR_LL_USERBUF);
			for(n = 0; n < 128; n++)
			{
	            *dataP++ = (u_int16)usm_read((U_INT32_OR_64)llHdl->ma,n);
			}
	        break;
        }
        /*--------------------------+
        |  onboard Led              |
        +--------------------------*/
        case M199_LED:
        {
			*valueP = (M199_MREAD_D16( llHdl->ma, M199_LED_REG ) & 0x7F);
            break;
        }
   		/*--------------------------+
        |  FPGA header              |
        +--------------------------*/
        case M199_BLK_FPGA_HEADER:
        {
			u_int32 index_i = 0;
			u_int16 *dataP = (u_int16*)blk->data;

			if (blk->size < 256)		/* check buf size */
				return(ERR_LL_USERBUF);

           	M199_MWRITE_D32( llHdl->ma, M199_FLASH_ADDR , 0 );
        	M199_MWRITE_D16( llHdl->ma, M199_FLASH_DATA , 0xFFFF );	/* READ-MODE */

        	for (index_i = 0; index_i < 128; index_i++)
        	{
        		M199_MWRITE_D32( llHdl->ma, M199_FLASH_ADDR ,(index_i*2) );
        		*dataP++ = (u_int16)OSS_SWAP16(M199_MREAD_D16( llHdl->ma, M199_FLASH_DATA ));
        	}
          	break;
        }
    	/*--------------------------+
        |  sdram data               |
        +--------------------------*/
        case M199_BLK_SDRAM:
        {
        	u_int32 n;
			M199_SDRAM_ACCESS *blksd = (M199_SDRAM_ACCESS*)blk->data;
			if( blk->size < M199_SDRAM_ACCESS_MINSIZE ){
				DBGWRT_ERR(( DBH, " *** M199_Getstat: Block size too small\n"));
				return(ERR_LL_USERBUF);
			}

#ifdef M199_A24
			for (n=0; n < (blksd->size/2); n++){		/* read blksd->size/2 words */
				(blksd->buf[n]) = (u_int16)M199_MREAD_D16(llHdl->ma, ((blksd->offset) + (n*2)));
			}
#else /* M199_A24 */
			/* The written address will be autoincremented in A08 mode          *
			*  at the M199_SDRAM_ADDR. The data will be read from next address  *
			*  if continued reading  											*/
			M199_MWRITE_D32(llHdl->ma,M199_SDRAM_ADDR,blksd->offset);
			for (n=0; n < (blksd->size/2); n++){		/* read blksd->size/2 words */
				(blksd->buf[n]) = MREAD_D16(llHdl->ma,M199_SDRAM_DATA);
				(blksd->buf[n]) = (u_int16)OSS_SWAP16((blksd->buf[n]));
			}
#endif /* M199_A24 */
        	break;
        }
        /*--------------------------+
        |  number of channels       |
        +--------------------------*/
        case M_LL_CH_NUMBER:
            *valueP = CH_NUMBER;
            break;
        /*--------------------------+
        |  irq counter              |
        +--------------------------*/
        case M_LL_IRQ_COUNT:
            *valueP = llHdl->irqCount;
            break;
        /*--------------------------+
        |  ID PROM check enabled    |
        +--------------------------*/
        case M_LL_ID_CHECK:
            *valueP = llHdl->idCheck;
            break;
        /*--------------------------+
        |   ID PROM size            |
        +--------------------------*/
        case M_LL_ID_SIZE:
            *valueP = MOD_ID_SIZE;
            break;
        /*--------------------------+
        |   ID PROM data            |
        +--------------------------*/
        case M_LL_BLK_ID_DATA:
		{
			u_int8 n;
			u_int16 *dataP = (u_int16*)blk->data;

			if (blk->size < MOD_ID_SIZE)		/* check buf size */
				return(ERR_LL_USERBUF);

			for (n=0; n<MOD_ID_SIZE/2; n++)		/* read MOD_ID_SIZE/2 words */
				*dataP++ = (u_int16)m_read((U_INT32_OR_64)llHdl->ma, n);

			break;
		}
        /*--------------------------+
        |   ident table pointer     |
        |   (treat as non-block!)   |
        +--------------------------*/
        case M_MK_BLK_REV_ID:
           *value64P = (INT32_OR_64)&llHdl->idFuncTbl;
           break;
        /*--------------------------+
        |  unknown                  |
        +--------------------------*/
        default:
            error = ERR_LL_UNK_CODE;
    }

	return(error);
} /* M199_GetStat */

/******************************* M199_BlockRead *******************************/
/** Read a data block from the device is not supported
 *
 *  \param llHdl       \IN  Low-level handle
 *  \param ch          \IN  Current channel
 *  \param buf         \IN  Data buffer
 *  \param size        \IN  Data buffer size
 *  \param nbrRdBytesP \OUT Number of read bytes
 *
 *  \return            \c 0 On success or error code
 ******************************************************************************/
static int32 M199_BlockRead(
     LL_HANDLE *llHdl,
     int32     ch,
     void      *buf,
     int32     size,
     int32     *nbrRdBytesP
)
{
    DBGWRT_ERR((DBH, "LL - M199_BlockRead: Not supported\n"));

	/* return number of read bytes */
	*nbrRdBytesP = 0;

	return(ERR_SUCCESS);
} /* M199_BlockRead */

/****************************** M199_BlockWrite *******************************/
/** Write a data block from the device is not supported
 *
 *  \param llHdl  	   \IN  Low-level handle
 *  \param ch          \IN  Current channel
 *  \param buf         \IN  Data buffer
 *  \param size        \IN  Data buffer size
 *  \param nbrWrBytesP \OUT Number of written bytes
 *
 *  \return            \c 0 On success or error code
 ******************************************************************************/
static int32 M199_BlockWrite(
     LL_HANDLE *llHdl,
     int32     ch,
     void      *buf,
     int32     size,
     int32     *nbrWrBytesP
)
{
    DBGWRT_ERR((DBH, "LL - M199_BlockWrite: Not supported\n"));

	/* return number of written bytes */
	*nbrWrBytesP = 0;

	return(ERR_SUCCESS);
} /* M199_BlockWrite */


/****************************** M199_Irq **************************************/
/** Interrupt service routine is not supported
 *
 *  The interrupt is triggered when ??? occurs.
 *
 *  If the driver can detect the interrupt's cause it returns
 *  LL_IRQ_DEVICE or LL_IRQ_DEV_NOT, otherwise LL_IRQ_UNKNOWN.
 *
 *  \param llHdl  	   \IN  Low-level handle
 *  \return LL_IRQ_DEVICE	IRQ caused by device
 *          LL_IRQ_DEV_NOT  IRQ not caused by device
 *          LL_IRQ_UNKNOWN  Unknown
 ******************************************************************************/
static int32 M199_Irq(
   LL_HANDLE *llHdl
)
{
    IDBGWRT_1((DBH, ">>> M199_Irq:\n"));

	/* ... */

	llHdl->irqCount++;

	return(LL_IRQ_DEV_NOT);		/* say: device supports no interrupt */
} /* M199_Irq */

/****************************** M199_Info *************************************/
/** Get information about hardware and driver requirements
 *
 *  The following info codes are supported:
 *
 * \code
 *  Code                      Description
 *  ------------------------  -----------------------------
 *  LL_INFO_HW_CHARACTER      Hardware characteristics
 *  LL_INFO_ADDRSPACE_COUNT   Number of required address spaces
 *  LL_INFO_ADDRSPACE         Address space information
 *  LL_INFO_IRQ               Interrupt required
 *  LL_INFO_LOCKMODE          Process lock mode required
 * \endcode
 *
 *  The LL_INFO_HW_CHARACTER code returns all address and
 *  data modes (ORed) which are supported by the hardware
 *  (MDIS_MAxx, MDIS_MDxx).
 *
 *  The LL_INFO_ADDRSPACE_COUNT code returns the number
 *  of address spaces used by the driver.
 *
 *  The LL_INFO_ADDRSPACE code returns information about one
 *  specific address space (MDIS_MAxx, MDIS_MDxx). The returned
 *  data mode represents the widest hardware access used by
 *  the driver.
 *
 *  The LL_INFO_IRQ code returns whether the driver supports an
 *  interrupt routine (TRUE or FALSE).
 *
 *  The LL_INFO_LOCKMODE code returns which process locking
 *  mode the driver needs (LL_LOCK_xxx).
 *
 *  \param infoType	   \IN  Info code
 *  \param ...         \IN  Argument(s)
 *
 *  \return            \c 0 On success or error code
 ******************************************************************************/
static int32 M199_Info(
   int32  infoType,
   ...
)
{
    int32   error = ERR_SUCCESS;
    va_list argptr;
    va_start(argptr, infoType );


    switch(infoType) {
		/*-------------------------------+
        |  hardware characteristics      |
        |  (all addr/data modes ORed)    |
        +-------------------------------*/
        case LL_INFO_HW_CHARACTER:
		{
			u_int32 *addrModeP = va_arg(argptr, u_int32*);
			u_int32 *dataModeP = va_arg(argptr, u_int32*);

			*addrModeP = MDIS_MA24 | MDIS_MA08;
			*dataModeP = MDIS_MD08 | MDIS_MD16 | MDIS_MD32 ;
			break;
	    }
		/*-------------------------------+
        |  nr of required address spaces |
        |  (total spaces used)           |
        +-------------------------------*/
        case LL_INFO_ADDRSPACE_COUNT:
		{
			u_int32 *nbrOfAddrSpaceP = va_arg(argptr, u_int32*);

			*nbrOfAddrSpaceP = ADDRSPACE_COUNT;
			break;
	    }
		/*-------------------------------+
        |  address space type            |
        |  (widest used data mode)       |
        +-------------------------------*/
        case LL_INFO_ADDRSPACE:
		{
			u_int32 addrSpaceIndex = va_arg(argptr, u_int32);
			u_int32 *addrModeP = va_arg(argptr, u_int32*);
			u_int32 *dataModeP = va_arg(argptr, u_int32*);
			u_int32 *addrSizeP = va_arg(argptr, u_int32*);

			switch(addrSpaceIndex)
			{
#ifndef M199_A24
				case 0:
					*addrModeP = MDIS_MA08;
					*dataModeP = MDIS_MD16 ;
					*addrSizeP = ADDRSPACEA08_SIZE;
					break;
#else
				case 0:
					*addrModeP = MDIS_MA24;
					*dataModeP = MDIS_MD32;
					*addrSizeP = ADDRSPACEA24_SIZE;
					break;
#endif /* M199_A24 */
				default:
					error = ERR_LL_ILL_PARAM;
			}
			break;
	    }
		/*-------------------------------+
        |   interrupt required           |
        +-------------------------------*/
        case LL_INFO_IRQ:
		{
			u_int32 *useIrqP = va_arg(argptr, u_int32*);

			*useIrqP = USE_IRQ;
			break;
	    }
		/*-------------------------------+
        |   process lock mode            |
        +-------------------------------*/
        case LL_INFO_LOCKMODE:
		{
			u_int32 *lockModeP = va_arg(argptr, u_int32*);

			*lockModeP = LL_LOCK_CALL;
			break;
	    }
		/*-------------------------------+
        |   (unknown)                    |
        +-------------------------------*/
        default:
          error = ERR_LL_ILL_PARAM;
    }

    va_end(argptr);
    return(error);
} /* M199_Info */

/*******************************  M199_Ident  **************************************/
/** Return ident string
 *
 *  \return            Pointer to ident string
 ******************************************************************************/
static char* M199_Ident( void )
{
    return( (char*) IdentString );
} /* M199_Ident*/

/********************************* M199_Cleanup *******************************/
/** Close all handles, free memory and return error code
 *
 *	\warning The low-level handle is invalid after this function is called.
 *
 *  \param llHdl      \IN  Low-level handle
 *  \param retCode    \IN  Return value
 *
 *  \return           \IN   retCode
 ******************************************************************************/
static int32 M199_Cleanup(
   LL_HANDLE    *llHdl,
   int32        retCode
)
{
	DBGWRT_1(( DBH, " Cleanup (M199) \n"));
    /*------------------------------+
    |  close handles                |
    +------------------------------*/
	/* clean up desc */
	if (llHdl->descHdl)
		DESC_Exit(&llHdl->descHdl);

	/* clean up debug */
	DBGEXIT((&DBH));

    /*------------------------------+
    |  free memory                  |
    +------------------------------*/
    /* free my handle */
    OSS_MemFree(llHdl->osHdl, (int8*)llHdl, llHdl->memAlloc);

    /*------------------------------+
    |  return error code            |
    +------------------------------*/
	return(retCode);
} /* M199_Cleanup */



