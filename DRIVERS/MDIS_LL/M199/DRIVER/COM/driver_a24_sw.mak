#***************************  M a k e f i l e  *******************************
#
#         Author: ck
#          $Date: 2007/08/17 13:47:56 $
#      $Revision: 1.1 $
#
#    Description: Makefile definitions for the M199 driver
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: driver_a24_sw.mak,v $
#   Revision 1.1  2007/08/17 13:47:56  CKauntz
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2007 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
#*****************************************************************************

MAK_NAME=m199_a24_sw

MAK_SWITCH=$(SW_PREFIX)MAC_MEM_MAPPED 	\
		   $(SW_PREFIX)MAC_BYTESWAP 	\
		   $(SW_PREFIX)ID_SW			\
		   $(SW_PREFIX)M199_A24

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/desc$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/id_sw$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/oss$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/dbg$(LIB_SUFFIX)	\


MAK_INCL=$(MEN_INC_DIR)/m199_drv.h	\
         $(MEN_INC_DIR)/men_typs.h	\
         $(MEN_INC_DIR)/oss.h		\
         $(MEN_INC_DIR)/mdis_err.h	\
         $(MEN_INC_DIR)/maccess.h	\
         $(MEN_INC_DIR)/desc.h		\
         $(MEN_INC_DIR)/mdis_api.h	\
         $(MEN_INC_DIR)/mdis_com.h	\
         $(MEN_INC_DIR)/modcom.h	\
         $(MEN_INC_DIR)/ll_defs.h	\
         $(MEN_INC_DIR)/ll_entry.h	\
         $(MEN_INC_DIR)/dbg.h		\

MAK_INP1=m199_drv$(INP_SUFFIX)
MAK_INP2=

MAK_INP=$(MAK_INP1) \
        $(MAK_INP2)

