/***********************  I n c l u d e  -  F i l e  ***********************/
/*!
 *        \file  m199_drv.h
 *
 *      \author  ck
 *
 *       \brief  Header file for M199 driver containing
 *               M199 specific status codes and
 *               M199 function prototypes
 *
 *    \switches  _ONE_NAMESPACE_PER_DRIVER_
 *               _LL_DRV_
 *
 *
 *---------------------------------------------------------------------------
 * Copyright 2007-2019, MEN Mikro Elektronik GmbH
 ****************************************************************************/

#ifndef _M199_DRV_H
#define _M199_DRV_H

#ifdef __cplusplus
      extern "C" {
#endif

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/** \name M199 specific Getstat/Setstat standard codes
 *  \anchor getstat_setstat_codes
 */
/**@{*/
#define M199_LED			 (M_DEV_OF+0x00)		/**< G,S: LED signal 			*/
/**@}*/

/** \name M199 specific Getstat/Setstat block codes */
/**@{*/
#define M199_BLK_SDRAM       (M_DEV_BLK_OF+0x00) 	/**< G,S: SDRAM read/write 		*/
#define M199_BLK_USM_MODULE  (M_DEV_BLK_OF+0x01) 	/**< G,S: USM EEPROM read/write */
#define M199_BLK_FPGA_HEADER (M_DEV_BLK_OF+0x02)	/**<  G:  Read Fpga_header 	  	*/
/**@}*/

#define M199_SDRAM_BUFFER_SIZE	512		/**< Buffersize for SDRAM data in words */

/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/
/** blk structure to be sent to setstat / getstat */
typedef struct {
	u_int32  offset;					/**< offset of the sdram base */
	u_int32  size;						/**< size in byte */
	u_int16 buf[M199_SDRAM_BUFFER_SIZE];/**< buffer for the data */
}M199_SDRAM_ACCESS;

#define M199_SDRAM_ACCESS_MINSIZE	\
	(sizeof(M199_SDRAM_ACCESS) - (sizeof(u_int16) * M199_SDRAM_BUFFER_SIZE))


/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
#ifdef _LL_DRV_

# ifdef _ONE_NAMESPACE_PER_DRIVER_
#  define M199_GetEntry    LL_GetEntry
# else
#  ifdef MAC_BYTESWAP
#   ifdef M199_A24
#    define M199_GetEntry   M199_A24_SW_GetEntry
#   else
# 	 define M199_GetEntry   M199_SW_GetEntry
#   endif	/* M199_A24 */
#  else
#   ifdef M199_A24
# 	 define	M199_GetEntry   M199_A24_GetEntry
#   endif	/* M199_A24 */
#  endif	/* MAC_BYTESWAP */
# endif 	/* _ONE_NAMESPACE_PER_DRIVER_ */

extern void M199_GetEntry(LL_ENTRY* drvP);

#endif /* _LL_DRV_ */

/*-----------------------------------------+
|  BACKWARD COMPATIBILITY TO MDIS4         |
+-----------------------------------------*/
#ifndef U_INT32_OR_64
 /* we have an MDIS4 men_types.h and mdis_api.h included */
 /* only 32bit compatibility needed!                     */
 #define INT32_OR_64  int32
 #define U_INT32_OR_64 u_int32
 typedef INT32_OR_64  MDIS_PATH;
#endif /* U_INT32_OR_64 */

#ifdef __cplusplus
      }
#endif

#endif /* _M199_DRV_H */

