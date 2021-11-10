#!/usr/bin/env python3

import os
import re
import sys

cc = "clang"
cflags = "-Os -std=c99 -D_POSIX_C_SOURCE"
ldflags = "-static"
out = "out/release/"
binname = "drte"

devcc = "clang"
devcflags = "-O0 -g -std=c99 -Wall -Wextra -Wmissing-prototypes -fsanitize=address -fno-omit-frame-pointer -D_POSIX_C_SOURCE"
devldflags = "-fsanitize=address -fno-omit-frame-pointer"
devout = "out/devel/"
devbinname = "drte-dev"

testcc = "clang"
testcflags = "-O0 -g -std=c99 -Wall -Wextra -DDRTE_TEST -Wno-implicit-function-declaration -fno-omit-frame-pointer -fsanitize=address -D_POSIX_C_SOURCE"
testldflags = "-fno-omit-frame-pointer -fsanitize=address"
testout = "out/devel/"

source = "src/"
testsource = "tests/"
testout = "tests/out/"
testbin = "tests/bin/"

docout = "doc/"

def release():
    files = os.listdir(source)
    for f in files:
        name, ext = os.path.splitext(f)
        if ext != ".c":
            continue
        com = cc + " " + cflags +  " -c " + source + f + " -o " + out + name + ".o"
        print(com)
        os.system(com)
    lcom = cc + " " + ldflags + " -o " + binname + " " + out + "*.o"
    print(lcom)
    os.system(lcom)

def devel():
    files = os.listdir(source)
    for f in files:
        name, ext = os.path.splitext(f)
        if ext != ".c":
            continue
        com = devcc + " " + devcflags +  " -c " + source + f + " -o " + devout + name + ".o"
        print(com)
        os.system(com)
    lcom = devcc + " " + devldflags + " -o " + devbinname + " " + devout + "*.o"
    print(lcom)
    os.system(lcom)

def test():
    files = os.listdir(source)
    for f in files:
        name, ext = os.path.splitext(f)
        if ext != ".c":
            continue
        c = testcc + " " + testcflags +  " -c " + source + f + " -o " + \
            testout + name + ".o"
        print(c)
        os.system(c)

    files = os.listdir(testsource)
    for f in files:
        name, ext = os.path.splitext(f)
        if ext != ".c":
            continue
        c = testcc + " " + testcflags + " -c " + testsource + f + \
            " -o " + testout + name + ".o"
        print(c)
        os.system(c)

    files = os.listdir(testout)
    r = re.compile("test_(.*).o")
    objects = ""
    for f in files:
        res = r.match(f)
        if res == None and f != "main.o":
            objects = objects + testout + f + " "
    for f in files:
        res = r.match(f)
        if res != None:
            name, ext = os.path.splitext(f)
            c = testcc + " " + testldflags + " -o " + testbin + name + " " + \
                testout + f + " " + objects
            print(c)
            os.system(c)

    print("\n\nRunning tests:")
    files = os.listdir(testbin)
    for f in files:
        os.system(testbin + f)

def clean():
    com = "rm " + out + "*.o"
    devcom = "rm " + devout + "*.o"
    testcom = "rm " + testout + "*.o " + testbin + "*"

    print(com)
    os.system(com)

    print(devcom)
    os.system(devcom)

    print(testcom)
    os.system(testcom)

def distclean():
    clean()
    com = "rm -r " + docout + "* " + binname + " " + devbinname
    print(com)
    os.system(com)

def doc():
    com = "doxygen Doxyfile"
    print(com)
    os.system(com)

def usage(fail = False):
    print("Usage: build.py target")
    print("targets: ")
    print("\trelease - Build drte in a release configuration.")
    print("\tdevel - Build drte in a development configuration.")
    print("\ttest - Build and execute the tests.")
    print("\tdoc - Generate documentation.")
    print("\tclean - Delete build artifacts.")
    print("\tdistclean - Delete bulid artifacts, binaries and documentation.")
    print("\tusage - Print this message.")
    if fail:
        sys.exit(-1)


if len(sys.argv) != 2:
    usage(True)

if sys.argv[1] == "release":
    release()
elif sys.argv[1] == "devel":
    devel()
elif sys.argv[1] == "test":
    test()
elif sys.argv[1] == "clean":
    clean()
elif sys.argv[1] == "distclean":
    distclean()
elif sys.argv[1] == "doc":
    doc()
elif sys.argv[1] == "usage":
    usage()
else:
    usage(True)
