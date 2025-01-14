#pragma once
#include "graph/graph.hpp"
#include "graph/views/views_utility.hpp"

//
// vertexlist(g) -> vertex_view<VId,V,VV> -> {id, vertex& [,value]}
//
// given:    vvf = [&g](vertex_reference_t<G> u) -> decl_type(vertex_value(g)) { return vertex_value(g,u);}
//           (trailing return type is required if defined inline as vertexlist parameter)
//
//           vertex_iterator<G> first = ..., last = ...;
//
// examples: for(auto&& [uid, u]      : vertexlist(g))
//         : for(auto&& [uid, u, val] : vertexlist(g,vvf)
//
//         : for(auto&& [uid, u]      : vertexlist(g, first, last))
//         : for(auto&& [uid, u, val] : vertexlist(g, first, last, vvf)
//
//         : for(auto&& [uid, u]      : vertexlist(g, vr))
//         : for(auto&& [uid, u, val] : vertexlist(g, vr, vvf)
//
namespace std::graph {

template <adjacency_list G, class VVF = void>
requires ranges::forward_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class vertexlist_iterator;


template <adjacency_list G, class VVF>
requires ranges::forward_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class vertexlist_iterator {
public:
  using graph_type            = G;
  using vertex_id_type        = vertex_id_t<graph_type>;
  using vertex_range_type     = vertex_range_t<graph_type>;
  using vertex_iterator_type  = ranges::iterator_t<vertex_range_type>;
  using vertex_type           = vertex_t<graph_type>;
  using vertex_reference_type = ranges::range_reference_t<vertex_range_type>;
  using vertex_value_func     = VVF;
  using vertex_value_type     = invoke_result_t<VVF, vertex_type&>;

  using iterator_category = forward_iterator_tag;
  using value_type        = vertex_view<const vertex_id_t<graph_type>, vertex_reference_type, vertex_value_type>;
  using difference_type   = ranges::range_difference_t<vertex_range_type>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;

protected:
  // use of shadow_vertex_type avoids difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_vertex_type = remove_reference_t<vertex_reference_type>;
  using shadow_value_type =
        vertex_view<vertex_id_t<graph_type>, shadow_vertex_type*, _detail::ref_to_ptr<vertex_value_type>>;

public:
  vertexlist_iterator(graph_type& g, const VVF& value_fn, vertex_iterator_type iter, vertex_id_type start_at = 0)
        : iter_(iter), value_fn_(&value_fn) {
    value_.id = start_at;
  }

  constexpr vertexlist_iterator()                           = default;
  constexpr vertexlist_iterator(const vertexlist_iterator&) = default;
  constexpr vertexlist_iterator(vertexlist_iterator&&)      = default;
  constexpr ~vertexlist_iterator()                          = default;

  constexpr vertexlist_iterator& operator=(const vertexlist_iterator&) = default;
  constexpr vertexlist_iterator& operator=(vertexlist_iterator&&)      = default;

public:
  constexpr reference operator*() const {
    value_.vertex = &*iter_;
    if constexpr (!is_void_v<vertex_value_type>)
      value_.value = invoke(*this->value_fn_, *iter_);
    return reinterpret_cast<reference>(value_);
  }

  constexpr vertexlist_iterator& operator++() {
    ++iter_;
    ++value_.id;
    // leave value_.vertex as-is to avoid dereferencing iter_ when it's at end()
    return *this;
  }
  constexpr vertexlist_iterator operator++(int) const {
    vertexlist_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const vertexlist_iterator& rhs) const { return iter_ == rhs.iter_; }

protected:
  mutable shadow_value_type value_ = {};
  vertex_iterator_type      iter_;
  const VVF*                value_fn_ = nullptr;

  friend bool operator==(const vertex_iterator_type& lhs, const vertexlist_iterator& rhs) { return lhs == rhs.iter_; }
};


template <adjacency_list G>
requires ranges::forward_range<vertex_range_t<G>> && integral<vertex_id_t<G>>
class vertexlist_iterator<G, void> {
public:
  using graph_type            = G;
  using vertex_id_type        = vertex_id_t<graph_type>;
  using vertex_range_type     = vertex_range_t<graph_type>;
  using vertex_iterator_type  = ranges::iterator_t<vertex_range_type>;
  using vertex_type           = vertex_t<graph_type>;
  using vertex_reference_type = ranges::range_reference_t<vertex_range_type>;
  using vertex_value_func     = void;
  using vertex_value_type     = void;

