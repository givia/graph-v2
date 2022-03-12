#pragma once
#include "graph/graph.hpp"

//
// incidence(g,u) -> edge_view<VKey,Sourced,E,EV>:
//
// enable: for([vkey, uv]        : incidence(g,u))
//         for([vkey, uv]        : incidence(g,ukey))
//         for([vkey, uv, value] : incidence(g,u,   fn))
//         for([vkey, uv, value] : incidence(g,ukey,fn))
//
//         for([ukey, vkey, uv]        : sourced_incidence(g,u))
//         for([ukey, vkey, uv]        : sourced_incidence(g,ukey))
//         for([ukey, vkey, uv, value] : sourced_incidence(g,u,   fn))
//         for([ukey, vkey, uv, value] : sourced_incidence(g,ukey,fn))
//
namespace std::graph::views {


template <class G, bool Sourced = false, class EVF = void>
requires(!Sourced || (Sourced && sourced_incidence_graph<G>)) //
      class incidence_iterator;


/// <summary>
/// Iterator for an incidence range of edges for a vertex.
/// </summary>
/// <typeparam name="G">Graph type</typeparam>
/// <typeparam name="EVF">Edge Value Function</typeparam>
template <class G, bool Sourced, class EVF>
requires(!Sourced || (Sourced && sourced_incidence_graph<G>)) //
      class incidence_iterator {
public:
  using graph_type      = G;
  using vertex_type     = vertex_t<graph_type>;
  using vertex_key_type = vertex_key_t<graph_type>;

  using edge_range          = vertex_edge_range_t<graph_type>;
  using edge_iterator       = vertex_edge_iterator_t<graph_type>;
  using edge_type           = edge_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using edge_value_type     = invoke_result_t<EVF, edge_reference_type>;

  using iterator_category = forward_iterator_tag;
  using value_type        = edge_view<const vertex_key_type, Sourced, edge_reference_type, edge_value_type>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

public:
  incidence_iterator(graph_type& g, edge_iterator iter, const EVF& value_fn)
        : g_(g), iter_(iter), value_fn_(&value_fn) {}
  incidence_iterator(graph_type& g, vertex_type& u, const EVF& value_fn)
        : incidence_iterator(g, ranges::begin(edges(g, u)), value_fn) {}

  constexpr incidence_iterator()                          = default;
  constexpr incidence_iterator(const incidence_iterator&) = default;
  constexpr incidence_iterator(incidence_iterator&&)      = default;
  constexpr ~incidence_iterator()                         = default;

  constexpr incidence_iterator& operator=(const incidence_iterator&) = default;
  constexpr incidence_iterator& operator=(incidence_iterator&&) = default;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_edge_type = remove_reference_t<edge_reference_type>;
  using shadow_value_type =
        edge_view<vertex_key_type, Sourced, shadow_edge_type*, _detail::ref_to_ptr<edge_value_type>>;

public:
  constexpr reference operator*() const {
    if constexpr (Sourced && sourced_incidence_graph<G>)
      value_ = {source_key(g_, *iter_), target_key(g_, *iter_), &*iter_, invoke(*value_fn_, *iter_)};
    else
      value_ = {target_key(g_, *iter_), &*iter_, invoke(*value_fn_, *iter_)};
    return reinterpret_cast<reference>(value_);
  }

  constexpr incidence_iterator& operator++() {
    ++iter_;
    return *this;
  }
  constexpr incidence_iterator operator++(int) const {
    incidence_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const incidence_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const incidence_iterator& rhs) const { return iter_ == rhs; }

private: // member variables
  mutable shadow_value_type        value_ = {};
  _detail::ref_to_ptr<graph_type&> g_;
  edge_iterator                    iter_;
  const EVF*                       value_fn_ = nullptr;

  friend bool operator==(const edge_iterator& lhs, const incidence_iterator& rhs) { return lhs == rhs.iter_; }
};


template <class G, bool Sourced>
requires(!Sourced || (Sourced && sourced_incidence_graph<G>)) //
      class incidence_iterator<G, Sourced, void> {
public:
  using graph_type      = G;
  using vertex_type     = vertex_t<graph_type>;
  using vertex_key_type = vertex_key_t<graph_type>;

  using edge_range          = vertex_edge_range_t<graph_type>;
  using edge_iterator       = vertex_edge_iterator_t<graph_type>;
  using edge_type           = edge_t<graph_type>;
  using edge_reference_type = edge_reference_t<graph_type>;
  using edge_value_type     = void;

