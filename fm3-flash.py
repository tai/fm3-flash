#!/usr/bin/env python

def usage():
  p = os.path.basename(sys.argv[0])
  return f"""
{p} - Flash programmer for Fujitsu/Spansion/Cypress/Infineon FM3 microcontroller
Usage: {p} [options] <commands...>
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
  $ {p} erase blankcheck write=fw.bin verify
  $ {p} flash=fw.bin
  $ {p} fw.bin
  $ {p} read=dump.bin
  $ {p} -p /dev/ttyUSB1 -s 38400 fw.bin
NOTE:
  - May need XTAL OSC for the right baudrate (9600@4MHz, ..., 48000@20MHz).
  - Stage-2 firmwares must be copied from genuine MCU Flash Programmer folder.
  - Only tested with MB9AF132L. Not tested with other models/stage-2 firmwares.
HACK:
  - This tool changes 8th byte of stage-2 firmware to make it work.
""".lstrip()

def help():
  sys.stderr.write(usage())
  sys.exit(0)

import sys
import os
import time
import binascii
from serial import Serial
from struct import pack, unpack
from argparse import ArgumentParser
from IPython import embed

import logging
log = logging.getLogger(__name__)

#
# Notes on command/response code
#
# Response code of FM3 bootloader is pretty complicated.
# It seems to have its root in F2MC-16L microcontroller bootloader,
# where its command byte and response byte has a following structure:
#
#    command byte = 0b01234567
#                     ^^^^command type
#                         ^^^^subtype
#
#   response byte = 0b01234567
#                     ^^^^echoback of a command type it is responding to
#                         ^^^^response code (1=0001=OK, 2=0010=ERROR)
#
# In FM3 microcontroller, this structure changed in following ways:
#
# - Flashing process is now 2-staged, each with different set of commands/responses.
# - Some commands do not respond with the same command type in response.
# - Additional response codes are defined (0=continue, 4=unknown)
#
# In initial stage right after a power reset ("pre-BiROM mode"),
# following commands are supported:
#
# - CMD_PING (0x18), expects response code 0x11
# - CMD_WRITE_RAM (0x00), expects response code 0x01
# - 0xC0, expects response code 0x31
# - 0xE8, expects response code 0x31
#
# To manage its flash memory, it must enter the "post-BiROM mode" by
# loading and running 2nd-stage bootloader on RAM (see FM3Flash.download() method).
# This bootloader firmware comes with the genuine MCU Flash Programmer tool.
# In this stage, following commands are supported:
#
# - CMD_ERASE_START (0x38)
# - CMD_ERASE_SYNC (0x39)
# - CMD_ERASE_END (0x3A)
# - CMD_CHECK_BLANK (0x48)
# - CMD_WRITE_FLASH (0x08)
# - CMD_READ_FLASH (0x28)
#
# These commands generally expects 0x31 as a successful final response,
# and also seem to treat 0x30 as a successful interim response.
#
# Please note that since there is no official doc on the process,
# all names/terms are local to this tool.
#

# commands in pre-BiROM mode
CMD_WRITE_RAM   = 0x00
CMD_PING        = 0x18

# commands in post-BiROM mode
CMD_WRITE_FLASH = 0x08
CMD_READ_FLASH  = 0x28
CMD_ERASE_START = 0x38
CMD_ERASE_SYNC  = 0x39
CMD_ERASE_END   = 0x3A
CMD_CHECK_BLANK = 0x48

######################################################################
# helpers
######################################################################

def die(msg):
  sys.stderr.write(msg)
  sys.stderr.write(os.linesep)
  sys.exit(1)

def to_int(v):
  return int(v, 0)

def hb(s):
  return bytes.fromhex(s)

def b4(v):
  return pack("<I", v)

def b2(v):
  return pack("<H", v)

def b1(v):
  return pack("B", v)

def checksum(buf, init=0x00):
  """Returns a checksum use to check command bytes"""
  return 0xFF & (init + sum(buf))

def crc16(buf):
  """Returns CRC16 used to verify read/write payload"""
  return pack(">H", binascii.crc_hqx(buf, 0))

def bufview(buf, thres=20, head=12, tail=8):
  """Returns a quick view of a buffer"""
  if len(buf) < thres:
    return buf.hex(' ') 
  return buf[:head].hex(' ') + " ... " + buf[-tail:].hex(' ')

