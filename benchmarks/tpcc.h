#pragma once 
#ifndef _NDB_BENCH_TPCC_H_
#define _NDB_BENCH_TPCC_H_

#include "../record/encoder.h"
#include "../record/inline_str.h"
#include "../macros.h"
#include "tpcc_keys.h"

#if HASHTABLE == 2
// dummy method to make hashtable code properly compile with
// range query interfaces (and of course it doesn't actually
// work) without breaking everything
template <typename T>
inline lcdf::Str&
EncodeK0(std::string& buf, const T& k) {
  return *reinterpret_cast<lcdf::Str *>(&buf);
}

template <typename T>
inline const T*
DecodeK(const std::string&, T&) {
  return nullptr;
}

template <typename T>
inline const T*
DecodeK(const char *, T&) {
  return nullptr;
}

#else

template <typename T>
inline std::string
EncodeK(const T &t)
{
  const encoder<T> enc;
  return enc.write(&t);
}

template <typename T>
inline const char *
EncodeK(uint8_t *buf, const T &t)
{
  const encoder<T> enc;
  return (const char *) enc.write(buf, &t);
}

template <typename T>
inline std::string &
EncodeK(std::string &buf, const T &t)
{
  const encoder<T> enc;
  return enc.write(buf, &t);
}

template <typename T>
inline std::string &
EncodeK0(std::string &buf, const T &t) {
  return EncodeK(buf, t);
}

template <typename T>
inline const T*
DecodeK(const std::string& buf, T& obj) {
  const encoder<T> enc;
  return enc.read(buf.data(), &obj);
}

template <typename T>
inline const T*
DecodeK(const char *buf, T& obj) {
  const encoder<T> enc;
  return enc.read(buf, &obj);
}

#endif

/* TABLE: customer */
#if HASHTABLE
inline customer_key EncodeK(const customer_key& k) {
    return k;	
}
inline customer_key EncodeK(std::string&, const customer_key& k) {
    return k;
}
#endif

#define CUSTOMER_KEY_FIELDS(x, y) \
  x(int32_t,c_w_id) \
  y(int32_t,c_d_id) \
  y(int32_t,c_id)
#define CUSTOMER_VALUE_FIELDS(x, y) \
  x(float,c_discount) \
  y(inline_str_fixed<2>,c_credit) \
  y(inline_str_8<16>,c_last) \
  y(inline_str_8<16>,c_first) \
  y(float,c_credit_lim) \
  y(float,c_balance) \
  y(float,c_ytd_payment) \
  y(int32_t,c_payment_cnt) \
  y(int32_t,c_delivery_cnt) \
  y(inline_str_8<20>,c_street_1) \
  y(inline_str_8<20>,c_street_2) \
  y(inline_str_8<20>,c_city) \
  y(inline_str_fixed<2>,c_state) \
  y(inline_str_fixed<9>,c_zip) \
  y(inline_str_fixed<16>,c_phone) \
  y(uint32_t,c_since) \
  y(inline_str_fixed<2>,c_middle) \
  y(inline_str_16<500>,c_data)
#if HASHTABLE
DO_STRUCT2(customer, customer_key, CUSTOMER_VALUE_FIELDS)
#else 
DO_STRUCT(customer, CUSTOMER_KEY_FIELDS, CUSTOMER_VALUE_FIELDS)
#endif

/* INDEX: customer_name_idx */
#if HASHTABLE == 2
inline lcdf::Str EncodeK(const customer_name_idx_key& k) {
    //static_assert(sizeof(k) == 40, "bad sizeof(customer_name_idx_key)");
    return lcdf::Str(reinterpret_cast<const char *>(&k), sizeof(k));
}
inline lcdf::Str EncodeK(std::string &, const customer_name_idx_key& k) {
    return lcdf::Str(reinterpret_cast<const char*>(&k), sizeof(k));
}
#endif

#define CUSTOMER_NAME_IDX_KEY_FIELDS(x, y) \
  x(int32_t,c_w_id) \
  y(int32_t,c_d_id) \
  y(inline_str_fixed<16>,c_last) \
  y(inline_str_fixed<16>,c_first)
