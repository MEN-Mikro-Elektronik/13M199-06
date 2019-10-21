/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  m199_doc.c
 *
 *      \author  ck
 *
 *      \brief   User documentation for M199 module driver
 *
 *     Required: -
 *
 *     \switches -
 *
 *
 *---------------------------------------------------------------------------
 * Copyright 2007-2019, MEN Mikro Elektronik GmbH
 ****************************************************************************/

 /*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*! \mainpage
    This is the documentation of the MDIS low-level driver for the M199 module.

    The M-Module M199 is a very flexible module that gets its specific functions
    through the IP cores implemented inside its on-board FPGA. This MDIS low-level
    driver can be used to set or clear the onboard light emitting diodes or read
    and write the SDRAM. The driver can be used to read or write the Universal
    Submodules EEPROM. Furthermore the header of the flash could be read.
    Attention: The driver requires an MDIS system package that also supports the
    USM functionality. 

    \n
    \section Variants Variants
    The M199 is an A08/A24/D16/D32 M-Module. It can be operated on all M-Module
    carrier boards. In order to support different CPU/carrier board
    combinations and to achieve maximum performance, the M199 driver can be
    built in some variants at compilation time:

    \code
    Driver              Variant Description
    --------            --------------------------------
    Standard            A08 address mode, non-swapped
    _sw 				A08 address mode, swapped
    _a24 				A24 address mode, non-swapped
    _a24_sw             A24 address mode, swapped
    \endcode

    Here are some combinations of MEN CPU and carrier boards together with the
    required variants:

    \code
    CPU                    Carrier Board          Driver Variant
    ----------------       -------------          --------------
    MEN A15	(MPC VME)	   A203N (VME64)		   Standard or _a24
    MEN A15 (MPC VME)      A015B                   _sw or _a24_sw
    MEN A15 (MPC VME)      A201  (VME64)           Standard
	MEN F14 (Pentium CPCI) F205 (CPCI Carrier)	   Standard or _a24
	\endcode

    \n \section FuncDesc Functional Description

    \n \subsection reading Reading and Writing Data
	The driver supports reading and writing data from and to the SDRAM with
	the Getstat and Setstat code M199_BLK_SDRAM.
	The Universal Submodule can be read or written with the Getstat and
	Setstat code M199_BLK_USM_MODULE.
	The driver supports reading the fpga header at the flash with the Getstat
	code M199_BLK_FPGA_HEADER.

	\n \subsection led LED switching
    The driver can set or clear the onboard LEDs. The seven light emitting
    diodes are active low and can be activated or cleared via the
    M199_LED SetStat code and the state of the light emitting diodes
    can be received through the GetStat M199_LED code.

    \n \section interrupts Interrupts
    The driver does not support interrupts from the M-Module.

    \n \section signals Signals
    The driver does not support signals.
    \n

    \n \section id_prom ID PROM
    The M-Module's ID PROM can be checked for validity before the device is
    initialized. You can set the ID_CHECK option in the device descriptor.


    \n \section api_functions Supported API Functions

    <table border="0">
    <tr>
        <td><b>API function</b></td>
        <td><b>Functionality</b></td>
        <td><b>Corresponding low level function</b></td></tr>

    <tr><td>M_open()</td><td>Open device</td><td>M199_Init()</td></tr>

    <tr><td>M_close()     </td><td>Close device             </td>
    <td>M199_Exit())</td></tr>
    <tr><td>M_read()      </td><td>Read from device         </td>
    <td>M199_Read()</td></tr>
    <tr><td>M_write()     </td><td>Write to device          </td>
    <td>M199_Write()</td></tr>
    <tr><td>M_setstat()   </td><td>Set device parameter     </td>
    <td>M199_SetStat()</td></tr>
    <tr><td>M_getstat()   </td><td>Get device parameter     </td>
    <td>M199_GetStat()</td></tr>
    <tr><td>M_getblock()  </td><td>Block read from device   </td>
    <td>M199_BlockRead()</td></tr>
    <tr><td>M_setblock()  </td><td>Block write from device  </td>
    <td>M199_BlockWrite()</td></tr>
    <tr><td>M_errstringTs() </td><td>Generate error message </td>
    <td>-</td></tr>
    </table>

    \n \section descriptor_entries Descriptor Entries

    The low-level driver initialization routine decodes the following entries
    ("keys") in addition to the general descriptor keys:

    <table border="0">
    <tr><td><b>Descriptor entry</b></td>
        <td><b>Description</b></td>
        <td><b>Values</b></td>
    </tr>
    <tr><td>ID_CHECK</td>
        <td>ID_CHECK = U_INT32 0</td>
    	<td>0..1, default: 1</td>
    </tr>
    <tr>
    	<td>LED</td>
        <td>LED = U_INT32 127</td>
        <td>0..127, default: 127</td>
    </tr>
    </table>

    \n \section codes M199 specific Getstat/Setstat codes
    see \ref getstat_setstat_codes "section about Getstat/Setstat codes"

    \n \section programs Overview of provided programs

    \subsection m199_simp  Simple example for using the driver
    m199_simp.c (see example section)
*/

/** \example m199_simp.c */

/*! \page dummy
  \menimages
*/


