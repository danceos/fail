SET(bochs_configure_params "--enable-a20-pin;--enable-x86-64;--enable-cpu-level=6;--enable-ne2000;--enable-acpi;--enable-pci;--enable-usb;--enable-trace-cache;--enable-fast-function-calls;--enable-host-specific-asms;--enable-readline;--enable-clgd54xx;--enable-fpu;--enable-vmx=2;--enable-monitor-mwait;--enable-cdrom;--enable-sb16=linux;--enable-gdb-stub;--with-nogui" CACHE STRING "")

SET(PLUGINS_ACTIVATED "tracing;checkpoint;randomgenerator" CACHE STRING "")

# Build the import and prune trace tools always
SET(BUILD_IMPORT_TRACE      ON CACHE BOOL "" FORCE)
SET(BUILD_PRUNE_TRACE       ON CACHE BOOL "" FORCE)
SET(BUILD_CONVERT_TRACE     ON CACHE BOOL "" FORCE)
SET(BUILD_DUMP_TRACE        ON CACHE BOOL "" FORCE)
SET(BUILD_LLVM_DISASSEMBLER ON CACHE BOOL "" FORCE)
