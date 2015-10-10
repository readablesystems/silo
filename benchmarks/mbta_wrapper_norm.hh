#pragma once
#include <atomic>
#include "abstract_db.h"
#include "abstract_ordered_index.h"
#include "sto/Transaction.hh"
#include "sto/MassTrans.hh"
#include "sto/Hashtable.hh"
#include <unordered_map> 
//#include "tpcc.h"

#define STD_OP(f) \
  try { \
    f; \
  } catch (Transaction::Abort E) { \
    throw abstract_db::abstract_abort_exception(); \
  }

#define OP_LOGGING 0

#if OP_LOGGING
std::atomic<long> mt_get(0);
std::atomic<long> mt_put(0);
std::atomic<long> mt_del(0);
std::atomic<long> mt_scan(0);
std::atomic<long> mt_rscan(0);
std::atomic<long> ht_get(0);
std::atomic<long> ht_put(0);
std::atomic<long> ht_insert(0);
std::atomic<long> ht_del(0);
#endif

/*namespace std
{
  template<>
  struct hash<item::key>
  {
     typedef item::key argument_type;
     typedef std::size_t result_type;

     result_type operator()(argument_type const& s) const {
       return (std::hash<int32_t>()(s.i_id));
     }
  };
}*/

class mbta_wrapper;

class mbta_ordered_index : public abstract_ordered_index {
public:
  mbta_ordered_index(const std::string &name, mbta_wrapper *db) : mbta(), name(name), db(db) {}

  std::string *arena(void);

    bool get(void *txn, lcdf::Str key, std::string &value, size_t max_bytes_read) {
#if OP_LOGGING
    mt_get++;
#endif
    STD_OP({
	// TODO: we'll still be faster if we just add support for max_bytes_read
        bool ret = mbta.transGet(key, value);
	// TODO: can we support this directly (max_bytes_read)? would avoid this wasted allocation
	return ret;
	  });
  }

  const char *put(void* txn,
                  lcdf::Str key,
                  const std::string &value)
  {
#if OP_LOGGING
    mt_put++;
#endif
    // TODO: there's an overload of put that takes non-const std::string and silo seems to use move for those.
    // may be worth investigating if we can use that optimization to avoid copying keys
    STD_OP({
        mbta.transPut<false>(key, value);
        return 0;
          });
  }
  
  const char *insert(void *txn,
                     lcdf::Str key,
                     const std::string &value)
  {
    STD_OP(mbta.transInsert<false>(key, value); return 0;)
  }

  void remove(void *txn, const std::string &key) {
#if OP_LOGGING
    mt_del++;
#endif
     STD_OP(mbta.transDelete<false>(key));
  }

  void scan(void *txn,
            lcdf::Str start_key,
            const std::string *end_key,
            scan_callback &callback,
            str_arena *arena = nullptr) {
#if OP_LOGGING
    mt_scan++;
#endif    
    mbta_type::Str end = end_key ? mbta_type::Str(*end_key) : mbta_type::Str();
    STD_OP(mbta.transQuery(start_key, end, [&] (mbta_type::Str key, std::string& value) {
          return callback.invoke(key.data(), key.length(), value);
        }, arena));
  }

  void rscan(void *txn,
             lcdf::Str start_key,
             const std::string *end_key,
             scan_callback &callback,
             str_arena *arena = nullptr) {
#if 1
#if OP_LOGGING
    mt_rscan++;
#endif
    mbta_type::Str end = end_key ? mbta_type::Str(*end_key) : mbta_type::Str();
    STD_OP(mbta.transRQuery(start_key, end, [&] (mbta_type::Str key, std::string& value) {
          return callback.invoke(key.data(), key.length(), value);
        }, arena));
#endif
  }

  size_t size() const
  {
    return mbta.approx_size();
  }

  // TODO: unclear if we need to implement, apparently this should clear the tree and possibly return some stats
  std::map<std::string, uint64_t>
  clear() {
    throw 2;
  }

  typedef MassTrans<std::string, versioned_str_struct, READ_MY_WRITES/*opacity*/> mbta_type;
private:
  friend class mbta_wrapper;
  mbta_type mbta;

  const std::string name;

