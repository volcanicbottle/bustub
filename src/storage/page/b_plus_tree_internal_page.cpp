//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// b_plus_tree_internal_page.cpp
//
// Identification: src/storage/page/b_plus_tree_internal_page.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <sstream>

#include "common/exception.h"
#include "storage/page/b_plus_tree_internal_page.h"

namespace bustub {
/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/

/**
 * @brief Init method after creating a new internal page.
 *
 * Writes the necessary header information to a newly created page,
 * including set page type, set current size, set page id, set parent id and set max page size,
 * must be called after the creation of a new page to make a valid BPlusTreeInternalPage.
 *
 * @param max_size Maximal size of the page
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Init(int max_size) {
    SetPageType(IndexPageType::INTERNAL_PAGE);
    SetSize(0);
    SetMaxSize(max_size);
}

/**
 * @brief Helper method to get/set the key associated with input "index"(a.k.a
 * array offset).
 *
 * @param index The index of the key to get. Index must be non-zero.
 * @return Key at index
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::KeyAt(int index) const -> KeyType {
  return key_array_[index];
}

/**
 * @brief Set key at the specified index.
 *
 * @param index The index of the key to set. Index must be non-zero.
 * @param key The new value for key
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetKeyAt(int index, const KeyType &key) {
  key_array_[index]=key;
}

/**
 * @brief Helper method to get the value associated with input "index"(a.k.a array
 * offset)
 *
 * @param index The index of the value to get.
 * @return Value at index
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueAt(int index) const -> ValueType {
  return page_id_array_[index];
}

/**
 * @brief Set value at the specified index.
 *
 * @param index The index of the value to set.
 * @param value The new value
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetValueAt(int index, const ValueType &value) {
  page_id_array_[index] = value;
}

/**
 * @brief Helper method to find the index of the value.
 *
 * @param value The value to search for
 * @return The index that corresponds to the specified value
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueIndex(const ValueType &value) const -> int {
  for (int i = 0; i < GetSize(); i++) {
    if (page_id_array_[i] == value) {
      return i;
    }
  }
  return -1;  // Not found
}

/**
 * @brief Find the index of the key using binary search
 * @param key The key to search for
 * @param comparator The key comparator
 * @return The index where the key should be inserted
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::KeyIndex(const KeyType &key, const KeyComparator &comparator) const -> int {
  // 在 keys 的有效区间 [1, GetSize()-1] 上 lower_bound
  int left = 1, right = GetSize() - 1, ans = GetSize();  // 若全部小于，则返回 size（方便后续用 i-1 做 value 下标）
  while (left <= right) {
    int mid = (left + right) / 2;
    if (comparator(key, key_array_[mid]) < 0) {
      ans = mid;
      right = mid - 1;
    } else {
      left = mid + 1;
    }
  }
  return ans;
}

/**
 * @brief Lookup the value for a given key
 * @param key The key to lookup
 * @param comparator The key comparator
 * @return The value (page_id) for the key
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::Lookup(const KeyType &key, const KeyComparator &comparator) const -> ValueType {
  // 找到第一个 key_array_[i] > key 的 i，然后走 value[i-1]
  if (GetSize() == 0) {
    throw Exception("InternalPage Lookup on empty node");
  }
  int i = KeyIndex(key, comparator);
  return page_id_array_[i - 1];
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::InsertNodeAfter(const ValueType &old_value,
                                                     const KeyType &new_key,
                                                     const ValueType &new_value) -> int {
  // 先找到 old_value 的位置
  int idx = ValueIndex(old_value);
  if (idx == -1) {
    throw Exception("InsertNodeAfter: old_value not found");
  }
  // 在 idx 之后插入一个 value 位，并在 idx+1 位置写入 new_key
  int size = GetSize();
  // values 向右挪一格：从末尾到 idx+1
  for (int i = size; i > idx + 1; --i) {
    page_id_array_[i] = page_id_array_[i - 1];
  }
  page_id_array_[idx + 1] = new_value;

  // keys 向右挪一格：有效范围 [1..size-1]，我们要在 idx+1 处放 key
  for (int i = size; i > idx + 1; --i) {
    key_array_[i] = key_array_[i - 1];
  }
  key_array_[idx + 1] = new_key;

  SetSize(size + 1);
  return GetSize();
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Remove(int index) {
  // 删除第 index 个 value；并且让右侧的 key 左移覆盖（相当于删除了与该 value 右相邻的 key）
  // 合法 index: 0..size-1
  int size = GetSize();
  BUSTUB_ASSERT(index >= 0 && index < size, "Remove index out of range");

  // values 左移
  for (int i = index; i < size - 1; ++i) {
    page_id_array_[i] = page_id_array_[i + 1];
  }

  // keys 左移（有效区间是 1..size-1），被删除的"配对 key"在 index+1 位置
  for (int i = index + 1; i < size; ++i) {
    key_array_[i] = key_array_[i + 1];
  }

  SetSize(size - 1);
}


INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::RemoveAndReturnOnlyChild() -> ValueType {
  // 用于根收缩：根内部页只有 1 个 child 时，返回它
  BUSTUB_ASSERT(GetSize() == 1, "Root should have exactly one child when shrinking");
  return page_id_array_[0];

}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveHalfTo(BPlusTreeInternalPage *recipient) {
  // 把后半段移动到 recipient（典型分裂用）
  int size = GetSize();
  int start = size / 2;          // 从第 start 个 value 开始搬（保留前半）
  int n = size - start;          // 要搬多少个 values
  recipient->CopyNFrom(this, start, n);
  SetSize(start);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyNFrom(const BPlusTreeInternalPage *donor, int start_value_index, int n) {
  // 把 donor 的 [start_value_index, start_value_index + n) 这段 value 以及其相邻 keys 复制到当前页尾部
  // 当前页尾部插入位置
  int cur_size = GetSize();
  // 先复制 values
  for (int i = 0; i < n; ++i) {
    page_id_array_[cur_size + i] = donor->page_id_array_[start_value_index + i];
  }
  // 再复制 keys：keys 与右侧 value 对齐，所以 keys 段是 [start_value_index+1 .. start_value_index + n]
  for (int i = 0; i < n; ++i) {
    key_array_[cur_size + i] = donor->key_array_[start_value_index + i + 1];
  }
  SetSize(cur_size + n);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveAllTo(BPlusTreeInternalPage *recipient) {
  // 合并：把自己所有 value/key 追加到 recipient 尾部
  int n = GetSize();
  recipient->CopyNFrom(this, 0, n);
  SetSize(0);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::PopulateNewRoot(const ValueType &old_value,
                                                     const KeyType &new_key,
                                                     const ValueType &new_value) {
  // 新根： [ old_value | new_key | new_value ]
  SetSize(0);
  page_id_array_[0] = old_value;    // 第一个 child
  key_array_[1]    = new_key;       // 第一个有效 key
  page_id_array_[1] = new_value;    // 第二个 child
  SetSize(2);                       // size = values 个数，此时有 2 个 child → 1 个有效 key
}

// 显式实例化
template class BPlusTreeInternalPage<GenericKey<4>, page_id_t, GenericComparator<4>>;
template class BPlusTreeInternalPage<GenericKey<8>, page_id_t, GenericComparator<8>>;
template class BPlusTreeInternalPage<GenericKey<16>, page_id_t, GenericComparator<16>>;
template class BPlusTreeInternalPage<GenericKey<32>, page_id_t, GenericComparator<32>>;
template class BPlusTreeInternalPage<GenericKey<64>, page_id_t, GenericComparator<64>>;

}  // namespace bustub