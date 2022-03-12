#include <catch2/catch.hpp>
#include "csv_routes.hpp"
#include "graph/graph.hpp"
#include "graph/views/neighbors.hpp"
#include "graph/container/csr_graph.hpp"

using std::ranges::forward_range;
using std::remove_reference_t;
using std::is_const_v;
using std::is_lvalue_reference_v;
using std::forward_iterator;
using std::input_iterator;

using std::graph::vertex_t;
using std::graph::vertex_key_t;
using std::graph::vertex_value_t;
using std::graph::vertex_edge_range_t;
using std::graph::vertex_reference_t;
using std::graph::edge_t;
using std::graph::edge_value_t;
using std::graph::edge_reference_t;

using std::graph::graph_value;
using std::graph::vertices;
using std::graph::edges;
using std::graph::vertex_key;
using std::graph::vertex_value;
using std::graph::target_key;
using std::graph::target;
using std::graph::edge_value;
using std::graph::degree;
using std::graph::find_vertex;
using std::graph::find_vertex_edge;

using routes_csr_graph_type = std::graph::container::csr_graph<double, std::string, std::string>;

template <typename G>
constexpr auto find_frankfurt_key(const G& g) {
  return find_city_key(g, "Frankf\xC3\xBCrt");
}

template <typename G>
auto find_frankfurt(G&& g) {
  return find_city(g, "Frankf\xC3\xBCrt");
}

#if 0
// Things to test
//  csr_graph with VV=void (does it compile?)
//  push_back and emplace_back work correctly when adding city names (applies to csr_graph & dynamic_graph)