  mbta_wrapper *db;

};

class ht_ordered_index_string : public abstract_ordered_index {
public:
  ht_ordered_index_string(const std::string &name, mbta_wrapper *db) : ht(), name(name), db(db) {}

  std::string *arena(void);

  bool get(void *txn, lcdf::Str key, std::string &value, size_t max_bytes_read) {
#if OP_LOGGING
    ht_get++;
#endif
    STD_OP({
	// TODO: we'll still be faster if we just add support for max_bytes_read
        bool ret = ht.read(key, value);
	//auto it = ht.find(key);
 	//bool ret = it != ht.end();
	//value = it->second;
        // TODO: can we support this directly (max_bytes_read)? would avoid this wasted allocation
	return ret;
	  });
  }

  const char *put(
      void* txn,
      lcdf::Str key,
      const std::string &value)
  {
#if OP_LOGGING
    ht_put++;
#endif
    // TODO: there's an overload of put that takes non-const std::string and silo seems to use move for those.
    // may be worth investigating if we can use that optimization to avoid copying keys
    STD_OP({
	ht.put(key, value);
        //ht[key] = value;
        return 0;
          });
  }

  const char *insert(void *txn,
                     lcdf::Str key,
                     const std::string &value)
  {
#if OP_LOGGING
    ht_insert++;
#endif
    STD_OP({
	ht.transInsert(key, value); return 0;
	});
  }

  void remove(void *txn, lcdf::Str key) {
#if OP_LOGGING
    ht_del++;
#endif    
    STD_OP({
	ht.transDelete(key);
    });
  }

  void scan(void *txn,
            lcdf::Str start_key,
            const std::string *end_key,
            scan_callback &callback,
            str_arena *arena = nullptr) {
    NDB_UNIMPLEMENTED("scan");
  }

  void rscan(void *txn,
             lcdf::Str start_key,
             const std::string *end_key,
             scan_callback &callback,
             str_arena *arena = nullptr) {
    NDB_UNIMPLEMENTED("rscan");
  }

  size_t size() const
  {
    return 0;
  }

  // TODO: unclear if we need to implement, apparently this should clear the tree and possibly return some stats
  std::map<std::string, uint64_t>
  clear() {
    throw 2;
  }

  typedef Hashtable<std::string, std::string, READ_MY_WRITES/*opacity*/, 1000000> ht_type;
  //typedef std::unordered_map<K, std::string> ht_type;
private:
  friend class mbta_wrapper;
  ht_type ht;

  const std::string name;

  mbta_wrapper *db;

};


class ht_ordered_index_int : public abstract_ordered_index {
public:
  ht_ordered_index_int(const std::string &name, mbta_wrapper *db) : ht(), name(name), db(db) {}

  std::string *arena(void);

  bool get(void *txn, lcdf::Str key, std::string &value, size_t max_bytes_read) {
    return false;
  }

  bool get(
      void *txn,
      int32_t key,
      std::string &value,
      size_t max_bytes_read = std::string::npos) {
#if OP_LOGGING
    ht_get++;
#endif
    STD_OP({
        bool ret = ht.read(key, value);
        return ret;
          });

  }


  const char *put(
      void* txn,
      lcdf::Str key,
      const std::string &value)
  {
    return 0;
  }

  const char *put(
      void* txn,
      int32_t key,
      const std::string &value)
  {
#if OP_LOGGING
    ht_put++;
#endif
    STD_OP({
        ht.put(key, value);
        return 0;
          });
  }

  
  const char *insert(void *txn,
                     lcdf::Str key,
                     const std::string &value)
  {
    return 0;
  }

  const char *insert(void *txn,
                     int32_t key,
                     const std::string &value)
  {
#if OP_LOGGING
    ht_insert++;
#endif
    STD_OP({
        ht.transInsert(key, value); return 0;});
  }


  void remove(void *txn, lcdf::Str key) {
      return;
  }

  void remove(void *txn, int32_t key) {
#if OP_LOGGING
    ht_del++;
#endif    
    STD_OP({
        ht.transDelete(key);});
  }     

  void scan(void *txn,
            lcdf::Str start_key,
            const std::string *end_key,
            scan_callback &callback,
            str_arena *arena = nullptr) {
    NDB_UNIMPLEMENTED("scan");
  }

