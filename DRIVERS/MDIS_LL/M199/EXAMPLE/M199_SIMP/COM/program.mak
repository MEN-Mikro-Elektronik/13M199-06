#***************************  M a k e f i l e  *******************************
#
#         Author: ck
#
#    Description: Makefile definitions for the M199 example program
#
#-----------------------------------------------------------------------------
#   Copyright 2007-2019, MEN Mikro Elektronik GmbH
#*****************************************************************************

MAK_NAME=m199_simp
# the next line is updated during the MDIS installation
STAMPED_REVISION="_"

DEF_REVISION=MAK_REVISION=$(STAMPED_REVISION)
MAK_SWITCH=$(SW_PREFIX)$(DEF_REVISION)

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)	\
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_utl$(LIB_SUFFIX)	\
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)	\

MAK_INCL=$(MEN_INC_DIR)/m199_drv.h	\
         $(MEN_INC_DIR)/men_typs.h	\
         $(MEN_INC_DIR)/usr_utl.h	\
         $(MEN_INC_DIR)/usr_oss.h	\
         $(MEN_INC_DIR)/mdis_api.h	\

MAK_INP1=$(MAK_NAME)$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
