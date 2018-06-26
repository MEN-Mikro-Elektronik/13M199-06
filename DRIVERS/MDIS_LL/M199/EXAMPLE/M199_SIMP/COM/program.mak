#***************************  M a k e f i l e  *******************************
#
#         Author: ck
#          $Date: 2007/08/17 13:48:08 $
#      $Revision: 1.1 $
#
#    Description: Makefile definitions for the M199 example program
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#   Revision 1.1  2007/08/17 13:48:08  CKauntz
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2007 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
#*****************************************************************************

MAK_NAME=m199_simp

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
