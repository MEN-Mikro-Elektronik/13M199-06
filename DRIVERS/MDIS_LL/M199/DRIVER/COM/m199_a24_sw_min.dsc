#************************** MDIS4 device descriptor *************************
#
#        Author: ck
#         $Date: 2007/08/17 13:48:05 $
#     $Revision: 1.1 $
#
#   Description: Metadescriptor for M199 in A24 swapped mode
#
#****************************************************************************

M199_A24_SW_1  {
	#------------------------------------------------------------------------
	#	general parameters (don't modify)
	#------------------------------------------------------------------------
    DESC_TYPE        = U_INT32  1           # descriptor type (1=device)
    HW_TYPE          = STRING   M199_A24_SW # hardware name of device

	#------------------------------------------------------------------------
	#	reference to base board
	#------------------------------------------------------------------------
    BOARD_NAME       = STRING   D201_1      # device name of baseboard
    DEVICE_SLOT      = U_INT32  0           # used slot on baseboard (0..n)

	#------------------------------------------------------------------------
	#	device parameters
	#------------------------------------------------------------------------
}
