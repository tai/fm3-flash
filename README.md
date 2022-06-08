# fm3-flash tool

Flash programmer for MB9AF132L.
Not tested, but may also work with other FM0+/FM3/FM4 microcontrollers with UART bootloader.

While SWD debugger can program the chip without this tool, this tool is needed when
you locked down the chip by (carelessly) enabling deep-sleep mode which disables SWD.
In that case, you will need to rewire the MD[1:0] pin on the chip and erase/reflash it with this tool.

# Usage
```
fm3-flash.py - Flash programmer for Fujitsu/Spansion/Cypress/Infineon FM3 microcontroller
Usage: fm3-flash.py [options] <commands...>
Options:
  -D, --debug <level>: Set debug level
  -S, --skip: Skip loading stage-2 firmware
  -2, --stage2 <file>: Specify BiROM stage-2 firmware (m_flash.9a132l)
  -b, --blocksize <size>: Set block size (512B)
  -f, --flashsize <size>: Set flash size (128KB)
  -r, --rambase <addr>: Set RAM base address (0x20000000)
  -p, --port <port>: Set serial port (/dev/ttyUSB0)
  -s, --speed <baud>: Set UART baud rate (9600)
Example:
  $ fm3-flash.py erase blankcheck write=fw.bin verify
  $ fm3-flash.py flash=fw.bin
  $ fm3-flash.py fw.bin
  $ fm3-flash.py read=dump.bin
  $ fm3-flash.py -p /dev/ttyUSB1 -s 38400 fw.bin
NOTE:
  - May need XTAL OSC for the right baudrate (9600@4MHz, ..., 48000@20MHz).
  - Stage-2 firmwares must be copied from genuine MCU Flash Programmer folder.
  - Only tested with MB9AF132L. Not tested with other models/stage-2 firmwares.
HACK:
  - This tool changes 8th byte of stage-2 firmware to make it work.
```

# NOTE

Bootloader protocol was reverse engineered from how genuine MCU Flash Programmer tool flashes the chip.

This version of Burn-in ROM (BiROM) protocol is quite complex due to its 2-staged process, and
some part of the protocol is just replayed to get the tool working. Also, for unexplainable
reason, genuine tool seems to change the 8th byte of the 2nd-stage bootloader when sending it
over the wire. This is done in this tool as well.

# TODO

Further reverse engineering of 2nd-stage bootloader is needed to create a completely free version of the tool.

# See Also
- https://github.com/shuffle2/fujitsu_rom_com
- https://www.infineon.com/cms/en/design-support/tools/programming-testing/16fx-flash-mcu-programmer/




