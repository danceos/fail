# --------------------------------------------------------------
# This script invokes the gem5 simulator. It is written to be used
# with the FAIL* framework. For further information, see
# http://www.m5sim.org/Simulation_Scripts_Explained
# This script is based on $FAIL/simulators/gem5/configs/example/fs.py
#
# Author: Adrian Böckenkamp
#

# --------------------------------------------------------------
# Required Python modules:
import optparse
import sys

import m5 # required by gem5
from m5.objects import * # dito
from m5.util import addToPath, fatal # some helpers

addToPath('../common') # to find the next modules:
from FSConfig import *
import Simulation # for setWorkCountOptions(), run()
import CacheConfig
import Options # for parsing the command line

# --------------------------------------------------------------
# Configuration phase:
parser = optparse.OptionParser() # TODO: functionality useful?
Options.addCommonOptions(parser) # TODO: remove unused options
Options.addFSOptions(parser) # TODO: dito

(options, args) = parser.parse_args()

if args:
    print "Error: script doesn't take any positional arguments"
    sys.exit(1)
print "[FAIL*] Welcome to the FULL SYSTEM simulation script for gem5!"

# Sets up the low-level system parameter:
class FailArmSystem(System):
    type = 'ArmSystem'
    load_addr_mask = 0xffffffff
    # 0x35 Implementor is '5' from "M5"
    # 0x0 Variant
    # 0xf Architecture from CPUID scheme
    # 0xc00 Primary part number ("c" or higher implies ARM v7)
    # 0x0 Revision
    midr_regval = Param.UInt32(0x350fc000, "MIDR value")
    multi_proc = Param.Bool(True, "Multiprocessor system?")
    boot_loader = Param.String("", "File that contains the boot loader code if any")
    gic_cpu_addr = Param.Addr(0, "Addres of the GIC CPU interface")
    flags_addr = Param.Addr(0, "Address of the flags register for MP booting")

# Creates gem5-related (Python) system objects (hierarchy):
def makeFailArmSystem(mem_mode, machine_type):
    assert machine_type

    self = FailArmSystem()

    # generic system
    mdesc = SysConfig()

    self.readfile = mdesc.script()
    self.iobus = NoncoherentBus()
    self.membus = MemBus()
    self.membus.badaddr_responder.warn_access = "warn"
    self.bridge = Bridge(delay='50ns', nack_delay='4ns')
    self.bridge.master = self.iobus.slave
    self.bridge.slave = self.membus.master

    self.mem_mode = mem_mode

    if machine_type == "RealView_PBX":
        self.realview = RealViewPBX()
    elif machine_type == "RealView_EB":
        self.realview = RealViewEB()
    elif machine_type == "VExpress_ELT":
        self.realview = VExpress_ELT()
    elif machine_type == "VExpress_EMM":
        self.realview = VExpress_EMM()
        self.load_addr_mask = 0xffffffff
    else:
        print "Unknown Machine Type"
        sys.exit(1)

    # No disk image / IDE controller interface stuff.

    # EOT character on UART will end the simulation
    self.realview.uart.end_on_eot = True
    self.physmem = SimpleMemory(range = AddrRange(Addr(mdesc.mem())),
                                zero = True)
    self.physmem.port = self.membus.master
    self.realview.attachOnChipIO(self.membus, self.bridge)
    self.realview.attachIO(self.iobus)
    self.intrctrl = IntrControl()
    self.terminal = Terminal()
    self.vncserver = VncServer()

    self.system_port = self.membus.slave

    return self

# Driver system CPU is always simple... note this is an assignment of
# a class, not an instance.
DriveCPUClass = AtomicSimpleCPU
drive_mem_mode = 'atomic'

# We use the atomic simple CPU model:
class TestCPUClass(AtomicSimpleCPU): pass
# "TestCPUClass" is our CPU class type (sic!), derived from AtomicSimpleCPU
test_mem_mode = 'atomic'
FutureClass = None

TestCPUClass.clock = '2GHz'
DriveCPUClass.clock = '2GHz'

# Extract #CPUs:
np = options.num_cpus

# This script is only designed to be used with ARM targets. However,
# it should be simple to extend it.
if buildEnv['TARGET_ISA'] != "arm":
	fatal("Incapable of building %s full system (only for ARM)! You may want to try fs.py instead.", buildEnv['TARGET_ISA'])

# Create the gem5 component hierarchy (see above for the definition).
# In particular, it specifies system-specific parameters (e.g., Multi-
# processor system flag, memory bus (type), an UART, ...):
test_sys = makeFailArmSystem(test_mem_mode, options.machine_type)

# Set the path to the kernel. Essentially, this the path to your
# experiment target (e.g., the abo-simple-arm.elf file). It should
# contain the bootload code as well.
if options.kernel is not None:
    print "[FAIL*] Using target: " + options.kernel
    test_sys.kernel = options.kernel
else:
	print "[FAIL*] No kernel target given, exiting!"
	sys.exit(1)

test_sys.init_param = options.init_param

# Construct the CPUs (np in total) and connect them to the "test system":
test_sys.cpu = [TestCPUClass(cpu_id=i) for i in xrange(np)]

# Create the memory:
mem_size = SysConfig().mem()

# Create the caches:
if options.caches or options.l2cache:
    test_sys.iocache = IOCache(addr_ranges=[test_sys.physmem.range])
    test_sys.iocache.cpu_side = test_sys.iobus.master
    test_sys.iocache.mem_side = test_sys.membus.slave
else:
    test_sys.iobridge = Bridge(delay='50ns', nack_delay='4ns',
                               ranges = [test_sys.physmem.range])
    test_sys.iobridge.slave = test_sys.iobus.master
    test_sys.iobridge.master = test_sys.membus.slave
    test_sys.failfake = FailFakeDevice(pio_addr=0xF00); #DanceOS
    test_sys.iobus.default = test_sys.failfake.pio #DanceOS

# Sanity check
if options.fastmem and (options.caches or options.l2cache):
    fatal("You cannot use fastmem in combination with caches!")

for i in xrange(np):
    if options.fastmem:
        test_sys.cpu[i].fastmem = True
    if options.checker:
        test_sys.cpu[i].addCheckerCpu()

CacheConfig.config_cache(options, test_sys)

# Setup the root node and connect the hierarchy:
root = Root(full_system=True, system=test_sys)

# TODO: What's this for?
if options.timesync:
    root.time_sync_enable = True

# TODO: What's this for?
if options.frame_capture:
    VncServer.frame_capture = True

# --------------------------------------------------------------
# Simulation phase:
Simulation.setWorkCountOptions(test_sys, options) # see Simulation.py
# In essence, "run(...)" calls m5.instantiate() and, finally, m5.simulate().
Simulation.run(options, root, test_sys, FutureClass)
