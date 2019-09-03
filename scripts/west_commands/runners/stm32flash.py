# SPDX-License-Identifier: Apache-2.0

'''Runner for flashing with stm32flash.'''

from os import path
import platform

from runners.core import ZephyrBinaryRunner, RunnerCaps

default_device = '/dev/ttyUSB0'
if platform.system() == 'Darwin':
    default_device = '/dev/tty.SLAB_USBtoUART'
    
class Stm32flashBinaryRunner(ZephyrBinaryRunner):
    '''Runner front-end for stm32flash.'''

    def __init__(self, cfg, device, baud='57600',
                 force_binary_parser=False, get_info=False,
                 start_address='0', exec_addr=None,
                 serial_mode='8e1', reset=False, verify=False):
        super(Stm32flashBinaryRunner, self).__init__(cfg)

        self.device = device
        self.baud = baud
        self.force_binary_parser = force_binary_parser
        self.get_info = get_info
        self.start_address = start_address
        self.exec_addr = exec_addr
        self.serial_mode = serial_mode
        self.reset = reset
        self.verify = verify

    @classmethod
    def name(cls):
        return 'stm32flash'

    @classmethod
    def capabilities(cls):
        return RunnerCaps(commands={'flash'})

    @classmethod
    def do_add_parser(cls, parser):

        # required argument(s)
        # none for now

        # optional argument(s)
        parser.add_argument('--device', default=default_device, required=False,
                            help='serial port to flash, default \'' + default_device + '\'')

        parser.add_argument('--baud-rate', default='57600', required=False,
                            choices=['1200', '1800', '2400', '4800', '9600', '19200',
                            '38400', '57600', '115200', '230400', '256000', '460800',
                            '500000', '576000', '921600', '1000000', '1500000', '2000000'],
                            help='serial baud rate, default \'57600\'')

        parser.add_argument('--force-binary-parser', required=False, action='store_true',
                            help='force binary parser')

        parser.add_argument('--get-info', required=False, action='store_true',
                            help='get device information')

        parser.add_argument('--start-address', default='0',required=False,
                            help='specify start address for write operation, default \'0\'')

        parser.add_argument('--execution-addr', default=None,required=False,
                            help='start execution at specified address, default: \'auto\' \
                                  equals same as start address')

        parser.add_argument('--serial-mode', default='8e1',required=False,
                            help='serial port mode, default \'8e1\'')

        parser.add_argument('--reset', default=False, required=False, action='store_true',
                            help='reset device at exit, default False')

        parser.add_argument('--verify', default=False, required=False, action='store_true',
                            help='verify writes, default False')

    @classmethod
    def create(cls, cfg, args):
        return Stm32flashBinaryRunner(cfg, device=args.device,baud=args.baud_rate,
            force_binary_parser=args.force_binary_parser,get_info=args.get_info,
            start_address=args.start_address, exec_addr=args.execution_addr,
            serial_mode=args.serial_mode,reset=args.reset,verify=args.verify)

    def do_run(self, command, **kwargs):
        self.require('stm32flash')

        bin_name = self.cfg.bin_file
        bin_size = path.getsize(bin_name)

        if self.get_info:
            # show device information and exit
            cmd_flash = ['stm32flash', '-b', self.baud,
            '-m', self.serial_mode, self.device]
            self.check_call(cmd_flash)
            return

        cmd_flash = ['stm32flash', '-b', self.baud,
            '-S', self.start_address + ":" + str(bin_size),
            '-m', self.serial_mode, '-w', bin_name]

        if self.exec_addr:
            if self.exec_addr.lower() == 'auto':
                self.exec_addr = self.start_address
            cmd_flash.extend(['-g', self.exec_addr])

        if self.force_binary_parser:
            cmd_flash.extend(['-f'])

        if self.reset:
            cmd_flash.extend(['-R'])

        if self.verify:
            cmd_flash.extend(['-v'])


        cmd_flash.extend([ self.device])
        self.logger.info('Flashing file: {}'.format(bin_name))
        self.check_call(cmd_flash)
        self.logger.info('Board flashed successfully.')