  using iterator_category = forward_iterator_tag;
  using value_type        = vertex_view<const vertex_id_type, vertex_reference_type, void>;
  using difference_type   = ranges::range_difference_t<vertex_range_type>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_vertex_type = remove_reference_t<vertex_reference_type>;
  using shadow_value_type  = vertex_view<vertex_id_type, shadow_vertex_type*, void>;

public:
  vertexlist_iterator(graph_type& g) : iter_(ranges::begin(vertices(const_cast<graph_type&>(g)))) {}
  vertexlist_iterator(vertex_iterator_type iter, vertex_id_type start_at = 0)
        : value_{start_at, nullptr}, iter_(iter) {}

  constexpr vertexlist_iterator()                           = default;
  constexpr vertexlist_iterator(const vertexlist_iterator&) = default;
  constexpr vertexlist_iterator(vertexlist_iterator&&)      = default;
  constexpr ~vertexlist_iterator()                          = default;

  constexpr vertexlist_iterator& operator=(const vertexlist_iterator&) = default;
  constexpr vertexlist_iterator& operator=(vertexlist_iterator&&)      = default;

public:
  constexpr reference operator*() const {
    value_.vertex = &*iter_;
    if constexpr (!is_void_v<vertex_value_type>)
      value_.value = this->value_fn_(*iter_);
    return reinterpret_cast<reference>(value_);
  }

  constexpr vertexlist_iterator& operator++() {
    ++iter_;
    ++value_.id;
    // leave value_.vertex as-is to avoid dereferencing iter_ to get value_.vertex when it's at end()
    return *this;
  }
  constexpr vertexlist_iterator operator++(int) const {
    vertexlist_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const vertexlist_iterator& rhs) const { return iter_ == rhs.iter_; }

protected:
  mutable shadow_value_type value_ = {};
  vertex_iterator_type      iter_;

  friend bool operator==(const vertex_iterator_type& lhs, const vertexlist_iterator& rhs) { return lhs == rhs.iter_; }
};


template <adjacency_list G, class VVF = void>
using vertexlist_view = ranges::subrange<vertexlist_iterator<G, VVF>, vertex_iterator_t<G>>;
} // namespace std::graph

namespace std::graph::tag_invoke {
// ranges
TAG_INVOKE_DEF(vertexlist); // vertexlist(g)               -> vertices[uid,u]
                            // vertexlist(g,fn)            -> vertices[uid,u,value]
                            // vertexlist(g,first,last)    -> vertices[uid,u]
                            // vertexlist(g,first,last,fn) -> vertices[uid,u,value]

template <class G>
concept _has_vertexlist_g_adl = vertex_range<G> && requires(G&& g) {
                                                     { vertexlist(g) };
                                                   };

template <class G, class VVF>
concept _has_vertexlist_g_fn_adl =
      vertex_range<G> && invocable<VVF, vertex_reference_t<G>> && requires(G&& g, const VVF& fn) {
                                                                    { vertexlist(g, fn) };
                                                                  };

template <class G>
concept _has_vertexlist_i_i_adl = vertex_range<G> && requires(G&& g, vertex_iterator_t<G> ui, vertex_iterator_t<G> vi) {
                                                       { vertexlist(g, ui, vi) };
                                                     };

template <class G, class VVF>
concept _has_vertexlist_i_i_fn_adl = vertex_range<G> && invocable<VVF, vertex_reference_t<G>> &&
                                     requires(G&& g, vertex_iterator_t<G> ui, vertex_iterator_t<G> vi, const VVF& fn) {
                                       { vertexlist(g, ui, vi, fn) };
                                     };

template <class G, class VR>
concept _has_vertexlist_vrng_adl =
      vertex_range<G> && ranges::random_access_range<VR> && requires(G&& g, vertex_range_t<G>& vr) {
                                                              { vertexlist(g, vr) };
                                                            };

template <class G, class VR, class VVF>
concept _has_vertexlist_vrng_fn_adl =
      vertex_range<G> && ranges::random_access_range<VR> && convertible_to<ranges::range_value_t<VR>, vertex_t<G>> &&
      invocable<VVF, vertex_reference_t<G>> && requires(G&& g, VR&& vr, const VVF& fn) {
                                                 { vertexlist(g, vr, fn) };
                                               };


} // namespace std::graph::tag_invoke