#define CUSTOMER_NAME_IDX_VALUE_FIELDS(x, y) \
  x(int32_t,c_id)
#if HASHTABLE == 2
DO_STRUCT2(customer_name_idx, customer_name_idx_key, CUSTOMER_NAME_IDX_VALUE_FIELDS)
#else
DO_STRUCT(customer_name_idx, CUSTOMER_NAME_IDX_KEY_FIELDS, CUSTOMER_NAME_IDX_VALUE_FIELDS)
#endif

/* TABLE: district */
#if HASHTABLE == 2
inline lcdf::Str EncodeK(const district_key& k) {
    static_assert(sizeof(k) == 8, "bad sizeof(district_key)");
    return lcdf::Str(reinterpret_cast<const char*>(&k), sizeof(k));
}
inline lcdf::Str EncodeK(std::string&, const district_key& k) {
    return lcdf::Str(reinterpret_cast<const char*>(&k), sizeof(k));
}
#endif

#define DISTRICT_KEY_FIELDS(x, y) \
  x(int32_t,d_w_id) \
  y(int32_t,d_id)
#define DISTRICT_VALUE_FIELDS(x, y) \
  x(float,d_ytd) \
  y(float,d_tax) \
  y(int32_t,d_next_o_id) \
  y(inline_str_8<10>,d_name) \
  y(inline_str_8<20>,d_street_1) \
  y(inline_str_8<20>,d_street_2) \
  y(inline_str_8<20>,d_city) \
  y(inline_str_fixed<2>,d_state) \
  y(inline_str_fixed<9>,d_zip)
#if HASHTABLE == 2
DO_STRUCT2(district, district_key, DISTRICT_VALUE_FIELDS)
#else
DO_STRUCT(district, DISTRICT_KEY_FIELDS, DISTRICT_VALUE_FIELDS)
#endif

/* TABLE: history */
#if HASHTABLE
inline lcdf::Str EncodeK(const history_key& k) {
    static_assert(sizeof(k) == 24, "bad sizeof(history_key)");
    return lcdf::Str(reinterpret_cast<const char*>(&k), sizeof(k));
}
inline lcdf::Str EncodeK(std::string&, const history_key& k) {
    return lcdf::Str(reinterpret_cast<const char*>(&k), sizeof(k));
}
#endif

#define HISTORY_KEY_FIELDS(x, y) \
  x(int32_t,h_c_id) \
  y(int32_t,h_c_d_id) \
  y(int32_t,h_c_w_id) \
  y(int32_t,h_d_id) \
  y(int32_t,h_w_id) \
  y(uint32_t,h_date)
#define HISTORY_VALUE_FIELDS(x, y) \
  x(float,h_amount) \
  y(inline_str_8<24>,h_data)
#if HASHTABLE
DO_STRUCT2(history, history_key, HISTORY_VALUE_FIELDS)
#else
DO_STRUCT(history, HISTORY_KEY_FIELDS, HISTORY_VALUE_FIELDS)
#endif

/* TABLE: item */
#if HASHTABLE
inline int32_t EncodeK(const item_key& k) {
    return k.i_id;
}
inline int32_t EncodeK(std::string&, const item_key& k) {
    return k.i_id;
}
#endif

#define ITEM_KEY_FIELDS(x, y) \
  x(int32_t,i_id)
#define ITEM_VALUE_FIELDS(x, y) \
  x(inline_str_8<24>,i_name) \
  y(float,i_price) \
  y(inline_str_8<50>,i_data) \
  y(int32_t,i_im_id)
#if HASHTABLE
DO_STRUCT2(item, item_key, ITEM_VALUE_FIELDS)
#else
DO_STRUCT(item, ITEM_KEY_FIELDS, ITEM_VALUE_FIELDS)
#endif

