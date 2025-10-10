//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/include/page/b_plus_tree_internal_page.h
//
// Copyright (c) 2018-2024, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#pragma once

#include <queue>
#include <string>

#include "storage/page/b_plus_tree_page.h"

namespace bustub {

#define B_PLUS_TREE_INTERNAL_PAGE_TYPE BPlusTreeInternalPage<KeyType, ValueType, KeyComparator>
#define INTERNAL_PAGE_HEADER_SIZE 12
#define INTERNAL_PAGE_SLOT_CNT \
  ((BUSTUB_PAGE_SIZE - INTERNAL_PAGE_HEADER_SIZE) / ((int)(sizeof(KeyType) + sizeof(ValueType))))  // NOLINT

/**
 * Store `n` indexed keys and `n + 1` child pointers (page_id) within internal page.
 * Pointer PAGE_ID(i) points to a subtree in which all keys K satisfy:
 * K(i) <= K < K(i+1).
 * NOTE: Since the number of keys does not equal to number of child pointers,
 * the first key in key_array_ always remains invalid. That is to say, any search / lookup
 * should ignore the first key.
 *
 * Internal page format (keys are stored in increasing order):
 *  ---------
 * | HEADER |
 *  ---------
 *  ------------------------------------------
 * | KEY(1)(INVALID) | KEY(2) | ... | KEY(n) |
 *  ------------------------------------------
 *  ---------------------------------------------
 * | PAGE_ID(1) | PAGE_ID(2) | ... | PAGE_ID(n) |
 *  ---------------------------------------------
 */
INDEX_TEMPLATE_ARGUMENTS
class BPlusTreeInternalPage : public BPlusTreePage {
 public:
  // Delete all constructor / destructor to ensure memory safety
  BPlusTreeInternalPage() = delete;
  BPlusTreeInternalPage(const BPlusTreeInternalPage &other) = delete;

  /**
   * Writes the necessary header information to a newly created page, must be called after
   * the creation of a new page to make a valid `BPlusTreeInternalPage`
   * @param max_size Maximal size of the page
   */
  void Init(int max_size = INTERNAL_PAGE_SLOT_CNT);

  /**
   * @param index The index of the key to get. Index must be non-zero.
   * @return Key at index
   */
  auto KeyAt(int index) const -> KeyType;

  /**
   * @param index The index of the key to set. Index must be non-zero.
   * @param key The new value for key
   */
  void SetKeyAt(int index, const KeyType &key);

  /**
   * @param value The value to search for
   * @return The index that corresponds to the specified value
   */
  auto ValueIndex(const ValueType &value) const -> int;

  /**
   * @param index The index to search for
   * @return The value at the index
   */
  auto ValueAt(int index) const -> ValueType;

  /**
   * @param index The index to set
   * @param value The new value
   */
  void SetValueAt(int index, const ValueType &value);

  /**
   * @brief Find the index of the key using binary search
   * @param key The key to search for
   * @param comparator The key comparator
   * @return The index where the key should be inserted
   */
  auto KeyIndex(const KeyType &key, const KeyComparator &comparator) const -> int;

  /**
   * @brief Lookup the value for a given key
   * @param key The key to lookup
   * @param comparator The key comparator
   * @return The value (page_id) for the key
   */
  auto Lookup(const KeyType &key, const KeyComparator &comparator) const -> ValueType;

  /**
   * @brief Insert a new key-value pair after an existing value
   * @param old_value The existing value to insert after
   * @param new_key The new key
   * @param new_value The new value
   * @return The new size
   */
  auto InsertNodeAfter(const ValueType &old_value, const KeyType &new_key, const ValueType &new_value) -> int;

  /**
   * @brief Remove a value at the specified index
   * @param index The index to remove
   */
  void Remove(int index);

  /**
   * @brief Remove and return the only child (for root shrinking)
   * @return The only child value
   */
  auto RemoveAndReturnOnlyChild() -> ValueType;

  /**
   * @brief Move half of the entries to another page (for splitting)
   * @param recipient The page to move entries to
   */
  void MoveHalfTo(BPlusTreeInternalPage *recipient);

  /**
   * @brief Copy n entries from another page
   * @param donor The source page
   * @param start_value_index The starting index in donor
   * @param n The number of entries to copy
   */
  void CopyNFrom(const BPlusTreeInternalPage *donor, int start_value_index, int n);

  /**
   * @brief Move all entries to another page (for merging)
   * @param recipient The page to move entries to
   */
  void MoveAllTo(BPlusTreeInternalPage *recipient);

  /**
   * @brief Populate a new root with two children
   * @param old_value The first child
   * @param new_key The key between children
   * @param new_value The second child
   */
  void PopulateNewRoot(const ValueType &old_value, const KeyType &new_key, const ValueType &new_value);

  /**
   * @brief For test only, return a string representing all keys in
   * this internal page, formatted as "(key1,key2,key3,...)"
   *
   * @return The string representation of all keys in the current internal page
   */
  auto ToString() const -> std::string {
    std::string kstr = "(";
    bool first = true;

    // First key of internal page is always invalid
    for (int i = 1; i < GetSize(); i++) {
      KeyType key = KeyAt(i);
      if (first) {
        first = false;
      } else {
        kstr.append(",");
      }

      kstr.append(std::to_string(key.ToString()));
    }
    kstr.append(")");

    return kstr;
  }

 private:
  // Array members for page data.
  KeyType key_array_[INTERNAL_PAGE_SLOT_CNT];
  ValueType page_id_array_[INTERNAL_PAGE_SLOT_CNT];
  // (Fall 2024) Feel free to add more fields and helper functions below if needed
};

}  // namespace bustub
