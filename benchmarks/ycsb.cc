#include <iostream>
#include <sstream>
#include <vector>
#include <utility>
#include <string>

#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

#include "../macros.h"
#include "../varkey.h"
#include "../thread.h"
#include "../util.h"
#include "../spinbarrier.h"

#include "bdb_wrapper.h"
#include "ndb_wrapper.h"
#include "mysql_wrapper.h"

using namespace std;
using namespace util;

static size_t nkeys = 100000;
static size_t nthreads = 1;
static volatile bool running = true;
static int verbose = 0;

class worker;
typedef void (*txn_fn_t)(worker *);
typedef vector<pair<double, txn_fn_t> > workload_desc;
static workload_desc *MakeWorkloadDesc();
static workload_desc *ycsb_workload = MakeWorkloadDesc();

class worker : public ndb_thread {
public:
  worker(unsigned long seed, abstract_db *db,
         spin_barrier *barrier_a, spin_barrier *barrier_b)
    : r(seed), db(db), barrier_a(barrier_a), barrier_b(barrier_b),
      // the ntxn_* numbers are per worker
      ntxn_commits(0), ntxn_aborts(0)
  {
  }

  ~worker()
  {
  }

  void
  txn_read()
  {
    void *txn = db->new_txn();
    string k = u64_varkey(r.next() % nkeys).str();
    try {
      char *v = 0;
      size_t vlen = 0;
      ALWAYS_ASSERT(db->get(txn, k.data(), k.size(), v, vlen));
      free(v);
      if (db->commit_txn(txn))
        ntxn_commits++;
    } catch (abstract_db::abstract_abort_exception &ex) {
      db->abort_txn(txn);
      ntxn_aborts++;
    }
  }
  static void TxnRead(worker *w) { w->txn_read(); }

  void
  txn_write()
  {
    void *txn = db->new_txn();
    string k = u64_varkey(r.next() % nkeys).str();
    try {
      string v(128, 'b');
      db->put(txn, k.data(), k.size(), v.data(), v.size());
      if (db->commit_txn(txn))
        ntxn_commits++;
    } catch (abstract_db::abstract_abort_exception &ex) {
      db->abort_txn(txn);
      ntxn_aborts++;
    }
  }
  static void TxnWrite(worker *w) { w->txn_write(); }

  void
  txn_rmw()
  {
    void *txn = db->new_txn();
    string k = u64_varkey(r.next() % nkeys).str();
    try {
      char *v = 0;
      size_t vlen = 0;
      ALWAYS_ASSERT(db->get(txn, k.data(), k.size(), v, vlen));
      free(v);
      string vnew(128, 'c');
      db->put(txn, k.data(), k.size(), vnew.data(), vnew.size());
      if (db->commit_txn(txn))
        ntxn_commits++;
    } catch (abstract_db::abstract_abort_exception &ex) {
      db->abort_txn(txn);
      ntxn_aborts++;
    }
  }
  static void TxnRmw(worker *w) { w->txn_rmw(); }

  class worker_scan_callback : public abstract_db::scan_callback {
  public:
    virtual bool
    invoke(const char *key, size_t key_len,
           const char *value, size_t value_len)
    {
      return true;
    }
  };

  void
  txn_scan()
  {
    void *txn = db->new_txn();
    size_t kstart = r.next() % nkeys;
    string kbegin = u64_varkey(kstart).str();
    string kend = u64_varkey(kstart + 100).str();
    worker_scan_callback c;
    try {
      db->scan(txn, kbegin.data(), kbegin.size(),
               kend.data(), kend.size(), true, c);
      if (db->commit_txn(txn))
        ntxn_commits++;
    } catch (abstract_db::abstract_abort_exception &ex) {
      db->abort_txn(txn);
      ntxn_aborts++;
    }
  }
  static void TxnScan(worker *w) { w->txn_scan(); }

  virtual void run()
  {
    db->thread_init();
    barrier_a->count_down();
    barrier_b->wait_for();
    while (running) {
      double d = r.next_uniform();
      for (size_t i = 0; i < ycsb_workload->size(); i++) {
        if ((i + 1) == ycsb_workload->size() || d < ycsb_workload->operator[](i).first) {
          ycsb_workload->operator[](i).second(this);
          break;
        }
      }
    }
    db->thread_end();
  }

  fast_random r;
  abstract_db *db;
  spin_barrier *barrier_a;
  spin_barrier *barrier_b;
  size_t ntxn_commits;
  size_t ntxn_aborts;
};

static workload_desc *
MakeWorkloadDesc()
{
  workload_desc *w = new workload_desc;
  w->push_back(make_pair(0.85, worker::TxnRead));
  w->push_back(make_pair(0.10, worker::TxnScan));
  w->push_back(make_pair(0.04, worker::TxnRmw));
  w->push_back(make_pair(0.01, worker::TxnWrite));
  return w;
}

