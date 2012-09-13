#!/bin/bash
declare testsuccess=$true;
declare build_dir=$PWD/../build
declare target_dir=$PWD/../../experiment_targets/regression-test
declare script_dir=$PWD

cd $build_dir;
if [ ! -d build ]; then
mkdir build;
cd build;
cmake ..;
fi
cp ../../experiment_targets/regression-test/CMakeCache.txt CMakeCache.txt -f
cd $target_dir;
cd $build_dir;
#$script_dir/rebuild-bochs.sh;
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
    echo -e  '\033[37;44m Regression-Test FAILED. \033[0m'
    if $testsuccess;
    then
        echo -e  '\033[37;44m Tracing-Plugin Test SUCCESS. \033[0m'
    else
        echo -e  '\033[37;44m Tracing-Plugin Test FAILED. \033[0m'
    fi
else
    if $testsuccess;
    then
        echo -e  '\033[37;44m Regression-Test SUCCESS. \033[0m'
        echo -e  '\033[37;44m Tracing-Plugin Test SUCCESS. \033[0m'
    else
        echo -e  '\033[37;44m Regression-Test FAILED. \033[0m'
        echo -e  '\033[37;44m Tracing-Plugin Test FAILED. \033[0m'
    fi
fi