  using iterator_category = forward_iterator_tag;
  using value_type        = edge_view<const vertex_key_type, Sourced, edge_reference_type, edge_value_type>;
  using difference_type   = ranges::range_difference_t<edge_range>;
  using pointer           = value_type*;
  using const_pointer     = const value_type*;
  using reference         = value_type&;
  using const_reference   = const value_type&;
  using rvalue_reference  = value_type&&;

protected:
  // avoid difficulty in undefined vertex reference value in value_type
  // shadow_vertex_value_type: ptr if vertex_value_type is ref or ptr, value otherwise
  using shadow_edge_type  = remove_reference_t<edge_reference_type>;
  using shadow_value_type = edge_view<vertex_key_type, Sourced, shadow_edge_type*, edge_value_type>;

public:
  incidence_iterator(graph_type& g, edge_iterator iter) : g_(g), iter_(iter) {}
  incidence_iterator(graph_type& g, vertex_type& u) : incidence_iterator(g, ranges::begin(edges(g, u))) {}

  constexpr incidence_iterator()                          = default;
  constexpr incidence_iterator(const incidence_iterator&) = default;
  constexpr incidence_iterator(incidence_iterator&&)      = default;
  constexpr ~incidence_iterator()                         = default;

  constexpr incidence_iterator& operator=(const incidence_iterator&) = default;
  constexpr incidence_iterator& operator=(incidence_iterator&&) = default;

public:
  constexpr reference operator*() const {
    if constexpr (Sourced && sourced_incidence_graph<G>)
      value_ = {source_key(g_, *iter_), target_key(g_, *iter_), &*iter_};
    else {
      value_ = {target_key(g_, *iter_), &*iter_};
    }
    return reinterpret_cast<reference>(value_);
  }

  constexpr incidence_iterator& operator++() {
    ++iter_;
    return *this;
  }
  constexpr incidence_iterator operator++(int) const {
    incidence_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  constexpr bool operator==(const incidence_iterator& rhs) const { return iter_ == rhs.iter_; }
  //constexpr bool operator==(const incidence_iterator& rhs) const { return iter_ == rhs; }

private: // member variables
  mutable shadow_value_type        value_ = {};
  _detail::ref_to_ptr<graph_type&> g_;
  edge_iterator                    iter_;

  friend bool operator==(const edge_iterator& lhs, const incidence_iterator& rhs) { return lhs == rhs.iter_; }
};


template <class G, bool Sourced, class EVF>
using incidence_view = ranges::subrange<incidence_iterator<G, Sourced, EVF>, vertex_edge_iterator_t<G>>;

namespace access {
  // ranges
  TAG_INVOKE_DEF(incidence); // incidence(g,u)               -> edges[vkey,v]
                             // incidence(g,ukey)            -> edges[vkey,v]
                             // incidence(g,u,   fn)         -> edges[vkey,v,value]
                             // incidence(g,ukey,fn)         -> edges[vkey,v,value]

  template <class G>
  concept _has_incidence_g_u_adl = vertex_range<G> && requires(G&& g, vertex_reference_t<G> u) {
    {incidence(g, u)};
  };
  template <class G>
  concept _has_incidence_g_ukey_adl = vertex_range<G> && requires(G&& g, vertex_key_t<G> ukey) {
    {incidence(g, ukey)};
  };
  template <class G, class EVF>
  concept _has_incidence_g_u_evf_adl = vertex_range<G> && requires(G&& g, vertex_reference_t<G> u, const EVF& evf) {
    {incidence(g, u, evf)};
  };
  template <class G, class EVF>
  concept _has_incidence_g_ukey_evf_adl = vertex_range<G> && requires(G&& g, vertex_key_t<G> ukey, const EVF& evf) {
    {incidence(g, ukey, evf)};
  };

  TAG_INVOKE_DEF(sourced_incidence); // sourced_incidence(g,u)       -> edges[ukey,vkey,v]
                                     // sourced_incidence(g,ukey)    -> edges[ukey,vkey,v]
                                     // sourced_incidence(g,u,   fn) -> edges[ukey,vkey,v,value]
                                     // sourced_incidence(g,ukey,fn) -> edges[ukey,vkey,v,value]

