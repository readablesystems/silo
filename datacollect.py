#!/usr/bin/python

import os
import sys
import subprocess

try:
    FILENAME = sys.argv[1]
except:
    print sys.argv[0], 'outputfile [runtime=30] [rounds=5] [optional test arguments]'
    sys.exit(0)
RUNTIME=30
ROUNDS=5
OTHER_OPTIONS=''
try:
    if sys.argv[1] == '-h' or sys.argv[1] == '--help':
        print sys.argv[0], 'outputfile [runtime=30] [rounds=5] [optional test arguments]'
        sys.exit(0)
    # will set these until we run out of arguments
    RUNTIME = int(sys.argv[2])
    ROUNDS = int(sys.argv[3])
    OTHER_OPTIONS = sys.argv[4]
except: pass

CMD="out-perf.masstree/benchmarks/dbtest --runtime %s %s " % (RUNTIME, OTHER_OPTIONS)
CMD_RMW="out-perf.rmw.masstree/benchmarks/dbtest --runtime %s %s " % (RUNTIME, OTHER_OPTIONS)
MAKE_CMD_TEMPL='MODE=perf %s make -j dbtest'
MAKE_CMD = MAKE_CMD_TEMPL % ''
MAKE_CMD_RMW = MAKE_CMD_TEMPL % 'STO_RMW=1'
SINGLE_THREADED="--num-threads 1 --scale-factor 1 "
NTHREADS = 24
MANY_THREADS = lambda n: "--num-threads %d --scale-factor %d " % (n, n)
MANY_THREADED= MANY_THREADS(NTHREADS)
MBTA="-dmbta "
SILO="--disable-snapshots "
TPCC = '--bench tpcc '
YCSB = '--bench ycsb '

TYPES = ['neworder', 'payment', 'delivery', 'orderstatus', 'stocklevel']

OUTPUT_DIR="benchmarks/data"

DRY_RUN = False

BEST = False

def basic_test(testtype, impl, threads, rmw=False):
    cmd = CMD_RMW if rmw else CMD
    cmd += threads + impl + testtype
    
    pts = []
    neworders = []
    for i in xrange(ROUNDS):
        if DRY_RUN:
            out = "111 222"
            print cmd
        else:
            out = subprocess.check_output(cmd, shell=True)
        outl = out.split(' ')
        data = outl[0]
        pts.append(float(data))
        if len(outl) == 6:
            neworders.append(float(outl[5]))

    if len(neworders):
        return repr((pts, neworders))
    return repr(pts)
#sorted(pts)[len(pts)/2]
#    return max(pts)
#    return sum(pts) / len(pts)

def run_test(testname, testtype, impl, threads, fileobj):
    fileobj.write("%s\n%s\n" % (testname, basic_test(testtype, impl, threads, rmw=
                                                     testname=='readmywrites')))

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
    simple_run(MAKE_CMD_RMW)

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
#    patch_apply('rmw.patch')
    run_test('readmywrites', testtype, MBTA, threads, fileobj)
#    patch_revoke('rmw.patch')

def stdtest(f, testtype, threads, testname):
    singlethr = threads==SINGLE_THREADED
    f.write(testname + '\n')
    us_and_silo(testtype, threads, f)

    rmw(testtype, threads, f)

#    if singlethr:
        # for single threaded we also run a no sort test
#        nosort(testtype, threads, f)

    #remake()

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
    f = open(OUTPUT_DIR + '/' + FILENAME, 'w')

    stdtest(f, TPCC, SINGLE_THREADED, "1 thread")
    for i in [4,8,16,24]:
        stdtest(f, TPCC, MANY_THREADS(i), "%d threads" % i)

#    indiv_tpcc_tests(f, SINGLE_THREADED)

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
