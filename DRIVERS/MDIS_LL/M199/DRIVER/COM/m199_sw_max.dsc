#************************** MDIS4 device descriptor *************************
#
#        Author: ck
#         $Date: 2007/08/17 13:47:59 $
#     $Revision: 1.1 $
#
#   Description: Metadescriptor for M199 in swapped mode
#
#****************************************************************************

M199_SW_1  {
	#------------------------------------------------------------------------
	#	general parameters (don't modify)
	#------------------------------------------------------------------------
    DESC_TYPE        = U_INT32  1           # descriptor type (1=device)
    HW_TYPE          = STRING   M199_SW     # hardware name of device

	#------------------------------------------------------------------------
	#	reference to base board
	#------------------------------------------------------------------------
    BOARD_NAME       = STRING   D201_1      # device name of baseboard
    DEVICE_SLOT      = U_INT32  0           # used slot on baseboard (0..n)

	#------------------------------------------------------------------------
	#	debug levels (optional)
	#   this keys have only effect on debug drivers
	#------------------------------------------------------------------------
    DEBUG_LEVEL      = U_INT32  0xc0008000  # LL driver
    DEBUG_LEVEL_MK   = U_INT32  0xc0008000  # MDIS kernel
    DEBUG_LEVEL_OSS  = U_INT32  0xc0008000  # OSS calls
    DEBUG_LEVEL_DESC = U_INT32  0xc0008000  # DESC calls
    DEBUG_LEVEL_MBUF = U_INT32  0xc0008000  # MBUF calls

	#------------------------------------------------------------------------
	#	device parameters
	#------------------------------------------------------------------------
    # Define wether M-Module ID-PROM is checked
    # 0 := disable -- ignore IDPROM
    # 1 := enable
    ID_CHECK = U_INT32 0

    # LED D1 - D7 low active 0=OFF, 1=ON
    LED = U_INT32 127
}