/* TABLE: new_order */
#if HASHTABLE == 2
inline lcdf::Str EncodeK(const new_order_key& k) {
    static_assert(sizeof(k) == 12, "bad sizeof(new_order_key)");
    return lcdf::Str(reinterpret_cast<const char *>(&k), sizeof(k));
}
inline lcdf::Str EncodeK(std::string&, const new_order_key& k) {
    return lcdf::Str(reinterpret_cast<const char *>(&k), sizeof(k));
}
#endif

#define NEW_ORDER_KEY_FIELDS(x, y) \
  x(int32_t,no_w_id) \
  y(int32_t,no_d_id) \
  y(int32_t,no_o_id)
// need dummy b/c our btree cannot have empty values.
// we also size value so that it can fit a key
#define NEW_ORDER_VALUE_FIELDS(x, y) \
  x(inline_str_fixed<12>,no_dummy)
#if HASHTABLE == 2
DO_STRUCT2(new_order, new_order_key, NEW_ORDER_VALUE_FIELDS)
#else
DO_STRUCT(new_order, NEW_ORDER_KEY_FIELDS, NEW_ORDER_VALUE_FIELDS)
#endif

/* TABLE: oorder */
#if HASHTABLE
inline lcdf::Str EncodeK(const oorder_key& k) {
    static_assert(sizeof(k) == 12, "bad sizeof(oorder_key)");
    return lcdf::Str(reinterpret_cast<const char*>(&k), sizeof(k));
}
inline lcdf::Str EncodeK(std::string&, const oorder_key& k) {
    return lcdf::Str(reinterpret_cast<const char*>(&k), sizeof(k));
}
#endif

#define OORDER_KEY_FIELDS(x, y) \
  x(int32_t,o_w_id) \
  y(int32_t,o_d_id) \
  y(int32_t,o_id)
#define OORDER_VALUE_FIELDS(x, y) \
  x(int32_t,o_c_id) \
  y(int32_t,o_carrier_id) \
  y(int8_t,o_ol_cnt) \
  y(bool,o_all_local) \
  y(uint32_t,o_entry_d)
#if HASHTABLE
DO_STRUCT2(oorder, oorder_key, OORDER_VALUE_FIELDS)
#else 
DO_STRUCT(oorder, OORDER_KEY_FIELDS, OORDER_VALUE_FIELDS)
#endif

/* INDEX: oorder_c_id_idx */
#if HASHTABLE == 2
inline lcdf::Str EncodeK(const oorder_c_id_idx_key& k) {
    static_assert(sizeof(k) == 16, "bad sizeof(oorder_c_id_idx_key)");
    return lcdf::Str(reinterpret_cast<const char*>(&k), sizeof(k));
}
inline lcdf::Str EncodeK(std::string&, const oorder_c_id_idx_key& k) {
    return lcdf::Str(reinterpret_cast<const char*>(&k), sizeof(k));
}
#endif

#define OORDER_C_ID_IDX_KEY_FIELDS(x, y) \
  x(int32_t,o_w_id) \
  y(int32_t,o_d_id) \
  y(int32_t,o_c_id) \
  y(int32_t,o_o_id)
#define OORDER_C_ID_IDX_VALUE_FIELDS(x, y) \
  x(uint8_t,o_dummy)
#if HASHTABLE == 2
DO_STRUCT2(oorder_c_id_idx, oorder_c_id_idx_key, OORDER_C_ID_IDX_VALUE_FIELDS)
#else
DO_STRUCT(oorder_c_id_idx, OORDER_C_ID_IDX_KEY_FIELDS, OORDER_C_ID_IDX_VALUE_FIELDS)
#endif

/* TABLE: order_line */
#if HASHTABLE == 2
inline lcdf::Str EncodeK(const order_line_key& k) {
    static_assert(sizeof(k) == 16, "bad sizeof(order_line_key)");
    return lcdf::Str(reinterpret_cast<const char*>(&k), sizeof(k));
}
inline lcdf::Str EncodeK(std::string&, const order_line_key& k) {
    return lcdf::Str(reinterpret_cast<const char*>(&k), sizeof(k));
}
#endif

#define ORDER_LINE_KEY_FIELDS(x, y) \
  x(int32_t,ol_w_id) \
  y(int32_t,ol_d_id) \
  y(int32_t,ol_o_id) \
  y(int32_t,ol_number)
