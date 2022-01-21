#!/usr/bin/env python3

import os
import re
import sys

cc = "clang"
cflags = "-Os -std=c99"
ldflags = ""
out = "out/release/"
name = "drte"

devcc = "clang"
devcflags = "-O0 -g -std=c99 -Wall -Wextra -Wmissing-prototypes\
 -fsanitize=address -fno-omit-frame-pointer"
devldflags = "-fsanitize=address -fno-omit-frame-pointer"
devout = "out/devel/"
devbinname = "drte-dev"

testcc = "clang"
testcflags = "-O0 -g -std=c99 -Wall -Wextra -DDRTE_TEST\
 -Wno-implicit-function-declaration -fno-omit-frame-pointer\
 -fsanitize=address"
testldflags = "-fno-omit-frame-pointer -fsanitize=address"
testout = "out/devel/"

source = "src/"
testsource = "tests/"
testout = "tests/out/"
testbin = "tests/bin/"

docout = "doc/"

def print_and_exec(command):
    """Print command and execute it."""
    print(command)
    os.system(command)

def compile_dir(srcdir, ccom, cflags, objdir):
    """Compile all c files in srcdir using ccom with cflags and
    put the objectfiles in objdir."""
    files = os.listdir(srcdir)
    for f in files:
        name, ext = os.path.splitext(f)
        if ext != ".c":
            continue
        com = ccom + " " + cflags +  " -c " + srcdir + f + " -o " + objdir + name + ".o"
        print_and_exec(com)

def link_dir(binname, ccom, ldflags, objdir):
    """Produce a binary named binname, by linking all object files in objdir
    using ccom with ldflags."""
    lcom = ccom + " " + ldflags + " -o " + binname + " " + objdir + "*.o"
    print_and_exec(lcom)

def release():
    compile_dir(source, cc, cflags, out)
    link_dir(name, cc, ldflags, out)

def devel():
    compile_dir(source, devcc, devcflags, devout)
    link_dir(devbinname, devcc, devldflags, devout)


def test():
    compile_dir(source, testcc, testcflags, testout)
    compile_dir(testsource, testcc, testcflags, testout)

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
            print_and_exec(c)

    print("\n\nRunning tests:")
    files = os.listdir(testbin)
    for f in files:
        os.system(testbin + f)

def clean():
    com = "rm " + out + "*.o"
    devcom = "rm " + devout + "*.o"
    testcom = "rm " + testout + "*.o " + testbin + "*"

    print_and_exec(com)
    print_and_exec(devcom)
    print_and_exec(testcom)

def distclean():
    clean()
    com = "rm -r " + docout + "* " + name + " " + devbinname
    print_and_exec(com)

def doc():
    com = "doxygen Doxyfile"
    print_and_exec(com)

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
