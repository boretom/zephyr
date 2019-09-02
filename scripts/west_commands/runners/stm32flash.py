# SPDX-License-Identifier: Apache-2.0

'''Runner for flashing with stm32flash.'''

import os
import platform
import sys

from runners.core import ZephyrBinaryRunner, RunnerCaps

default_device = '/dev/ttyUSB0'
if platform.system() == 'Darwin':
    default_device = '/dev/tty.SLAB_USBtoUART'
    
class Stm32flashBinaryRunner(ZephyrBinaryRunner):
    '''Runner front-end for stm32flash.'''

    def __init__(self, cfg, device, start_address='0x0', 
                 baud='57600', firmware=None):
        super(Stm32flashBinaryRunner, self).__init__(cfg)
        self.device = device
        self.start_address = start_address
        self.baud = baud
        self.firmware = cfg.bin_file

    @classmethod
    def name(cls):
        return 'stm32flash'

    @classmethod
    def capabilities(cls):
        return RunnerCaps(commands={'flash'})

    @classmethod
    def do_add_parser(cls, parser):

        # required argument(s)
        # optional argument(s)
        parser.add_argument('--device', default=default_device, required=False,
                            help='serial port to flash, default ' + default_device)

        parser.add_argument('--start-address', default='0x0',required=False,
                            help='specify start address for write operation, default 0x0')

        parser.add_argument('--baud-rate', default='57600', required=False,
                            choices=['1200', '1800', '2400', '4800', '9600', '19200',
                            '38400', '57600', '115200', '230400', '256000', '460800',
                            '500000', '576000', '921600', '1000000', '1500000', '2000000'],
                            help='serial baud rate, default 57600')

        parser.add_argument('--firmware', default='zephyr.bin', required=False,
                            help='firmware file to flash, default {}')

    @classmethod
    def create(cls, cfg, args):
        if args.firmware:
            firmware = args.firmware
        else:
            firmware = cfg.bin_file

        return Stm32flashBinaryRunner(cfg, device=args.device,
            start_address=args.start_address, 
            baud=args.baud_rate, firmware=firmware)

    def do_run(self, command, **kwargs):
        self.require('stm32flash')
        bin_name = self.firmware
        bin_size = os.path.getsize(bin_name)
        if self.baud:
            baud = self.baud
        else:
            baud = '123456'

        cmd_flash = ['stm32flash', '-b', baud,
            '-S', self.start_address + ":" + str(bin_size),
            '-g', self.start_address, '-v',
            '-w', bin_name, self.device]

        self.logger.info('Flashing file: {}'.format(bin_name))
        self.check_call(cmd_flash)
        self.logger.info('Board flashed successfully.')
