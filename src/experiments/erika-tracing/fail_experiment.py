#!/usr/bin/env python

import os,sys
import logging
import imp
import math
import collections
import numpy

tmp_path = "%s/git/versuchung/src"% os.environ["HOME"]
if os.path.exists(tmp_path):
    sys.path.append(tmp_path)


from versuchung.experiment import Experiment
from versuchung.types import String, Bool,List
from versuchung.database import Database
from versuchung.files import File, Directory, Executable
from versuchung.archives import GitArchive
from versuchung.execute import shell
from versuchung.tex import DatarefDict
from threading import Thread

class FailTrace(Experiment):
    inputs = {
        "erika": GitArchive("gitosis@i4git.informatik.uni-erlangen.de:erika"),
        "bochs-runner": Executable("/proj/i4danceos/tools/fail/bochs-experiment-runner.py"),
        "erika-tracing": Executable("/proj/i4danceos/tools/fail/erika-tracing"),
    }
    outputs = {
        "trace": Directory("trace"),
        "elf": File("erika.elf"),
        "iso": File("erika.iso"),
    }

    def run(self):
        logging.info("Cloning ERIKA...")

        with self.erika as erika_path:
            shell("cd %s/examples/x86/coptermock-isorc; make", erika_path)

            self.iso.copy_contents(os.path.join(erika_path, "examples/x86/coptermock-isorc/Debug/erika.iso"))
            self.elf.copy_contents(os.path.join(erika_path, "examples/x86/coptermock-isorc/Debug/Debug/out.elf"))

        shell(("cd %(resultdir)s;  python %(bochs)s -F 50 -i %(iso)s -e %(elf)s -f %(fail)s"
              + " -m 8 -1 --  -Wf,--end-symbol=test_finish -Wf,--start-symbol=EE_oo_StartOS"
              + " -Wf,--trace-file=trace.pb -Wf,--save-symbol=EE_oo_StartOS") % {
              "resultdir": self.trace.path,
              "bochs": self.bochs_runner.path,
              "iso": self.iso.path,
              "elf": self.elf.path,
              "fail": self.erika_tracing.path
              }
        )


class FailImport(Experiment):
    inputs = {
        "trace": FailTrace("FailTrace"),
        "fail-tool-dir": Directory("/proj/i4danceos/tools/fail"),
    }

    def run(self):
        variant = "erika/error-hook"
        for (label, importer, importer_args) in [\
                                ("mem",    "MemoryImporter", []),
                                ("regs",   "RegisterImporter", []),
                                ("ip",     "RegisterImporter", ["--no-gp", "--ip"]),
                                ("flags",  "RegisterImporter", ["--no-gp", "--flags"]),
                                            ]:
            benchmark = label
            logging.info("Importing coredos/%s", benchmark)
            cmdline = "%(path)s/import-trace -v %(variant)s -b %(benchmark)s -i %(importer)s "\
                      + "-t %(trace)s -e %(elf)s %(args)s"
            shell(cmdline %\
                  {"path": self.fail_tool_dir.path,
                   "variant": variant,
                   "benchmark": benchmark,
                   "importer": importer,
                   "trace":  os.path.join(self.trace.trace.path, "trace.pb"),
                   "elf":  self.trace.elf.path,
                   "args": " ".join(importer_args)})
        shell("%s/prune-trace -v %s -b %% -p basic --overwrite",
              self.fail_tool_dir.path, variant)


if __name__ == "__main__":
    import shutil, sys
    if len(sys.argv) > 1:
        action = sys.argv[1]; del sys.argv[1]
    actions = {"trace": FailTrace,
               "import": FailImport}

    if action in actions:
        experiment = actions[action]()
        dirname = experiment(sys.argv)
    else:
        print "No action to be taken, use: %s" % ", ".join(actions.keys())

