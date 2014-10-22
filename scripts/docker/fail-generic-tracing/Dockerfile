# Inherit from docker container that has the fail source code prepared,
# including all tools which are needed to build FAIL*
FROM danceos/fail-base
MAINTAINER Christian Dietrich <stettberger@dokucode.de>

USER fail

# Configure the Weather Monitor Experiment
ENV PATH /home/fail/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
WORKDIR /home/fail/fail
RUN mkdir build-tracer; cd build-tracer; ../configurations/x86_pruning.sh generic-tracing
WORKDIR build-tracer

# Make FAIL*
RUN make -j$(getconf _NPROCESSORS_ONLN) || make -j$(getconf _NPROCESSORS_ONLN)

RUN ln -s /home/fail/fail/build-tracer/bin/fail-client    /home/fail/bin/fail-x86-tracing;    \
    ln -s /home/fail/fail/build-tracer/bin/import-trace   /home/fail/bin/;                    \
    ln -s /home/fail/fail/build-tracer/bin/prune-trace    /home/fail/bin/;                    \
    ln -s /home/fail/fail/build-tracer/bin/dump-trace     /home/fail/bin/;                    \
    ln -s /home/fail/fail/build-tracer/bin/convert-trace  /home/fail/bin/;                    \
    cp /home/fail/fail/tools/bochs-experiment-runner/bochs-experiment-runner.py  /home/fail/bin/bochs-experiment-runner.py; \
    chmod a+x /home/fail/bin/bochs-experiment-runner.py;

USER root
