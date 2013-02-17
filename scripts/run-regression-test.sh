#!/bin/bash
declare testsuccess=true;
declare script_dir=$(dirname $0);
declare fail_dir=$script_dir/../;
declare build_dir=$PWD;
declare target_dir=$script_dir/../../experiment_targets/regression-test;

#find . -name *.ah -not -regex "./src/.*" ! -path $build_dir -exec rm {} \;
cd $build_dir
if [ -f bin/fail-client ]; then
echo -e  '\033[37;44m Build-Environment already exists. Start to compile changes. \033[0m'
$script_dir/rebuild-bochs.sh -;
else
echo -e  '\033[37;44m Start to generate Build-Environment. \033[0m'
rm * -rf;
cmake -DCONFIG_EVENT_BREAKPOINTS:BOOL=ON  -DCONFIG_EVENT_BREAKPOINTS_RANGE:BOOL=ON \
-DCONFIG_EVENT_GUESTSYS:BOOL=ON -DCONFIG_EVENT_INTERRUPT:BOOL=ON \
-DCONFIG_EVENT_IOPORT:BOOL=ON -DCONFIG_EVENT_JUMP:BOOL=ON -DCONFIG_EVENT_MEMREAD:BOOL=ON \
-DCONFIG_EVENT_MEMWRITE:BOOL=ON -DCONFIG_EVENT_TRAP:BOOL=ON -DCONFIG_FAST_BREAKPOINTS:BOOL=OFF \
-DCONFIG_FAST_WATCHPOINTS:BOOL=OFF -DCONFIG_FIRE_INTERRUPTS:BOOL=ON -DCONFIG_SR_REBOOT:BOOL=ON \
-DCONFIG_SR_RESTORE:BOOL=ON -DCONFIG_SR_SAVE:BOOL=ON -DCONFIG_SUPPRESS_INTERRUPTS:BOOL=ON \
-DEXPERIMENTS_ACTIVATED:STRING=regression-test -DPLUGINS_ACTIVATED:STRING='serialoutput;tracing' ..;
echo -e  '\033[37;44m Start to compile. \033[0m'
$script_dir/rebuild-bochs.sh;
fi
if [ -d $build_dir/../build ]; then
echo -e  '\033[37;44m Restore old configuration in build. \033[0m'
cd $build_dir/../build/;
cmake ..;
fi

cd $target_dir;
$build_dir/bin/fail-client -q;

#diff tracing plugin
diff -q regression-trace.results golden_run/regression-trace.results
if [ ! $? -eq 0 ]
then
    testsuccess=false;
fi

#diff Main results

diff -q regression-test.results golden_run/regression-test.results
if [ ! $? -eq 0 ]
then
    echo -e  '\033[37;44m Regression-Test FAILED.\033[0m'
    if $testsuccess;
    then
        echo -e  '\033[37;44m Tracing-Plugin Test SUCCESSFUL. \033[0m'
        echo -e  '\033[37;44m The output of regression-test differs from that of the golden-run.\033[0m'
    else
        echo -e  '\033[37;44m Tracing-Plugin Test FAILED. Look at regression-trace.results for more information. \033[0m'
        echo -e  '\033[37;44m The output of regression-test and the trace differs from those of the golden-run.\033[0m'
    fi
else
    echo -e  '\033[37;44m Regression-Test SUCCESSFUL. \033[0m'
    if $testsuccess;
    then
        echo -e  '\033[37;44m Tracing-Plugin Test SUCCESSFUL. \033[0m'
    else
        echo -e  '\033[37;44m Tracing-Plugin Test FAILED. \033[0m'
        echo -e  '\033[37;44m The regression-test was successful, but the trace differs from that of the golden-run. \033[0m'
    fi
fi

