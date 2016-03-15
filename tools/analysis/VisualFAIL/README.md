VisualFAIL*
===========

Guest-system setup
------------------
For analysis with VisualFAIL*, the guest-system ELF binary must be compiled with
debugging information (gcc/g++/clang/clang++ compiler flag "-g").  Note that
debugging information quality/accuracy usually decreases with higher
optimization levels.

Database preparation
--------------------
1. import-trace -t yourtrace.tc -i MemoryImporter
2. import-trace -t yourtrace.tc -i FullTraceImporter
3. import-trace -t yourtrace.tc -i ElfImporter -e yourbinary.elf --objdump $(which objdump) --sources

Step 1 is the prerequisite to run the fault-injection campaign (you may use
other importers as well, e.g., the RegisterImporter).  Steps 2 and 3 are
required for VisualFAIL* to work.

Setup
-----
Copy CONFIGURATION.php.dist to CONFIGURATION.php, edit it, and add your MySQL
database credentials and the result-table name (usually starting with
"result...", "echo SHOW TABLES | mysql yourdatabase" on the command line should
give you the correct table name).

(In a later version of VisualFAIL*, we will probably add automatic loading of
~/.my.cnf if available.)

Running and using VisualFAIL*
-----------------------------
./StartVF.sh requires PHP 5.4.0 or newer and uses its simple built-in web
server.  You can connect to it by using http://localhost:1234 in a web browser
on the same machine.  (If you need to connect from another machine, manually run
"php -S 0.0.0.0:1234 -t .")

Alternatively, VisualFAIL* can be set up using a "real" web server with a recent
PHP version (5.x should suffice).

Once the web server runs, you can use VisualFAIL*:

- Pick a coloring (currently, only "Right margin" really makes sense), a
  benchmark and a variant from the drop-down menus.  Click "Analyze".  Wait.

- Enable the experiment result types you want to highlight by clicking them in
  the top row.

- Scroll down the left column with the disassembled machine code.  Instructions
  that activated faults in the FI experiments, and led to one of the enabled
  result types, are highlighted with red color.  The stronger the coloring, the
  more experiments led to one of these failure modes.  (Actually, not the raw
  number of actual experiments is used, but is weighted with the size of the
  corresponding def/use equivalence class.)

- Click one of the highlighted instructions to display a popup with the actual
  result numbers, and the distribution among the different failure modes.

- Pick a source-code file from the right-hand side pulldown to look into the
  instructions generated from that file.  The right column now contains the
  source code from that file, interspersed with the disassembled machine code
  generated from it.  These disassembled machine-code lines are, just like in
  the disassembly column, clickable to display details.  (The source-code lines
  are currently not clickable, although the mouse cursor indicates otherwise.)
