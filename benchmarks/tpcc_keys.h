#pragma once

#include "../record/encoder.h"
#include "../record/inline_str.h"
#include "../macros.h"
#include "../third-party/lz4/xxhash.h"

struct __attribute__((packed)) customer_key {
    inline customer_key() {
    }
    inline customer_key(int32_t w_id, int32_t d_id, int32_t c_id)
        : c_w_id(w_id), c_d_id(d_id), c_id(c_id) {
    }
    inline bool operator==(const customer_key& other) const {
        return c_w_id == other.c_w_id && c_d_id == other.c_d_id && c_id == other.c_id;
    }
    inline bool operator!=(const customer_key& other) const {
        return !(*this == other);
    }
    int32_t c_w_id;
    int32_t c_d_id;
    int32_t c_id;
};

namespace std
{
  template<>
  struct hash<customer_key>
  {
     typedef customer_key argument_type;
     typedef std::size_t result_type;

     result_type operator()(argument_type const& s) const {
       //return XXH32(&s, sizeof(argument_type), 11);
       return ((size_t)s.c_id) | (((size_t)s.c_d_id & ((1 << 16) - 1)) << 32) | (((size_t)s.c_w_id & ((1 << 16) -1)) << 48);
     }
  };
}

struct __attribute__((packed)) history_key {
    inline history_key() {
    }
    inline history_key(int32_t h_c_id, int32_t h_c_d_id, int32_t h_c_w_id,
			int32_t h_d_id, int32_t h_w_id, uint32_t h_date)
        : h_c_id(h_c_id), h_c_d_id(h_c_d_id), h_c_w_id(h_c_w_id), h_d_id(h_d_id), h_w_id(h_w_id), h_date(h_date) {
    }
    inline bool operator==(const history_key& other) const {
        return h_c_id == other.h_c_id && h_c_d_id == other.h_c_d_id && h_c_w_id == other.h_c_w_id && h_d_id == other.h_d_id && h_w_id == other.h_w_id && h_date == other.h_date;
    }
    inline bool operator!=(const history_key& other) const {
        return !(*this == other);
    }
    int32_t h_c_id;
    int32_t h_c_d_id;
    int32_t h_c_w_id;
    int32_t h_d_id;
    int32_t h_w_id;
    uint32_t h_date;
};

namespace std
{
  template<>
  struct hash<history_key>
  {
     typedef history_key argument_type;
     typedef std::size_t result_type;

     result_type operator()(argument_type const& s) const {
       return XXH32(&s, sizeof(argument_type), 11);
     }
  };
}


struct __attribute__((packed)) district_key {
    inline district_key() {
    }
    inline district_key(int32_t w_id, int32_t d_id)
        : d_w_id(w_id), d_id(d_id) {
    }
    inline bool operator==(const district_key& other) const {
        return d_w_id == other.d_w_id && d_id == other.d_id;
    }
    inline bool operator!=(const district_key& other) const {
        return !(*this == other);
    }
    int32_t d_w_id;
    int32_t d_id;
};

namespace std
{
  template<>
  struct hash<district_key>
  {
     typedef district_key argument_type;
     typedef std::size_t result_type;

     result_type operator()(argument_type const& s) const {
       //return XXH32(&s, sizeof(argument_type), 11);
       return ((size_t)s.d_id) | ((size_t)s.d_w_id  << 32) ;
     }
  };
}


struct __attribute__((packed)) item_key {
    inline item_key() {
    }
    inline item_key(int32_t i_id)
        : i_id(i_id) {
    }
    inline bool operator==(const item_key& other) const {
        return i_id == other.i_id;
    }
    inline bool operator!=(const item_key& other) const {
        return !(*this == other);
    }
    int32_t i_id;
};

struct __attribute__((packed)) oorder_key {
    inline oorder_key() {
    }
    inline oorder_key(int32_t w_id, int32_t d_id, int32_t o_id)
        : o_w_id(w_id), o_d_id(d_id), o_id(o_id) {
    }
    inline bool operator==(const oorder_key& other) const {
        return o_w_id == other.o_w_id && o_d_id == other.o_d_id && o_id == other.o_id;
    }
    inline bool operator!=(const oorder_key& other) const {
        return !(*this == other);
    }
    int32_t o_w_id;
    int32_t o_d_id;
    int32_t o_id;
};

namespace std
{
  template<>
  struct hash<oorder_key>
  {
     typedef oorder_key argument_type;
     typedef std::size_t result_type;

     result_type operator()(argument_type const& s) const {
       //return XXH32(&s, sizeof(argument_type), 11);
       return ((size_t)s.o_id) | (((size_t)s.o_d_id & ((1 << 16) - 1)) << 32) | (((size_t)s.o_w_id & ((1 << 16) -1)) << 48);
     }
  };
}

struct __attribute__((packed)) stock_key {
    inline stock_key() {
    }
    inline stock_key(int32_t w_id, int32_t i_id)
        : s_w_id(w_id), s_i_id(i_id) {
    }
    inline bool operator==(const stock_key& other) const {
        return s_w_id == other.s_w_id && s_i_id == other.s_i_id;
    }
    inline bool operator!=(const stock_key& other) const {
        return !(*this == other);
    }
    int32_t s_w_id;
    int32_t s_i_id;
};