namespace std::graph::views {
//
// vertexlist(g [,proj])
//
template <adjacency_list G>
constexpr auto vertexlist(G&& g) {
  if constexpr (std::graph::tag_invoke::_has_vertexlist_g_adl<G>)
    return std::graph::tag_invoke::vertexlist(forward<G>(g));
  else
    return vertexlist_view<G>(vertices(forward<G>(g)));
}

template <adjacency_list G, class VVF>
requires invocable<VVF, vertex_reference_t<G>>
constexpr auto vertexlist(G&& g, const VVF& value_fn) {
  using iterator_type = vertexlist_iterator<G, VVF>;
  if constexpr (std::graph::tag_invoke::_has_vertexlist_g_fn_adl<G, VVF>)
    return std::graph::tag_invoke::vertexlist(forward<G>(g), value_fn);
  else
    return vertexlist_view<G, VVF>(iterator_type(forward<G>(g), value_fn, begin(vertices(forward<G>(g)))),
                                   end(vertices(forward<G>(g))));
}

//
// vertexlist(g, first, last [,proj])
//
template <adjacency_list G>
requires ranges::random_access_range<vertex_range_t<G>>
constexpr auto vertexlist(G&& g, vertex_iterator_t<G> first, vertex_iterator_t<G> last) {
  using iterator_type = vertexlist_iterator<G>;
  if constexpr (std::graph::tag_invoke::_has_vertexlist_i_i_adl<G>)
    return std::graph::tag_invoke::vertexlist(forward<G>(g), first, last);
  else
    return vertexlist_view<G>(iterator_type(first, static_cast<vertex_id_t<G>>(first - begin(vertices(g)))), last);
}

template <adjacency_list G, class VVF>
requires ranges::random_access_range<vertex_range_t<G>> && invocable<VVF, vertex_reference_t<G>>
constexpr auto vertexlist(G&& g, vertex_iterator_t<G> first, vertex_iterator_t<G> last, const VVF& value_fn) {
  using iterator_type = vertexlist_iterator<G, VVF>;
  if constexpr (std::graph::tag_invoke::_has_vertexlist_i_i_fn_adl<G, VVF>)
    return std::graph::tag_invoke::vertexlist(forward<G>(g), first, last, value_fn);
  else {
    auto first_id = static_cast<vertex_id_t<G>>(first - begin(vertices(g)));
    return vertexlist_view<G, VVF>(iterator_type(forward<G>(g), value_fn, first), last, first_id);
  }
}

//
// vertexlist(g, ur, [,proj])
//
template <adjacency_list G, ranges::random_access_range VR>
requires ranges::random_access_range<vertex_range_t<G>>
constexpr auto vertexlist(G&& g, VR&& vr) {
  using iterator_type = vertexlist_iterator<G>;
  if constexpr (std::graph::tag_invoke::_has_vertexlist_vrng_adl<G, VR>)
    return std::graph::tag_invoke::vertexlist(forward<G>(g), forward<VR>(vr));
  else {
    auto first    = ranges::begin(vr);
    auto last     = ranges::end(vr);
    auto first_id = static_cast<vertex_id_t<G>>(first - begin(vertices(g)));
    return vertexlist_view<G>(iterator_type(first, first_id), last);
  }
}

template <adjacency_list G, ranges::random_access_range VR, class VVF>
requires invocable<VVF, vertex_reference_t<G>> &&
         convertible_to<ranges::iterator_t<VR>, vertex_iterator_t<G>> // allow for ranges::subrange of vertices
constexpr auto vertexlist(G&& g, VR&& vr, const VVF& value_fn) {
  using iterator_type = vertexlist_iterator<G, VVF>;
  if constexpr (std::graph::tag_invoke::_has_vertexlist_vrng_fn_adl<G, VR, VVF>)
    return std::graph::tag_invoke::vertexlist(forward(g), forward(vr), value_fn);
  else {
    auto first    = ranges::begin(vr);
    auto last     = ranges::end(vr);
    auto first_id = static_cast<vertex_id_t<G>>(first - begin(vertices(g)));
    return vertexlist_view<G, VVF>(iterator_type(forward<G>(g), value_fn, first, first_id), last);
  }
}


} // namespace std::graph::views
