FAIL* - FAult Injection Leveraged
=======================================

FAIL* is a fault-injection (FI) framework that provides support for
detailed fault-injection campaigns. It provides carefully-chosen
abstractions simplifying both the implementation of different
simulator/hardware target backends and the reuse of experiment code,
while retaining the ability for deep target-state access for
specialized FI experiments.

The FI experiments are expressed with C++ and programmed against the
FAIL* API which provides full access to the internal state of the
system-under-test. Currently, several hardware/simulator based
backends are available:

- [Bochs](http://bochs.sourceforge.net): Bochs is an x86
  simulator. FAIL* can run and inject bare-metal system images. This
  backend is the most mature and well tested one for undertaking large
  fault-injection campaigns with several millions of injected faults.

- [Gem5](http://www.gem5.org): Gem5 is a simulator for the ARM
  platform, which is supported by FAIL. For Gem5, FAIL supports the
  simulation of the Panda Board, which contains an ARM A9 core.

- [OpenOCD](http://openocd.org/): The OpenOCD project provides a
  unified debugging interface for real embedded ARM platforms. FAIL
  uses this interface to inject faults onto a real hardware
  platform. Several techniques, like SmartHopping, were developed to
  make campaigns with real hardware faster and more feasible.

- Support for other platforms were developed, but are not as mature as
  the other backends: The Trace32 simulator backend for TriCore; An
  x86 backend using QEMU.

During the FI experiment, FAIL provides an interface for triggering
actions on certain instructions, instruction ranges, memory
reads/writes, interrupts, IO operations and timer events. As well the
state of the registers as the main memory of the system-under-test can
be manipulated.

Building FAIL*
--------------

Since FAIL* is a complex research project with many dependencies,
which are listed in `doc/how-to-build.txt`, we provide several
[Docker.io](http://www.docker.com) images that contain all
requirements to build and run a FI campaign with FAIL. After
installing and starting docker on your system, you have to type in the
root directory of the git archive:

    cd scripts/docker; make

As a result, you get three docker images:

- **fail-base**: The docker image contains all requirements to build
  FAIL*. This docker image is built on top of ubuntu 14.10 and
  provides access via SSH (User: fail; Password: fail).

- **fail-generic-tracing**: Upon the fail-base image, this images
  provides the tooling to generate golden-run traces of the
  system-under-test. These traces contain all instruction pointer and
  memory events of the golden run. The FAIL toolchain can record,
  show, and import those traces to an MySQL database.

- **fail-demo**: This image contains as well an generic FAIL
  experiment, a simple system-under-test, and scripts to run an first
  FI campaign within less than 20 minutes. This image has to be
  connected to a MySQL docker image.

Using FAIL*
-----------

After building the docker images, you can run the FAIL demonstration
image by typing:

    cd scripts/docker
    docker pull mysql
    make run-fail-db
    make run-fail-demo
    make ssh-fail-demo

The last command starts a SSH connection to the demonstration
system. Username, as well as password is 'fail'. In the default
configuration, no SSH port is exposed to your normal network
interface. After the connection, you should start in the
`~/fail-targets` directory, which contains a clone of
[https://github.com/danceos/fail-targets](https://github.com/danceos/fail-targets).

The demo system-under test is built and traced with:

    make
    make trace-main

The golden run is traced with:

    make import-main

The fault injection itself is separated into a server process and many
injection workers. Both can be started at once with:

    make server-main &
    make client-main

The results can be displayed on the console or with an browser-based
viewer. By default, port 5000 is exposed to the machine running the
docker instance.

    make result-main
    make resultbrowser


Mailing list
------------
The Fail* developers, and some of its previous and current users, can be
contacted on the
[fail@lists.cs.tu-dortmund.de](mailto:fail@lists.cs.tu-dortmund.de)
mailing list
([subscribe!](https://postamt.cs.uni-dortmund.de/mailman/listinfo/fail)).

Publications about FAIL*
------------------------

 - H. Schirmeier, M. Hoffmann, R. Kapitza, D. Lohmann, and
   O. Spinczyk. FAIL*: Towards a versatile fault-injection experiment
   framework. In G. Mühl, J. Richling, and A. Herkersdorf, editors,
   25th International Conference on Architecture of Computing Systems
   (ARCS '12), Workshop Proceedings, volume 200 of Lecture Notes in
   Informatics, pages 201–210. German Society of Informatics,
   Mar. 2012.
   [PDF](http://danceos.org/publications/VERFE-2012-Schirmeier.pdf)

Selected publications using FAIL*
---------------------------------

- M. Hoffmann, F. Lukas, C. Dietrich, and D. Lohmann. dOSEK: The design and
  implementation of a dependability-oriented static embedded kernel. In
  Proceedings of the 21st IEEE Real-Time and Embedded Technology and
  Applications (RTAS '15), Los Alamitos, CA, USA, Apr. 2015. IEEE Computer
  Society Press.

- M. Hoffmann, P. Ulbrich, C. Dietrich, H. Schirmeier, D. Lohmann, and W.
  Schröder-Preikschat. Experiences with software-based soft-error mitigation
  using AN codes. Software Quality Journal, pages 1–27, 2015.

- I. Stilkerich, P. Taffner, C. Erhardt, C. Dietrich, C. Wawersich, and
  M. Stilkerich. Team Up: Cooperative Memory Management in Embedded
  Systems.  In Proceedings of the 2014 Conference on Compilers,
  Architectures and Synthesis for Embedded Systems (CASES '14). ACM,
  October 2014.

- H. Schirmeier, C. Borchert, and O. Spinczyk. Rapid fault-space exploration by
  evolutionary pruning. In Proceedings of the 33rd International Conference on
  Computer Safety, Reliability and Security (SAFECOMP '14), Lecture Notes in
  Computer Science, pages 17–32. Springer-Verlag, Sept. 2014.

- M. Hoffmann, C. Borchert, C. Dietrich, H. Schirmeier, R. Kapitza, O.
  Spinczyk, and D. Lohmann. Effectiveness of fault detection mechanisms in
  static and dynamic operating system designs. In Proceedings of the 17th IEEE
  International Symposium on Object-Oriented Real-Time Distributed Computing
  (ISORC '14), pages 230–237. IEEE Computer Society Press, June 2014.

- H. Schirmeier, L. Rademacher, and O. Spinczyk. Smart-hopping: Highly efficient
  ISA-level fault injection on real hardware. In Proceedings of the 19th IEEE
  European Test Symposium (ETS '14), pages 69–74. IEEE Computer Society Press,
  May 2014.

- M. Hoffmann, P. Ulbrich, C. Dietrich, H. Schirmeier, D. Lohmann, and W.
  Schröder-Preikschat. A practitioner's guide to software-based soft-error
  mitigation using AN-codes. In Proceedings of the 15th IEEE International
  Symposium on High Assurance Systems Engineering (HASE '14), pages 33–40,
  Miami, Florida, USA, Jan. 2014. IEEE Computer Society Press.

- C. Borchert, H. Schirmeier, and O. Spinczyk. Return-address
  protection in C/C++ code by dependability aspects. In Proceedings of
  the 2nd GI Workshop on Software-Based Methods for Robust Embedded
  Systems (SOBRES '13), Lecture Notes in Informatics. German Society
  of Informatics, Sept. 2013.

- M. Hoffmann, C. Dietrich, and D. Lohmann. Failure by design:
  Influence of the RTOS interface on memory fault resilience. In
  Proceedings of the 2nd GI Workshop on Software-Based Methods for
  Robust Embedded Systems (SOBRES '13), Lecture Notes in
  Informatics. German Society of Informatics, Sept. 2013.

- I. Stilkerich, M. Strotz, C. Erhardt, M. Hoffmann, D. Lohmann, F.
  Scheler, and W. Schröder-Preikschat. A JVM for Soft-Error-Prone
  Embedded Systems. In Proceedings of the 2013 ACM SIGPLAN/SIGBED
  Conference on Languages, Compilers and Tools for Embedded Systems
  (LCTES '13), pages 21–32, June 2013.

- C. Borchert, H. Schirmeier, and O. Spinczyk. Generative
  software-based memory error detection and correction for operating
  system data structures. In Proceedings of the 43nd IEEE/IFIP
  International Conference on Dependable Systems and Networks (DSN
  '13). IEEE Computer Society Press, June 2013.

- H. Schirmeier, I. Korb, O. Spinczyk, and M. Engel. Efficient online
  memory error assessment and circumvention for Linux with RAMpage.
  International Journal of Critical Computer-Based Systems,
  4(3):227–247, 2013. Special Issue on PRDC 2011 Dependable Architecture
  and Analysis.

- B. Döbel, H. Schirmeier, and M. Engel. Investigating the limitations
  of PVF for realistic program vulnerability assessment. In Proceedings
  of the 5rd HiPEAC Workshop on Design for Reliability (DFR '13),
  Berlin, Germany, Jan. 2013.

- C. Borchert, H. Schirmeier, and O. Spinczyk. Protecting the dynamic
  dispatch in C++ by dependability aspects. In Proceedings of the 1st
  GI Workshop on Software-Based Methods for Robust Embedded Systems
  (SOBRES '12), Lecture Notes in Informatics, pages 521–535. German
  Society of Informatics, Sept. 2012.