namespace std
{
  template<>
  struct hash<stock_key>
  {
     typedef stock_key argument_type;
     typedef std::size_t result_type;

     result_type operator()(argument_type const& s) const {
       //return XXH32(&s, sizeof(argument_type), 11);
       return ((size_t)s.s_i_id) | ((size_t)s.s_w_id  << 32) ;
     }
  };
}


struct __attribute__((packed)) warehouse_key {
    inline warehouse_key() {
    }
    inline warehouse_key(int32_t w_id)
        : w_id(w_id) {
    }
    inline bool operator==(const warehouse_key& other) const {
        return w_id == other.w_id;
    }
    inline bool operator!=(const warehouse_key& other) const {
        return !(*this == other);
    }
    int32_t w_id;
};

// More types added here to enable hash table for all indexes
struct customer_name_idx_key {
    inline customer_name_idx_key() {}
    inline customer_name_idx_key(int32_t w_id, int32_t d_id, const std::string& last, const std::string& first)
        : c_w_id(w_id), c_d_id(d_id), c_last(last), c_first(first) {}
    inline bool operator==(const customer_name_idx_key& other) const {
        return (c_w_id == other.c_w_id) && (c_d_id == other.c_d_id)
            && (c_last == other.c_last) && (c_first == other.c_first);
    }
    inline bool operator!=(const customer_name_idx_key& other) const {
        return !(*this == other);
    }

    int32_t c_w_id;
    int32_t c_d_id;
    std::string c_last;
    std::string c_first;
};

namespace std
{
    template<>
    struct hash<customer_name_idx_key>
    {
        typedef customer_name_idx_key argument_type;
        typedef std::size_t result_type;

        result_type operator()(argument_type const& s) const {
            result_type r1 = (result_type)s.c_w_id | ((result_type)s.c_d_id << 32);
            result_type r2 = XXH32(s.c_last.c_str(), s.c_last.length(), 11);
            r2 |= ((result_type)XXH32(s.c_first.c_str(), s.c_first.length(), 11) << 32);
            return r1 ^ r2;
        }
    };
}

struct __attribute__((packed)) new_order_key {
    inline new_order_key() {}
    inline new_order_key(int32_t w_id, int32_t d_id, int32_t o_id)
        : no_w_id(w_id), no_d_id(d_id), no_o_id(o_id) {}
    inline bool operator==(const new_order_key& other) const {
        return (no_w_id == other.no_w_id) && (no_d_id == other.no_d_id) && (no_o_id == other.no_o_id);
    }
    inline bool operator!=(const new_order_key& other) const {
        return !(*this == other);
    }
    int32_t no_w_id;
    int32_t no_d_id;
    int32_t no_o_id;
};

namespace std
{
    template<>
    struct hash<new_order_key>
    {
        typedef new_order_key argument_type;
        typedef std::size_t result_type;

        result_type operator()(argument_type const& s) const {
            return ((size_t)s.no_o_id) | (((size_t)s.no_d_id & ((1 << 16) - 1)) << 32) | (((size_t)s.no_w_id & ((1 << 16) -1)) << 48);
        }
    };
}

struct __attribute__((packed)) oorder_c_id_idx_key {
    inline oorder_c_id_idx_key() {}
    inline oorder_c_id_idx_key(int32_t w_id, int32_t d_id, int32_t c_id, int32_t o_id)
        : o_w_id(w_id), o_d_id(d_id), o_c_id(c_id), o_o_id(o_id) {}
    inline bool operator==(const oorder_c_id_idx_key& other) const {
        return (o_w_id == other.o_w_id) && (o_d_id == other.o_d_id)
            && (o_c_id == other.o_c_id) && (o_o_id == other.o_o_id);
    }
    inline bool operator!=(const oorder_c_id_idx_key& other) const {
        return !(*this == other);
    }
    int32_t o_w_id;
    int32_t o_d_id;
    int32_t o_c_id;
    int32_t o_o_id;
};

namespace std
{
    template<>
    struct hash<oorder_c_id_idx_key>
    {
        typedef oorder_c_id_idx_key argument_type;
        typedef std::size_t result_type;

        result_type operator()(argument_type const& s) const {
            return XXH32(&s, sizeof(argument_type), 11);
        }
    };
}

struct __attribute__((packed)) order_line_key {
    inline order_line_key() {}
    inline order_line_key(int32_t w_id, int32_t d_id, int32_t o_id, int32_t number)
        : ol_w_id(w_id), ol_d_id(d_id), ol_o_id(o_id), ol_number(number) {}
    inline bool operator==(const order_line_key& other) const {
        return (ol_w_id == other.ol_w_id) && (ol_d_id == other.ol_d_id)
            && (ol_o_id == other.ol_o_id) && (ol_number == other.ol_number);
    }
    inline bool operator!=(const order_line_key& other) const {
        return !(*this == other);
    }
    int32_t ol_w_id;
    int32_t ol_d_id;
    int32_t ol_o_id;
    int32_t ol_number;
};

namespace std
{
    template<>
    struct hash<order_line_key>
    {
        typedef order_line_key argument_type;
        typedef std::size_t result_type;

        result_type operator()(argument_type const& s) const {
            return XXH32(&s, sizeof(argument_type), 11);
        }
    };
}
