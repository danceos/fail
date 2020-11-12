#!/bin/bash -e
if [[ -z "$1" ]]
then
    echo "no binary passed. exiting."
    exit 1
fi

if [[ -z "$2" ]]
then
    set -- $1 $(basename -- "$1")
fi

temp=$(mktemp -d)
libdir=${temp}/"lib_$2"
mkdir -p "$libdir" #<copy the libraries here

#use ldd to resolve the libs and use `patchelf --print-needed to filter out
# "magic" libs kernel-interfacing libs such as linux-vdso.so, ld-linux-x86-65.so or libpthread
# which you probably should not relativize anyway
#join \
    #<(ldd "$1" | awk '{if(substr($3,0,1)=="/") print $1,$3}' | sed '/libc/d' | sort) \
    #<(patchelf --print-needed "$1" | sed '/libc/d' | awk '{print $1"libicui18n.so.60\nlibicuuc.so.60\n"}' | sort ) |cut -d\  -f2 |
#ldd "$1" | awk '{if(substr($3,0,1)=="/") print $1,$3}' | sed '/libc|vdso|ld-linux|libpthread/d' | sort | cut -d\  -f2 |
ldd "$1" | awk '{if(substr($3,0,1)=="/") print $1,$3}' | sed '/libc\|vdso\|ld-linux\|libpthread\|librt\|libdl/d' | sort | cut -d\  -f2 |
#copy the lib selection to ./lib
xargs -d '\n' -I{} cp --copy-contents {} "$libdir"
#copy elf to output
cp $1 ${temp}/$2
#make the relative lib paths override the system lib path
patchelf --set-rpath "\$ORIGIN/lib_$2" "${temp}/$2"
# tar archive the resulting directory
tar -czf $2.tar.gz -C "${temp}" .
