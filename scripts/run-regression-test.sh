#!/bin/bash
declare testsuccess=$true;
declare fail_dir=$PWD/../
declare build_dir=$PWD/../build
declare target_dir=$PWD/../../experiment_targets/regression-test
declare script_dir=$PWD

#Exists build-directory?
cd $fail_dir;
if [ ! -d build ]; then
mkdir build;
cd build;
cmake ..;
fi
cd $build_dir;
cp $target_dir/CMakeCache.txt CMakeCache.txt -f
$script_dir/rebuild-bochs.sh;
cd $target_dir;
$build_dir/bin/fail-client -q;

#diff tracing plugin
diff -q regression-trace.results golden_run/regression-trace.results
if [ ! $? -eq 0 ]
then
    testsuccess=$false;
fi

#diff Main results

diff -q regression-test.results golden_run/regression-test.results
if [ ! $? -eq 0 ]
then
    echo -e  '\033[37;44m Regression-Test FAILED. Look at regression-test.results for more information. \033[0m'
    if $testsuccess;
    then
        echo -e  '\033[37;44m Tracing-Plugin Test SUCCESS. \033[0m'
    else
        echo -e  '\033[37;44m Tracing-Plugin Test FAILED. Look at regression-trace.results for more information. \033[0m'
    fi
else
    if $testsuccess;
    then
        echo -e  '\033[37;44m Regression-Test SUCCESS. \033[0m'
        echo -e  '\033[37;44m Tracing-Plugin Test SUCCESS. \033[0m'
    else
        echo -e  '\033[37;44m Regression-Test FAILED.  Look at regression-test.results for more information. \033[0m'
        echo -e  '\033[37;44m Tracing-Plugin Test FAILED. \033[0m'
    fi
fi

