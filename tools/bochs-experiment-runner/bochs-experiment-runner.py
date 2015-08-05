#!/usr/bin/python

import os, sys
from optparse import OptionParser
from subprocess import *
from tempfile import mkstemp, mkdtemp
import shutil


def parseArgs():
    parser = OptionParser()
    parser.add_option("-e", "--elf-file", dest="elf_file",
                      help="elf file to be executed", metavar="ELF")
    parser.add_option("-i", "--iso-file", dest="iso_file",
                      help="iso file to be executed", metavar="ISO")
    parser.add_option("-f", "--fail-client", dest="fail_client",
                      help="fail-client to be executed", metavar="ISO")
    parser.add_option("-m", "--memory", dest="memory", default="16",
                      help="memory for the bochs VM", metavar="SIZE")

    parser.add_option("-b", "--bios", dest="bios", default="/proj/i4ciao/tools/fail/BIOS-bochs-latest",
                      help="bios image for bochs", metavar="BIOS")

    parser.add_option("-V", "--vgabios", dest="vgabios", default="/proj/i4ciao/tools/fail/vgabios.bin",
                      help="vgabios image for bochs", metavar="VGABIOS")

    parser.add_option("-F", "--freq", dest="freq", default="5",
                      help="frquency in MHZ", metavar="MHZ")

    parser.add_option("-1", "--once",
                      action="store_false", dest="forever", default=True,
                      help="fail-client to be executed")

    parser.add_option("-j", "--jobs",
                      dest="jobs", default="1",
                      help="parallel execution")


    (options, args) = parser.parse_args()

    if not (options.elf_file and options.iso_file and options.fail_client):
        parser.error("elf, iso and fail-client are required")

    return options, args

def execute(options, args, bochsrc, statedir):
    command = "env FAIL_ELF_PATH=%s FAIL_STATEDIR=%s %s -q -f %s %s" %\
    (options.elf_file, statedir, options.fail_client, bochsrc, " ".join(args))
    print "executing: " + command
    p = Popen(command, shell=True, stdout=PIPE, stderr=STDOUT)
    reconnect = 0
    while p.poll() is None:
        line = p.stdout.readline()
        if line is None:
            break
        if "Connection refused" in line:
            reconnect += 1
        print line,
        if reconnect > 10:
            return 1
    p.wait()

    if reconnect > 0:
        return 123
    return p.returncode

def main(options, args):
    bochsrc_args = {
        "memory": options.memory,
        "bios": options.bios,
        "vgabios": options.vgabios,
        "iso": options.iso_file,
        "ips": int(options.freq) * 1000000,
        }

    bochsrc_text = """
config_interface: textconfig
display_library: nogui
romimage: file="{bios}"
cpu: count=1, ips={ips}, reset_on_triple_fault=1, ignore_bad_msrs=1, msrs="msrs.def"
cpuid: mmx=1, sep=1, sse=sse4_2, xapic=1, aes=1, movbe=1, xsave=1, cpuid_limit_winnt=0
memory: guest={memory}, host={memory}
vgaromimage: file="{vgabios}"
vga: extension=vbe
ata0: enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0, irq=14
ata1: enabled=0, ioaddr1=0x170, ioaddr2=0x370, irq=15
ata2: enabled=0, ioaddr1=0x1e8, ioaddr2=0x3e0, irq=11
ata3: enabled=0, ioaddr1=0x168, ioaddr2=0x360, irq=9
ata0-slave: type=cdrom, path="{iso}", status=inserted
boot: cdrom
clock: sync=none, time0=946681200
floppy_bootsig_check: disabled=0
panic: action=fatal
error: action=fatal
info: action=ignore
debug: action=ignore
pass: action=ignore
debugger_log: -
parport1: enabled=0
vga_update_interval: 300000
keyboard_serial_delay: 250
keyboard_paste_delay: 100000
private_colormap: enabled=0
i440fxsupport: enabled=0, slot1=pcivga
""".format(**bochsrc_args)

    bochsrc = mkstemp()
    fd = os.fdopen(bochsrc[0], "w")
    fd.write(bochsrc_text)
    fd.close()
    bochsrc = bochsrc[1]

    statedir = mkdtemp()

    if options.forever:
        while True:
            res = execute(options, args, bochsrc, statedir)
            if res != 0:
                break

        ret = 0
    else:
        ret = execute(options, args, bochsrc, statedir)

    os.unlink(bochsrc)
    shutil.rmtree(statedir)
    sys.exit(ret)

if __name__ == "__main__":
    (options, args) = parseArgs()

    import thread
    i = 1
    # n-1 jobs in threads
    while i < int(options.jobs):
        thread.start_new_thread(main, (options, args))
        i = i + 1

    main(options, args)


