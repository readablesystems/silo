#!/usr/bin/python

import os
import sys
import subprocess

RUNTIME=30
ROUNDS=5
OTHER_OPTIONS=''
try:
    # will set these until we run out of arguments
    RUNTIME = sys.argv[1]
    ROUNDS = sys.argv[2]
    OTHER_OPTIONS = sys.argv[3]
except: pass

CMD="out-perf.masstree/benchmarks/dbtest --runtime %s %s " % (RUNTIME, OTHER_OPTIONS)
MAKE_CMD='MODE=perf make -j dbtest'
SINGLE_THREADED="--num-threads 1 --scale-factor 1 "
NTHREADS = 24
MANY_THREADED="--num-threads %d --scale-factor %d " % (NTHREADS, NTHREADS)
MBTA="-dmbta "
SILO="--disable-snapshots --disable-gc "
TPCC = '--bench tpcc '
YCSB = '--bench ycsb '

TYPES = ['neworder', 'payment', 'delivery', 'orderstatus', 'stocklevel']

OUTPUT_DIR="benchmarks/data"

DRY_RUN = False

def basic_test(testtype, impl, threads):
    cmd = CMD + threads + impl + testtype
    
    best = 0
    for i in xrange(ROUNDS):
        if DRY_RUN:
            out = "111 222"
            print cmd
        else:
            out = subprocess.check_output(cmd, shell=True)
        data = out.split(' ')[0]
        if float(data) > best:
            best = float(data)

    return str(best)

def run_test(testname, testtype, impl, threads, fileobj):
    fileobj.write("%s\n%s\n" % (testname, basic_test(testtype, impl, threads)))

def us_and_silo(testtype, threads, fileobj):
    run_test('us', testtype, MBTA, threads, fileobj)
    run_test('silo', testtype, SILO, threads, fileobj)


def simple_run(c):
    if DRY_RUN:
        print c
    else:
        os.system(c)

def ignore_run(c):
    simple_run(c + ' > /dev/null 2> /dev/null')

def remake():
    simple_run(MAKE_CMD)

def patch_apply(name):
    simple_run('patch -p1 < ' + name)
    remake()

def patch_revoke(name):
    simple_run('patch -p1 -R < ' + name)

def nosort_apply():
    # this is kinda dumb, we could just pass a -D flag to make probably
    simple_run('patch -p1 < nosort.patch')
    remake()

def nosort_revoke():
    simple_run('patch -p1 -R < nosort.patch')

def nostablesort_apply():
    simple_run('patch -p0 < nostablesort.patch')
    remake()

def nostablesort_revoke():
    simple_run('patch -p0 -R < nostablesort.patch')

def nosort(testtype, threads, fileobj):
    nosort_apply()
    run_test('no sort', testtype, MBTA, threads, fileobj)
    nosort_revoke()

def nostablesort(testtype, threads, fileobj):
    nostablesort_apply()
    run_test('no stable sort', testtype, MBTA, threads, fileobj)
    nostablesort_revoke()



def rmw(testtype, threads, fileobj):
    patch_apply('rmw.patch')
    run_test('readmywrites', testtype, MBTA, threads, fileobj)
    patch_revoke('rmw.patch')

def stdtest(f, testtype, threads, testname):
    singlethr = threads==SINGLE_THREADED
    f.write(testname + '\n')
    us_and_silo(testtype, threads, f)

    rmw(testtype, threads, f)

    if singlethr:
        # for single threaded we also run a no sort test
        nosort(testtype, threads, f)

    remake()

    f.write('\n')

def indiv_type(t):
    idx = TYPES.index(t)
    mix = ''
    for i in xrange(len(TYPES)):
        mix += '100,' if i==idx else '0,'
    mix = mix[:-1] # remove trailing comma
    return '-o --workload-mix=' + mix

def indiv_tpcc_tests(f, threads):
    singlethr = threads==SINGLE_THREADED
    for testname in TYPES:
        f.write(testname + '\n')
        testtype = TPCC + indiv_type(testname)
        us_and_silo(testtype, threads, f)
        rmw(testtype, threads, f)
        if singlethr:
            nosort(testtype, threads, f)
        remake()
        f.write('\n')
    
    
def tpcc():
    f = open(OUTPUT_DIR + '/' + 'tpcc', 'w')

    stdtest(f, TPCC, SINGLE_THREADED, "45-43-4-4-4")
    stdtest(f, TPCC, MANY_THREADED, "%d threads" % NTHREADS)

    indiv_tpcc_tests(f, SINGLE_THREADED)

    f.close()

def ycsb():
    f = open(OUTPUT_DIR + '/' + 'ycsb', 'w')

    stdtest(f, YCSB, SINGLE_THREADED, "default")
    stdtest(f, YCSB, MANY_THREADED, "%d threads" % NTHREADS)

    f.close()

simple_run('mkdir ' + OUTPUT_DIR)
remake()
tpcc()
#ycsb()
