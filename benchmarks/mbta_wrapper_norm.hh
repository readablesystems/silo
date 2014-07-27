#pragma once

#include "abstract_db.h"
#include "abstract_ordered_index.h"
#include "Transaction.hh"
#include "MassTrans.hh"

#define STD_OP(f) auto& t = *unpack<Transaction*>(txn); \
  try { \
    f; \
  } catch (Transaction::Abort E) { \
    throw abstract_db::abstract_abort_exception(); \
  }

class mbta_wrapper;

class mbta_ordered_index : public abstract_ordered_index {
public:
  mbta_ordered_index(const std::string &name, mbta_wrapper *db) : mbta(), name(name), db(db) {}

  std::string *arena(void);

  bool get(void *txn, const std::string &key, std::string &value, size_t max_bytes_read) {
    STD_OP({
	// TODO: we'll still be faster if we just add support for max_bytes_read
	bool ret = mbta.transGet(t, key, value, max_bytes_read);
	// TODO: can we support this directly (max_bytes_read)? would avoid this wasted allocation
	return ret;
	  });
  }

  const char *put(
      void *txn,
      const std::string &key,
      const std::string &value)
  {
    // TODO: there's an overload of put that takes non-const std::string and silo seems to use move for those.
    // may be worth investigating if we can use that optimization to avoid copying keys
    STD_OP({
        mbta.transPut(t, key, value);
        return 0;
          });
  }
  
  const char *insert(
                                         void *txn,
                                         const std::string &key,
                                         const std::string &value)
  {
    STD_OP(mbta.transInsert(t, key, value); return 0;)
  }

  void remove(void *txn, const std::string &key) {
    STD_OP(mbta.transDelete(t, key));
  }

  void scan(
            void *txn,
            const std::string &start_key,
            const std::string *end_key,
            scan_callback &callback,
            str_arena *arena = nullptr) {
    mbta_type::Str end = end_key ? mbta_type::Str(*end_key) : mbta_type::Str();
    STD_OP(mbta.transQuery(t, start_key, end, [&] (mbta_type::Str key, versioned_str* value) {
	  auto *s = (*arena)();
	  s->assign(value->data(), value->length());
          return callback.invoke(key.data(), key.length(), *s);
        }));
  }

  void rscan(
             void *txn,
             const std::string &start_key,
             const std::string *end_key,
             scan_callback &callback,
             str_arena *arena = nullptr) {
#if 1
    mbta_type::Str end = end_key ? mbta_type::Str(*end_key) : mbta_type::Str();
    STD_OP(mbta.transRQuery(t, start_key, end, [&] (mbta_type::Str key, versioned_str* value) {
	  auto *s = (*arena)();
	  s->assign(value->data(), value->length());
          return callback.invoke(key.data(), key.length(), *s);
        }));
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

private:
  friend class mbta_wrapper;
  typedef MassTrans<versioned_str> mbta_type;
  mbta_type mbta;

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
    printf("v: %lu, k %lu, ref %lu, read %lu\n", version_mallocs, key_mallocs, ref_mallocs, read_mallocs);
#endif
    //txn_epoch_sync<Transaction>::finish();
  }

  void
  thread_init(bool loader)
  {
    static int tidcounter = 0;
    Transaction::threadid = tidcounter++;
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
    Transaction *txn = new (buf) Transaction;
    thr_arena = &arena;
    return txn;
  }

  bool commit_txn(void *txn) {
    STD_OP(return t.commit());
    return false;
  }

  void abort_txn(void *txn) {
    try {
      ((Transaction*)txn)->abort();
    } catch (Transaction::Abort E) {}
  }

  abstract_ordered_index *
  open_index(const std::string &name,
             size_t value_size_hint,
             bool mostly_append = false) {
    auto ret = new mbta_ordered_index(name, this);
    ret->mbta.thread_init();
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