#define ORDER_LINE_VALUE_FIELDS(x, y) \
  x(int32_t,ol_i_id) \
  y(uint32_t,ol_delivery_d) \
  y(float,ol_amount) \
  y(int32_t,ol_supply_w_id) \
  y(int8_t,ol_quantity)
#if HASHTABLE == 2
DO_STRUCT2(order_line, order_line_key, ORDER_LINE_VALUE_FIELDS)
#else
DO_STRUCT(order_line, ORDER_LINE_KEY_FIELDS, ORDER_LINE_VALUE_FIELDS)
#endif

/* TABLE: stock */
#if HASHTABLE
inline lcdf::Str EncodeK(const stock_key& k) {
    static_assert(sizeof(k) == 8, "bad sizeof(stock_key)");
    return lcdf::Str(reinterpret_cast<const char*>(&k), sizeof(k));
}
inline lcdf::Str EncodeK(std::string&, const stock_key& k) {
    return lcdf::Str(reinterpret_cast<const char*>(&k), sizeof(k));
}
#endif

#define STOCK_KEY_FIELDS(x, y) \
  x(int32_t,s_w_id) \
  y(int32_t,s_i_id)
#define STOCK_VALUE_FIELDS(x, y) \
  x(int16_t,s_quantity) \
  y(float,s_ytd) \
  y(int32_t,s_order_cnt) \
  y(int32_t,s_remote_cnt)
#if HASHTABLE
DO_STRUCT2(stock, stock_key, STOCK_VALUE_FIELDS)
#else
DO_STRUCT(stock, STOCK_KEY_FIELDS, STOCK_VALUE_FIELDS)
#endif

/* TABLE: stock_data */
#define STOCK_DATA_KEY_FIELDS(x, y) \
  x(int32_t,s_w_id) \
  y(int32_t,s_i_id)
#define STOCK_DATA_VALUE_FIELDS(x, y)           \
  x(inline_str_8<50>,s_data) \
  y(inline_str_fixed<24>,s_dist_01) \
  y(inline_str_fixed<24>,s_dist_02) \
  y(inline_str_fixed<24>,s_dist_03) \
  y(inline_str_fixed<24>,s_dist_04) \
  y(inline_str_fixed<24>,s_dist_05) \
  y(inline_str_fixed<24>,s_dist_06) \
  y(inline_str_fixed<24>,s_dist_07) \
  y(inline_str_fixed<24>,s_dist_08) \
  y(inline_str_fixed<24>,s_dist_09) \
  y(inline_str_fixed<24>,s_dist_10)
#if HASHTABLE
DO_STRUCT2(stock_data, stock_key, STOCK_DATA_VALUE_FIELDS)
#else 
DO_STRUCT(stock_data, STOCK_DATA_KEY_FIELDS, STOCK_DATA_VALUE_FIELDS)
#endif

#if HASHTABLE
inline int32_t EncodeK(const warehouse_key& k) {
    return k.w_id;
}
inline int32_t EncodeK(std::string&, const warehouse_key& k) {
    return k.w_id;
}
#endif

#define WAREHOUSE_KEY_FIELDS(x, y) \
  x(int32_t,w_id)
#define WAREHOUSE_VALUE_FIELDS(x, y) \
  x(float,w_ytd) \
  y(float,w_tax) \
  y(inline_str_8<10>,w_name) \
  y(inline_str_8<20>,w_street_1) \
  y(inline_str_8<20>,w_street_2) \
  y(inline_str_8<20>,w_city) \
  y(inline_str_fixed<2>,w_state) \
  y(inline_str_fixed<9>,w_zip)
#if HASHTABLE == 2
DO_STRUCT2(warehouse, warehouse_key, WAREHOUSE_VALUE_FIELDS)
#else
DO_STRUCT(warehouse, WAREHOUSE_KEY_FIELDS, WAREHOUSE_VALUE_FIELDS)
#endif

#endif //#ifndef _NDB_BENCH_TPCC_H_