  void rscan(void *txn,
             lcdf::Str start_key,
             const std::string *end_key,
             scan_callback &callback,
             str_arena *arena = nullptr) {
    NDB_UNIMPLEMENTED("rscan");
  }

  size_t size() const
  {
    return 0;
  }

  // TODO: unclear if we need to implement, apparently this should clear the tree and possibly return some stats
  std::map<std::string, uint64_t>
  clear() {
    throw 2;
  }

  typedef Hashtable<int32_t, std::string, READ_MY_WRITES/*opacity*/, 1000000> ht_type;
  //typedef std::unordered_map<K, std::string> ht_type;
private:
  friend class mbta_wrapper;
  ht_type ht;

  const std::string name;

  mbta_wrapper *db;

};



class mbta_wrapper : public abstract_db {
public:
  ssize_t txn_max_batch_size() const OVERRIDE { return 100; }
  
  void
  do_txn_epoch_sync() const
  {
    //txn_epoch_sync<Transaction>::sync();
  }

  void
  do_txn_finish() const
  {
#if PERF_LOGGING
    Transaction::print_stats();
    //    printf("v: %lu, k %lu, ref %lu, read %lu\n", version_mallocs, key_mallocs, ref_mallocs, read_mallocs);
   {
        using thd = threadinfo_t;
        thd tc = Transaction::tinfo_combined();
        printf("total_n: %llu, total_r: %llu, total_w: %llu, total_searched: %llu, total_aborts: %llu (%llu aborts at commit time)\n", tc.p(txp_total_n), tc.p(txp_total_r), tc.p(txp_total_w), tc.p(txp_total_searched), tc.p(txp_total_aborts), tc.p(txp_commit_time_aborts));
    }

#endif
    std::cout << "Find traversal: " << ct << std::endl;
    std::cout << "Max traversal: " << max_ct << std::endl;
#if OP_LOGGING
    printf("mt_get: %ld, mt_put: %ld, mt_del: %ld, mt_scan: %ld, mt_rscan: %ld, ht_get: %ld, ht_put: %ld, ht_insert: %ld, ht_del: %ld\n", mt_get.load(), mt_put.load(), mt_del.load(), mt_scan.load(), mt_rscan.load(), ht_get.load(), ht_put.load(), ht_insert.load(), ht_del.load());
#endif 
    //txn_epoch_sync<Transaction>::finish();
  }

  void
  thread_init(bool loader)
  {
    static int tidcounter = 0;
    Transaction::threadid = __sync_fetch_and_add(&tidcounter, 1);
    if (Transaction::threadid == 0) {
      // someone has to do this (they don't provide us with a general init callback)
      mbta_ordered_index::mbta_type::static_init();
      // need this too
      pthread_t advancer;
      pthread_create(&advancer, NULL, Transaction::epoch_advancer, NULL);
      pthread_detach(advancer);
    }
    mbta_ordered_index::mbta_type::thread_init();
  }

  void
  thread_end()
  {

  }

  size_t
  sizeof_txn_object(uint64_t txn_flags) const
  {
    return sizeof(Transaction);
  }

  static __thread str_arena *thr_arena;
  void *new_txn(
                uint64_t txn_flags,
                str_arena &arena,
                void *buf,
                TxnProfileHint hint = HINT_DEFAULT) {
    Sto::start_transaction();
    thr_arena = &arena;
    return NULL;
  }

  bool commit_txn(void *txn) {
    return Sto::try_commit();
  }

  void abort_txn(void *txn) {
    Sto::silent_abort();
  }

  abstract_ordered_index *
  open_index(const std::string &name,
             size_t value_size_hint,
	     bool mostly_append = false,
             bool use_hashtable = false) {
    if (use_hashtable) {
      return new ht_ordered_index_int(name, this);
    }
    auto ret = new mbta_ordered_index(name, this);
    return ret;
  }

 void
 close_index(abstract_ordered_index *idx) {
   delete idx;
 }

};

__thread str_arena* mbta_wrapper::thr_arena;

std::string *mbta_ordered_index::arena() {
  return (*db->thr_arena)();
}
