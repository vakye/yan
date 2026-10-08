#!/bin/bash

if [ ! -d build ]; then
    mkdir -p build;
fi

ShadersDirectory="code/shaders"

pushd $ShadersDirectory > /dev/null
for ShaderFile in *.vert *.frag; do
    glslangValidator --target-env vulkan1.4 -x $ShaderFile -o $ShaderFile.h;
done
popd > /dev/null

SourceFile="code/linux_main.c"
OutputFile="build/yan"

Compiler="clang"

CompileFlags=" \
    -g \
    -O0 \
    -ffreestanding \
    -fpie \
    -nostdlib \
    -std=c11 \
    -Wall -Wextra -Werror -Wpedantic \
    -Wno-unused-parameter \
    -Wno-unused-function \
    -Wno-unused-variable \
    -o $OutputFile"

LinkFlags=" \
    -fuse-ld=lld \
    -Wl,-nostdlib \
    -Wl,--entry,EntryPoint \
    -Wl,-lc \
    -Wl,-lwayland-client"

$Compiler $CompileFlags $SourceFile $LinkFlags

if [ $? == 0 ]; then
    echo $(basename $SourceFile)
fi

