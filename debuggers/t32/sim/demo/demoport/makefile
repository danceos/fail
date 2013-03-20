COPTS = -c -W3 -D_X86_=1 -DWIN32 -Z7 -Od -Gs

all: demoport.dll

demoport.dll: demoport.obj simul.obj
	link -dll -debug:full -debugtype:cv -out:demoport.dll demoport.obj simul.obj -def:demoport.def

.c.obj: ; cl $(COPTS) $*.c


