#ifndef PATTERN_GENERATOR_HH
#define PATTERN_GENERATOR_HH

#include <vector>
#include "Graph.hh"
#include "caf/actor_ostream.hpp"
#include "caf/actor_system.hpp"
#include "caf/caf_main.hpp"
#include "caf/event_based_actor.hpp"
#include "caf/all.hpp"
#include "caf/io/all.hpp"

using namespace caf;

namespace Peregrine
{
  using Edges = std::vector<std::pair<uint32_t,uint32_t>>;
  struct PatternGenerator
  {
    PatternGenerator();
    static SmallGraph clique(uint32_t sz);
    static SmallGraph star(uint32_t sz);
    static std::vector<SmallGraph> all(uint32_t sz, bool vertex_based, bool anti_edges);
    static std::vector<SmallGraph> extend(const std::vector<SmallGraph> &from, bool vertex_based, bool overwrite_anti_edges = OVERWRITE_ANTI_EDGES);
    bool is_connected_pattern(Edges edge_list);

    static const bool VERTEX_BASED = true;
    static const bool EDGE_BASED = false;
    static const bool INCLUDE_ANTI_EDGES = true;
    static const bool EXCLUDE_ANTI_EDGES = false;
    static const bool OVERWRITE_ANTI_EDGES = true;
    static const bool MAINTAIN_ANTI_EDGES = false;
  };
  template <class Inspector>
  bool inspect(Inspector& f, PatternGenerator& x) {
          return f.object(x).fields(f.field("VERTEX_BASED", x.VERTEX_BASED),
                              f.field("EDGE_BASED", x.EDGE_BASED),
                              f.field("INCLUDE_ANTI_EDGES", x.INCLUDE_ANTI_EDGES),
                              f.field("EXCLUDE_ANTI_EDGES", x.EXCLUDE_ANTI_EDGES),
                              f.field("OVERWRITE_ANTI_EDGES", x.OVERWRITE_ANTI_EDGES),
                              f.field("MAINTAIN_ANTI_EDGES", x.MAINTAIN_ANTI_EDGES)
                            );
}


      }
#endif
