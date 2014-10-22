# Inherit from docker container that has the fail source code
# prepared, including all tools which are needed to build FAIL*. The
# generic-tracing experiment was already built and the binaries are in
# place (~fail/bin/*)

FROM danceos/fail-generic-tracing
MAINTAINER Christian Dietrich <stettberger@dokucode.de>

# Install Additional Packages
RUN apt-get install -y  \
    python-minimal      \
    grub-common         \
    xorriso             \
    grub-pc-bin         \
    mysql-client        \
    python-flask        \
    python-mysqldb      \
    python-yaml


# Passwort for MySQL Daemon
ADD my.cnf /home/fail/.my.cnf
RUN chown fail /home/fail/.my.cnf

USER fail
WORKDIR /home/fail
RUN echo 'export PATH=$HOME/bin:$PATH' >> ~/.profile;\
    echo 'cd $HOME/fail-targets' >> ~/.profile

RUN git clone https://github.com/danceos/fail-targets.git

WORKDIR fail
RUN mkdir build; cd build; ../configurations/x86_pruning.sh generic-experiment
WORKDIR build

# Make FAIL*
RUN make -j$(getconf _NPROCESSORS_ONLN) || make -j$(getconf _NPROCESSORS_ONLN)
RUN ln -s /home/fail/fail/build/bin/fail-client    /home/fail/bin/generic-experiment-client;    \
    ln -s /home/fail/fail/build/bin/generic-experiment-server   /home/fail/bin/; \
    ln -s /home/fail/fail/tools/analysis/resultbrowser/run.py /home/fail/bin/resultbrowser

# For the resultbrowser, we expose port 5000 to the outside world.
EXPOSE 5000

USER root