static void
do_test(abstract_db *db)
{
  spin_barrier barrier_a(nthreads);
  spin_barrier barrier_b(1);

  db->thread_init();

  // load
  const size_t batchsize = (db->txn_max_batch_size() == -1) ?
    10000 : db->txn_max_batch_size();
  ALWAYS_ASSERT(batchsize > 0);
  size_t nbatches = nkeys / batchsize;
  for (size_t i = 0; i < nbatches; i++) {
    size_t keyend = (i == nbatches - 1) ? nkeys : (i + 1) * batchsize;
    void *txn = db->new_txn();
    for (size_t j = i * batchsize; j < keyend; j++) {
      string k = u64_varkey(j).str();
      string v(128, 'a');
      db->insert(txn, k.data(), k.size(), v.data(), v.size());
    }
    cerr << "batch " << (i + 1) << "/" << nbatches << " done" << endl;
    ALWAYS_ASSERT(db->commit_txn(txn));
  }

  fast_random r(8544290);
  vector<worker *> workers;
  for (size_t i = 0; i < nthreads; i++)
    workers.push_back(new worker(r.next(), db, &barrier_a, &barrier_b));
  for (size_t i = 0; i < nthreads; i++)
    workers[i]->start();
  barrier_a.wait_for();
  barrier_b.count_down();
  timer t;
  sleep(30);
  running = false;
  __sync_synchronize();
  unsigned long elapsed = t.lap();
  size_t n_commits = 0;
  size_t n_aborts = 0;
  for (size_t i = 0; i < nthreads; i++) {
    workers[i]->join();
    n_commits += workers[i]->ntxn_commits;
    n_aborts += workers[i]->ntxn_aborts;
  }

  double agg_throughput = double(n_commits) / (double(elapsed) / 1000000.0);
  double avg_per_core_throughput = agg_throughput / double(nthreads);

  double agg_abort_rate = double(n_aborts) / (double(elapsed) / 1000000.0);
  double avg_per_core_abort_rate = agg_abort_rate / double(nthreads);

  if (verbose) {
    cerr << "agg_throughput: " << agg_throughput << " ops/sec" << endl;
    cerr << "avg_per_core_throughput: " << avg_per_core_throughput << " ops/sec/core" << endl;
    cerr << "agg_abort_rate: " << agg_abort_rate << " aborts/sec" << endl;
    cerr << "avg_per_core_abort_rate: " << avg_per_core_abort_rate << " aborts/sec/core" << endl;
  }
  cout << agg_throughput << endl;

  db->thread_end();
}

int
main(int argc, char **argv)
{
  abstract_db *db = NULL;
  string db_type;
  char *curdir = get_current_dir_name();
  string basedir = curdir;
  free(curdir);
  while (1) {
    static struct option long_options[] =
    {
      {"verbose",     no_argument,       &verbose, 1},
      {"num-keys",    required_argument, 0,       'k'},
      {"num-threads", required_argument, 0,       't'},
      {"db-type",     required_argument, 0,       'd'},
      {"basedir",     required_argument, 0,       'b'},
      {0, 0, 0, 0}
    };
    int option_index = 0;
    int c = getopt_long(argc, argv, "vk:t:d:b:", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 0:
      if (long_options[option_index].flag != 0)
        break;
      abort();
      break;

    case 'k':
      nkeys = strtoul(optarg, NULL, 10);
      ALWAYS_ASSERT(nkeys > 0);
      break;

    case 't':
      nthreads = strtoul(optarg, NULL, 10);
      ALWAYS_ASSERT(nthreads > 0);
      break;

    case 'd':
      db_type = optarg;
      break;

    case 'b':
      basedir = optarg;
      break;

    case '?':
      /* getopt_long already printed an error message. */
      break;

    default:
      abort();
    }
  }

  if (db_type == "bdb") {
    string cmd = "rm -rf " + basedir + "/db/*";
    int ret UNUSED = system(cmd.c_str());
    db = new bdb_wrapper("db", "ycsb.db");
  } else if (db_type == "ndb-proto1") {
    db = new ndb_wrapper(ndb_wrapper::PROTO_1);
  } else if (db_type == "ndb-proto2") {
    db = new ndb_wrapper(ndb_wrapper::PROTO_2);
  } else if (db_type == "mysql") {
    string dbdir = basedir + "/mysql-db";
    db = new mysql_wrapper(dbdir.c_str(), "ycsb");
  } else
    ALWAYS_ASSERT(false);

  if (verbose) {
    cerr << "settings:" << endl;
    cerr << "  num-keys    : " << nkeys << endl;
    cerr << "  num-threads : " << nthreads << endl;
    cerr << "  db-type     : " << db_type << endl;
    cerr << "  basedir     : " << basedir << endl;
  }

  do_test(db);

  delete db;
  return 0;
}