######################################################################

class FM3Flash(object):
  def __init__(self, sio, norun=False):
    self.sio = sio
    self.norun = norun
    self.reset()

  def reset(self):
    self.sio.reset_input_buffer()
    self.sio.reset_output_buffer()

  def send(self, cmd=None, buf=None, cksum=False):
    # checksum should includes a command byte (if given)
    chk = checksum(buf, init=cmd or 0) if cksum else None

    # flag to see if cmd is 0x00 or None
    run = cmd is not None

    # show trace
    msg = ""
    if run: msg += f"cmd:{cmd:02X} "
    if buf: msg += f"buf:" + bufview(buf) + " "
    if chk: msg += f"chk:{chk:02X}"
    log.debug(msg)

    # simulation mode
    if self.norun: return

    # send to device
    if run: self.sio.write(b1(cmd))
    if buf: self.sio.write(buf)
    if chk: self.sio.write(b1(chk))

  def recv(self, nr=None, timeout=None):
    # simulation mode
    if self.norun: return

    timeout_orig = self.sio.timeout
    try:
      if timeout:
        self.sio.timeout = timeout
      buf = self.sio.read(nr or self.sio.in_waiting)
      log.debug("got:" + bufview(buf))
      return buf
    finally:
      self.sio.timeout = timeout_orig

  def sendrecv(self, nr, cmd, buf=None, cksum=False):
    self.send(cmd, buf, cksum)
    return self.recv(nr)

  def ping(self, nr=1, interval=0.5):
    """"Check liveness. Only available in pre-BiROM mode"""
    for i in range(nr):
      if self.sendrecv(1, CMD_PING) == b'\x11':
        return True
      time.sleep(interval)
    return False

  def download(self, img, addr=0x20000000):
    """Load and run stage-2 firmware. Enters post-BiROM mode"""
    if not self.ping():
      return False

    # HACK: On UART, this byte seems to get changed from what is on disk
    buf = bytearray(img)
    buf[7] |= 0x03

    # send address and size
    if self.sendrecv(1, CMD_WRITE_RAM, b4(addr) + b4(len(buf)), cksum=True) != b'\x01':
      return False

    # send firmware image
    if self.sendrecv(1, None, buf, cksum=True) !=  b'\x01':
      return False

    if not self.ping():
      return False

    # checking something?
    time.sleep(0.3)
    if self.sendrecv(1, 0xC0, hb('00000000 00000000'), cksum=True) != b'\x31':
      return False

    # execute loaded firmware and enter BiROM mode?
    time.sleep(0.3)
    return self.sendrecv(1, 0xE8) == b'\x31'

  def erase(self, addr_end=0x0001FFFF):
    """Erases flash. Only available in post-BiROM mode"""
    # sync?
    ret = self.sendrecv(10, CMD_ERASE_SYNC, hb('11223344'))
    if ret is None or ret[-1] != 0x31:
      return False

    time.sleep(0.1)

    # trigger erase and check if it started
    if self.sendrecv(1, CMD_ERASE_START, b4(addr_end)) != b'\x30':
      return False

    # wait for completion
    if self.recv(1, timeout=60) != b'\x31':
      return False

    # sync?
    ret = self.sendrecv(10, CMD_ERASE_SYNC, hb('11223344'))
    if ret is None or ret[-1] != 0x31:
      return False

    # closing command?
    ret = self.sendrecv(2, CMD_ERASE_END, hb('00000000'))
    return ret == b'\x30\x31'

  def check_blank(self, addr_start=0, addr_end=0x00020000, regcheck=False):
    """Check blankness. Only available in post-BiROM mode"""
    # check flash
    if self.sendrecv(2, CMD_CHECK_BLANK, b4(addr_start) + b4(addr_end)) != b'\x30\x31':
      return False

    # check security/CR trim register (FIXME: is this same on all models?)
    if regcheck:
      return self.sendrecv(2, CMD_CHECK_BLANK, b4(0x00100000) + b4(0x00100002)) == b'\x30\x31'
    return True

  def write_flash(self, img, bs=512):
    """Write flash memory. Only available in post-BiROM mode"""
    for i in range(0, len(img), bs):
      if self.sendrecv(2, CMD_WRITE_FLASH, b4(i)) != b'\x30\x31':
        return False

      # send chunk+crc, pad with FF
      buf = img[i:i+bs]
      buf += b'\xFF' * (bs - len(buf))
      buf += crc16(buf)
      if self.sendrecv(2, None, buf) != b'\x30\x31':
        return False

    return True

  def read_flash(self, size, bs=512):
    """Read flash memory. Only available in post-BiROM mode"""
    buf = bytearray()
    for i in range(0, size, bs):
      if self.sendrecv(2, CMD_READ_FLASH, b4(i)) != b'\x30\x31':
        return None

      chunk_data = self.recv(bs)
      chunk_csum = self.recv(2)

      if chunk_csum != crc16(chunk_data):
        return None
      if self.recv(1) != b'\x31':
        return None

      buf += chunk_data
    return buf

  def verify(self, img, bs=512):
    buf = self.read_flash(len(img), bs)
    return img == buf[:len(img)]

