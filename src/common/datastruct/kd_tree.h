#pragma once

#include <memory>
#include <utility>
#include <vector>

namespace beacon
{
using point_t = std::vector<double>;
using point_arr_t = std::vector<point_t>;

using index_arr_t = std::vector<std::size_t>;

// point_index_t 是对当前点的一个封装，
using point_index_t = std::pair<std::vector<double>, std::size_t>;


class KDNode
{
public:
  using KDNodePtr = std::shared_ptr<KDNode>;

  // index 用于表示当前点的一个 index(索引，类似于 hash)
  std::size_t index_{0};
  point_t point_;
  KDNodePtr left_;
  KDNodePtr right_;

  // initializer 构造函数
  KDNode() = default;

  KDNode(point_t point, const size_t& index, KDNodePtr left, KDNodePtr right)
    : index_(index)
    , point_(std::move(point))
    , left_(std::move(left))
    , right_(std::move(right))
  {}

  KDNode(const point_index_t& point_index, KDNodePtr left, KDNodePtr right)
    : index_(point_index.second)
    , point_(point_index.first)
    , left_(std::move(left))
    , right_(std::move(right))
  {}

  ~KDNode() = default;

  double Coord(const size_t& index)
  {
    return point_.at(index);
  }

  explicit operator bool() const
  {
    return !point_.empty();
  }

  explicit operator size_t() const
  {
    return index_;
  }

  explicit operator point_t() const
  {
    return point_;
  }

  explicit operator point_index_t()
  {
    return point_index_t{point_, index_};
  }
};


using KDNodePtr = std::shared_ptr<KDNode>;

// square euclidean distance
inline double dist2(const point_t&, const point_t&);
inline double dist2(const KDNodePtr&, const KDNodePtr&);

// euclidean distance
inline double dist(const point_t&, const point_t&);
inline double dist(const KDNodePtr&, const KDNodePtr&);



} // namespace beacon