TEST_CASE("neighbors test", "[csr][neighbors]") {

  init_console();

  using G  = routes_csr_graph_type;
  auto&& g = load_ordered_graph<G>(TEST_DATA_ROOT_DIR "germany_routes.csv", name_order_policy::source_order_found);
  // name_order_policy::source_order_found gives best output with least overlap for germany routes

  const auto frankfurt     = find_frankfurt(g);
  const auto frankfurt_key = find_frankfurt_key(g);

  SECTION("non-const neighbors_iterator") {
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(g)>>);
    static_assert(!std::is_const_v<G>);

    REQUIRE(frankfurt);
    vertex_t<G>& u = **frankfurt;

    std::graph::views::neighbors_iterator<G> i0; // default construction
    std::graph::views::neighbors_iterator<G> i1(g, u);
    static_assert(std::forward_iterator<decltype(i1)>, "neighbors_iterator must be a forward_iterator");
    static_assert(std::is_move_assignable_v<decltype(i0)>, "neighbors_iterator must be move_assignable");
    static_assert(std::is_copy_assignable_v<decltype(i0)>, "neighbors_iterator must be copy_assignable");
    {
      auto&& [vkey, uv] = *i1;
      static_assert(is_const_v<decltype(vkey)>, "vertex key must be const");
      static_assert(!is_const_v<remove_reference_t<decltype(uv)>>, "edge must be non-const");
      REQUIRE(vkey == 1);
    }
    {
      auto&& [vkey, uv] = *++i1;
      REQUIRE(vkey == 4);
      auto i1b = i1;
      REQUIRE(i1b == i1);
    }

    std::graph::views::neighbors_iterator<G> i2(g, u);
    {
      auto&& [vkey, uv] = *i2;
      static_assert(is_const_v<decltype(vkey)>, "vertex key must be const");
      static_assert(is_lvalue_reference_v<decltype(uv)>, "edge must be lvalue reference");
      static_assert(!is_const_v<remove_reference_t<decltype(uv)>>, "edge must be non-const");
      REQUIRE(vkey == 1);
    }
    {
      auto&& [vkey, uv] = *++i2;
      REQUIRE(vkey == 4);
      auto i2b = i2;
      REQUIRE(i2b == i2);
    }

    static_assert(std::input_or_output_iterator<decltype(i1)>);
    using _It = std::graph::views::neighbors_iterator<G>;
    using _Se = std::graph::vertex_iterator_t<G>;
    bool yy   = std::sentinel_for<_Se, _It>;
    bool xx   = std::sized_sentinel_for<_Se, _It>;
    static_assert(std::sized_sentinel_for<_Se, _It> == false);
    auto _Ki =
          std::sized_sentinel_for<_Se, _It> ? std::ranges::subrange_kind::sized : std::ranges::subrange_kind::unsized;

    auto evf  = [&g](edge_t<G>& uv) -> double& { return edge_value(g, uv); };
    using EVF = decltype(evf);

    std::graph::views::neighbors_iterator<G, false, EVF> i3(g, begin(edges(g, u)), evf);
    {
      // The following asserts are used to isolate problem with failing input_or_output_iterator concept for neighbors_iterator
      static_assert(std::movable<decltype(i3)>, "neighbors_iterator<G,EVF> is NOT movable");
      static_assert(std::default_initializable<decltype(i3)>, "neighbors_iterator<G,EVF> is NOT default_initializable");
      //static_assert(std::__detail::__is_signed_integer_like<std::iter_difference_t<decltype(i3)>>, "neighbors_iterator<G,EVF> is NOT __is_signed_integer_like");
      static_assert(std::weakly_incrementable<decltype(i3)>, "neighbors_iterator<G,EVF> is NOT weakly_incrementable");
      static_assert(std::input_or_output_iterator<decltype(i3)>,
                    "neighbors_iterator<G,EVF> is NOT an input_or_output_iterator");

      auto&& [vkey, uv, km] = *i3;
      REQUIRE(vkey == 1);
      REQUIRE(km == 85.0);
    }
    {
      auto&& [vkey, uv, km] = *++i3;
      REQUIRE(vkey == 4);
      REQUIRE(km == 217.0);
    }

    //std::graph::views::neighbors_iterator<const G> j0;
    //j0 = i0;
    //i0 == j0;
  }

  SECTION("const neighbors_iterator") {
    using G2 = const G;
    G2& g2   = g;
    static_assert(std::is_const_v<std::remove_reference_t<decltype(g2)>>, "graph must be const");

    vertex_t<G2>& u = **frankfurt;

    //std::graph::views::neighbors_iterator<G2> i0; // default construction
    std::graph::views::neighbors_iterator<G2, false> i1(g2, u);
    static_assert(std::forward_iterator<decltype(i1)>, "neighbors_iterator must be a forward_iterator");
    {
      auto&& [vkey, uv] = *i1;

      edge_reference_t<G2> uv2 = uv;
      static_assert(is_const_v<remove_reference_t<decltype(uv2)>>, "edge must be const");

      static_assert(is_const_v<decltype(vkey)>, "key must be const");
      static_assert(is_lvalue_reference_v<decltype(uv)>, "edge must be lvalue reference");
      static_assert(is_const_v<remove_reference_t<decltype(uv)>>, "edge must be const");
      REQUIRE(vkey == 1);
    }
    {
      auto&& [vkey, uv] = *++i1;
      REQUIRE(vkey == 4);
      auto i1b = i1;
      REQUIRE(i1b == i1);
    }

    std::graph::views::neighbors_iterator<G2, false> i2(g2, u);
    {
      auto&& [vkey, uv] = *i2;
      static_assert(is_const_v<decltype(vkey)>, "key must be const");
      static_assert(is_const_v<remove_reference_t<decltype(uv)>>, "edge must be const");
      REQUIRE(vkey == 1);
    }
    {
      auto&& [vkey, uv] = *++i2;
      REQUIRE(vkey == 4);
      auto i2b = i2;
      REQUIRE(i2b == i2);
    }

    auto evf  = [&g2](edge_reference_t<G2> uv) -> const double& { return edge_value(g2, uv); };
    using EVF = decltype(evf);
    std::graph::views::neighbors_iterator<G2, false, EVF> i3(g2, begin(edges(g2, u)), evf);
    {
      auto&& [vkey, uv, km] = *i3;
      REQUIRE(vkey == 1);
      REQUIRE(km == 85.0);
    }
    {
      auto&& [vkey, uv, km] = *++i3;
      REQUIRE(vkey == 4);
      REQUIRE(km == 217);
    }
  }

  SECTION("non-const neighbors") {
    vertex_t<G>& u = **frankfurt;
    using view_t   = decltype(std::graph::views::neighbors(g, u));
    static_assert(forward_range<view_t>, "neighbors(g) is not a forward_range");
    size_t cnt = 0;
    for (auto&& [vkey, uv] : std::graph::views::neighbors(g, u)) {
      ++cnt;
    }
    REQUIRE(cnt == size(edges(g, u)));
  }

  SECTION("const neighbors") {
    using G2                  = const G;
    G2&                    g2 = g;
    vertex_reference_t<G2> u  = **frankfurt;

    using view_t = decltype(std::graph::views::neighbors(g2, u));
    static_assert(forward_range<view_t>, "neighbors(g) is not a forward_range");
    size_t cnt = 0;
    for (auto&& [vkey, uv] : std::graph::views::neighbors(g2, u)) {
      ++cnt;
    }
    REQUIRE(cnt == size(edges(g2, u)));
  }

  SECTION("non-const neighbors with vertex_fn") {
    // Note: must include trailing return type on lambda
    vertex_reference_t<G> u       = **frankfurt;
    size_t                cnt     = 0;
    auto                  edge_fn = [&g](edge_reference_t<G> uv) -> double& { return edge_value(g, uv); };
    for (auto&& [vkey, uv, val] : std::graph::views::neighbors(g, u, edge_fn)) {
      ++cnt;
    }
    REQUIRE(cnt == size(edges(g, u)));
  }
  SECTION("const neighbors with vertex_fn") {
    // Note: must include trailing return type on lambda
    using G2                       = const G;
    G2&                    g2      = g;
    vertex_reference_t<G2> u       = **frankfurt;
    auto                   edge_fn = [&g2](edge_reference_t<G2> uv) -> const double& { return edge_value(g2, uv); };
    size_t                 cnt     = 0;
    for (auto&& [vkey, uv, val] : std::graph::views::neighbors(g2, u, edge_fn)) {
      ++cnt;
    }
    REQUIRE(cnt == size(edges(g2, u)));
  }
}

#endif //0