  template <class G>
  concept _has_sourced_incidence_g_u_adl = vertex_range<G> && requires(G&& g, vertex_reference_t<G> u) {
    {sourced_incidence(g, u)};
  };
  template <class G>
  concept _has_sourced_incidence_g_ukey_adl = vertex_range<G> && requires(G&& g, vertex_key_t<G> ukey) {
    {sourced_incidence(g, ukey)};
  };
  template <class G, class EVF>
  concept _has_sourced_incidence_g_u_evf_adl = vertex_range<G> &&
        requires(G&& g, vertex_reference_t<G> u, const EVF& evf) {
    {sourced_incidence(g, u, evf)};
  };
  template <class G, class EVF>
  concept _has_sourced_incidence_g_ukey_evf_adl = vertex_range<G> &&
        requires(G&& g, vertex_key_t<G> ukey, const EVF& evf) {
    {sourced_incidence(g, ukey, evf)};
  };

} // namespace access

//
// incidence(g,u)
// incidence(g,ukey)
//
template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto incidence(G&& g, vertex_reference_t<G> u) {
  if constexpr (access::_has_incidence_g_u_adl<G>) {
    return access::incidence(g, u);
  } else {
    using iterator_type = incidence_iterator<G, false, void>;
    return incidence_view<G, false, void>(iterator_type(g, ranges::begin(edges(g, u))), ranges::end(edges(g, u)));
  }
}

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto incidence(G&& g, vertex_key_t<G> ukey) {
  if constexpr (access::_has_incidence_g_ukey_adl<G>)
    return access::incidence(g, ukey);
  else
    return incidence(g, *find_vertex(g, ukey));
}


//
// incidence(g,u,evf)
// incidence(g,ukey,evf)
//
template <class G, class EVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto incidence(G&& g, vertex_reference_t<G> u, const EVF& evf) {
  if constexpr (access::_has_incidence_g_u_evf_adl<G, EVF>) {
    return access::incidence(g, u, evf);
  } else {
    using iterator_type = incidence_iterator<G, false, EVF>;
    return incidence_view<G, false, EVF>(iterator_type(g, ranges::begin(edges(g, u)), evf), ranges::end(edges(g, u)));
  }
}

template <class G, class EVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto incidence(G&& g, vertex_key_t<G> ukey, const EVF& evf) {
  if constexpr (access::_has_incidence_g_ukey_evf_adl<G, EVF>)
    return access::incidence(g, ukey, evf);
  else
    return incidence(g, *find_vertex(g, ukey), evf);
}


//
// sourced_incidence(g,u)
// sourced_incidence(g,ukey)
//
template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto sourced_incidence(G&& g, vertex_reference_t<G> u) {
  if constexpr (access::_has_sourced_incidence_g_u_adl<G>)
    return access::sourced_incidence(g, u);
  else {
    using iterator_type = incidence_iterator<G, true, void>;
    return incidence_view<G, true, void>(iterator_type(g, ranges::begin(edges(g, u))), ranges::end(edges(g, u)));
  }
}

template <class G>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto sourced_incidence(G&& g, vertex_key_t<G> ukey) {
  if constexpr (access::_has_sourced_incidence_g_ukey_adl<G>)
    return access::sourced_incidence(g, ukey);
  else
    return sourced_incidence(g, *find_vertex(g, ukey));
}


//
// sourced_incidence(g,u,evf)
// sourced_incidence(g,ukey,evf)
//
template <class G, class EVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto sourced_incidence(G&& g, vertex_reference_t<G> u, const EVF& evf) {
  if constexpr (access::_has_sourced_incidence_g_u_evf_adl<G, EVF>) {
    return access::sourced_incidence(g, u, evf);
  } else {
    using iterator_type = incidence_iterator<G, true, EVF>;
    return incidence_view<G, true, EVF>(iterator_type(g, ranges::begin(edges(g, u)), evf), ranges::end(edges(g, u)));
  }
}

template <class G, class EVF>
requires ranges::forward_range<vertex_range_t<G>>
constexpr auto sourced_incidence(G&& g, vertex_key_t<G> ukey, const EVF& evf) {
  if constexpr (access::_has_sourced_incidence_g_ukey_evf_adl<G, EVF>)
    return access::sourced_incidence(g, ukey, evf);
  else
    return sourced_incidence(g, *find_vertex(g, ukey), evf);
}


} // namespace std::graph::views