def loadfile(fn, dirs=None):
  files = [fn] + list(map(lambda d: d + os.path.sep + fn, dirs or []))
  for file in files:
    if os.path.exists(file):
      return open(file, 'rb').read()
  return None

def main(opt):
  sio = Serial(opt.port, opt.speed, timeout=1)
  fm3 = FM3Flash(sio)

  # Load stage-2 bootloader here
  if not opt.skip:
    if not fm3.ping(10):
      die("MCU not responding")
    img = loadfile(opt.stage2, opt.libdir)
    if not img:
      die("Failed to find stage-2 firmware")
    if not fm3.download(img):
      die("Failed to load stage-2 firmware")
    log.info('Loaded stage-2 firmware')

  # process CLI commands
  for arg in opt.args:
    cmd, *cargs = arg.split('=', 2)

    # let filename trigger full reflash process
    if os.path.exists(cmd):
      cmd, *cargs = ('flash', cmd)

    ret = False

    if cmd in ['debug']:
      embed()

    elif cmd in ['ping']:
      ret = fm3.ping(int(cargs[0]) if cargs else 1)

    elif cmd in ['flash']:
      img = open(cargs[0], 'rb').read()
      fm3.erase()
      fm3.check_blank(0, opt.flashsize)
      fm3.write_flash(img, opt.blocksize)
      ret = fm3.verify(img, opt.blocksize)

    elif cmd in ['erase']:
      ret = fm3.erase()

    elif cmd in ['bc', 'blankcheck']:
      ret = fm3.check_blank(0, opt.flashsize, regcheck=True)

    elif cmd in ['w', 'write']:
      img = open(cargs[0], 'rb').read()
      ret = fm3.write_flash(img, opt.blocksize)

    elif cmd in ['r', 'read']:
      buf = fm3.read_flash(opt.flashsize, opt.blocksize)
      ret = open(cargs[0], 'wb').write(buf) > 0

    elif cmd in ['v', 'verify']:
      if cargs:
        img = open(cargs[0], 'rb').read()
      ret = fm3.verify(img, opt.blocksize)

    else:
      help()

    print(cmd, "OK" if ret else "NG")
    if not ret: break

if __name__ == '__main__':
  ap = ArgumentParser()
  ap.print_help = help
  ap.add_argument('-D', '--debug', nargs='?', default='WARN')
  ap.add_argument('-S', '--skip', action='store_true', default=False)
  ap.add_argument('-L', '--libdir', action='append', default=[])
  ap.add_argument('-2', '--stage2', default='m_flash.9a132l')
  ap.add_argument('-b', '--blocksize', type=to_int, default=512)
  ap.add_argument('-f', '--flashsize', type=to_int, default=128*1024)
  ap.add_argument('-r', '--rambase', type=to_int, default=0x20000000)
  ap.add_argument('-s', '--speed', type=int, default=9600)
  ap.add_argument('-p', '--port', default='/dev/ttyUSB0')
  ap.add_argument('args', nargs='*')

  opt = ap.parse_args()
  if not opt.args: help()

  dir = os.getenv('FM3FLASHDIR')
  if dir: opt.libdir += [dir]

  logging.basicConfig(level=eval('logging.' + opt.debug))

  main(opt)
