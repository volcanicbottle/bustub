//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.cpp
//
// Identification: src/buffer/lru_k_replacer.cpp
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_k_replacer.h"
#include <cstddef>
#include <limits>
#include <mutex>
#include "common/config.h"
#include "common/exception.h"
#include "common/macros.h"

namespace bustub {

LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(num_frames), k_(k) {
  node_store_.reserve(num_frames);
}

auto LRUKReplacer::Evict() -> std::optional<frame_id_t> {
  std::lock_guard<std::mutex> lock(latch_);
  if (curr_size_ == 0) return std::nullopt;
  frame_id_t to_clear_frame = -1;
  size_t max_dis = 0;
  size_t oldest_stamp = std::numeric_limits<size_t>::max();

  for (const auto &[id, node] : node_store_) {
    if (!node.IsEvictable()) {
      continue;  // 跳过不可驱逐的帧
    }
    size_t distance;
    if (node.GetHistory().size() < k_) {
      distance = std::numeric_limits<size_t>::max();
    } else {
      distance = current_timestamp_ - node.GetHistory().front();
    }
    if (distance > max_dis) {
      max_dis = distance;
      to_clear_frame = node.GetId();
      oldest_stamp = node.GetHistory().empty() ? 0 : node.GetHistory().front();
    } else if (distance == max_dis && distance == std::numeric_limits<size_t>::max()) {
      size_t frame_oldest = node.GetHistory().empty() ? 0 : node.GetHistory().front();
      if (frame_oldest < oldest_stamp) {
        oldest_stamp = frame_oldest;
      }
    }
  }
  if (to_clear_frame != -1) {
    node_store_.erase(to_clear_frame);
    curr_size_--;
    return to_clear_frame;
  }
  return std::nullopt;
}

void LRUKReplacer::RecordAccess(frame_id_t frame_id, [[maybe_unused]] AccessType access_type) {
  std::lock_guard<std::mutex> lock(latch_);
  BUSTUB_ASSERT(static_cast<size_t>(frame_id) < replacer_size_, "Frame ID is invalid");
  current_timestamp_++;
  if (node_store_.find(frame_id) == node_store_.end()) {
    node_store_.emplace(frame_id, LRUKNode(k_, frame_id));
  }
  node_store_.at(frame_id).AddAccess(current_timestamp_);
}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
  std::lock_guard<std::mutex> lock(latch_);
  BUSTUB_ASSERT(static_cast<size_t>(frame_id) < replacer_size_, "Frame ID is invalid");
  if (node_store_.find(frame_id) == node_store_.end()) {
    return;
  }
  bool curr_evic = node_store_.at(frame_id).IsEvictable();

  if (curr_evic == set_evictable) {
    return;
  }

  node_store_.at(frame_id).SetEvictable(set_evictable);

  if (curr_evic && !set_evictable) {
    curr_size_--;
  } else if (!curr_evic && set_evictable) {
    curr_size_++;
  }
}

void LRUKReplacer::Remove(frame_id_t frame_id) {
  std::lock_guard<std::mutex> lock(latch_);
  BUSTUB_ASSERT(static_cast<size_t>(frame_id) < replacer_size_, "Frame ID is invalid");

  auto it = node_store_.find(frame_id);
  if (it == node_store_.end()) return;

  if (it->second.IsEvictable()) {
    node_store_.erase(it);
    curr_size_--;
  } else {
    BUSTUB_ASSERT(false, "Cannot remove not evictable frame");
  }
}

auto LRUKReplacer::Size() -> size_t { return curr_size_; }

}  // namespace bustub
