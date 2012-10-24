/*
 * Copyright (c) 2011 ARM Limited
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2006 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Kevin Lim
 */

#include <string>

#include "cpu/checker/cpu_impl.hh"
#include "cpu/o3/dyn_inst.hh"
#include "cpu/o3/impl.hh"
#include "cpu/inst_seq.hh"
#include "params/O3Checker.hh"
#include "sim/process.hh"
#include "sim/sim_object.hh"

class MemObject;

template
class Checker<O3CPUImpl>;

/**
 * Specific non-templated derived class used for SimObject configuration.
 */
class O3Checker : public Checker<O3CPUImpl>
{
  public:
    O3Checker(Params *p)
          : Checker<O3CPUImpl>(p)
    { }
};

////////////////////////////////////////////////////////////////////////
//
//  CheckerCPU Simulation Object
//
O3Checker *
O3CheckerParams::create()
{
    O3Checker::Params *params = new O3Checker::Params();
    params->name = name;
    params->numThreads = numThreads;
    params->max_insts_any_thread = 0;
    params->max_insts_all_threads = 0;
    params->max_loads_any_thread = 0;
    params->max_loads_all_threads = 0;
    params->exitOnError = exitOnError;
    params->updateOnError = updateOnError;
    params->warnOnlyOnLoadError = warnOnlyOnLoadError;
    params->clock = clock;
    params->tracer = tracer;
    // Hack to touch all parameters.  Consider not deriving Checker
    // from BaseCPU..it's not really a CPU in the end.
    Counter temp;
    temp = max_insts_any_thread;
    temp = max_insts_all_threads;
    temp = max_loads_any_thread;
    temp = max_loads_all_threads;
    temp++;
    Tick temp2 = progress_interval;
    params->progress_interval = 0;
    temp2++;

    params->itb = itb;
    params->dtb = dtb;
    params->system = system;
    params->cpu_id = cpu_id;
    params->profile = profile;
    params->interrupts = NULL;
    params->workload = workload;

    O3Checker *cpu = new O3Checker(params);
    return cpu;
}
