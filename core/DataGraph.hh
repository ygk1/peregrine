#ifndef BUFFER_HH
#define BUFFER_HH

#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <list>
#include <unordered_set>
#include <stdint.h> // uint32_t

#include "Graph.hh"
#include "caf/actor_ostream.hpp"
#include "caf/actor_system.hpp"
#include "caf/caf_main.hpp"
#include "caf/event_based_actor.hpp"
#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/all.hpp"
using namespace caf;


namespace Peregrine
{

  struct adjlist
  {
    adjlist() : length(0), ptr(nullptr) {}
    adjlist(uint32_t l, uint32_t *p) : length(l), ptr(p) {}
    uint32_t length;
    uint32_t *ptr;
  };


  class DataGraph
  {

   public:
    DataGraph(const std::string &data_graph_path);
    DataGraph(const SmallGraph &p);
    DataGraph(DataGraph &&other);
    DataGraph(DataGraph &) = delete;

    void set_rbi(const AnalyzedPattern &rbi);
    void set_known_labels(const std::vector<SmallGraph> &patterns);
    void set_known_labels(const std::vector<uint32_t> &labels);
    bool known_label(const uint32_t label) const;

    const std::vector<uint32_t> &get_upper_bounds(uint32_t v) const;
    const std::vector<uint32_t> &get_lower_bounds(uint32_t v) const;
    const adjlist &get_adj(uint32_t v) const;
    uint32_t get_vgs_count() const;
    uint32_t get_vertex_count() const;
    uint64_t get_edge_count() const;
    const SmallGraph &get_vgs(unsigned fi) const;
    const SmallGraph &get_pattern() const;
    const std::vector<std::vector<uint32_t>> &get_qs(unsigned fi) const;
    uint32_t vmap_at(unsigned fi, uint32_t v, unsigned qsi) const;
    uint32_t label(uint32_t dv) const;
    const std::vector<uint32_t> &get_qo(uint32_t fi) const;
    uint32_t original_id(uint32_t v) const;
    const std::pair<uint32_t, uint32_t> &get_label_range() const;
    // caf::type_id_t type() const noexcept {
    //   return caf::type_id_v<DataGraph>;
    //   }
    //AnalyzedPattern rbi;
    // uint32_t new_label;
    template <class Inspector>
    friend bool inspect(Inspector& f, DataGraph& x) { 
          return f.object(x).fields(f.field("rbi", x.rbi),
                                  f.field("new_label", x.new_label),
                                  f.field("vertex_count", x.vertex_count),
                                  f.field("edge_count", x.edge_count),
                                  f.field("forest_count", x.forest_count),
                                  f.field("labelled_graph", x.labelled_graph),
                                  f.field("labels", x.labels),
                                  f.field("label_range", x.label_range),
                                  f.field("ids", x.ids),
                                  f.field("data_graph", x.data_graph),
                                  f.field("graph_in_memory", x.graph_in_memory),
                                  f.field("known_labels", x.known_labels)
          );
    }
    AnalyzedPattern rbi;
    uint32_t new_label;
   private:
    void from_smallgraph(const SmallGraph &p);

    uint32_t vertex_count;
    uint64_t edge_count;
    unsigned forest_count;
    bool labelled_graph = false;
    std::unique_ptr<uint32_t[]> labels;
    std::pair<uint32_t, uint32_t> label_range;
    std::unique_ptr<uint32_t[]> ids;
    std::unique_ptr<adjlist[]> data_graph;
    std::unique_ptr<uint32_t[]> graph_in_memory;
    std::unordered_set<uint32_t> known_labels;
  };

  }
  

#endif
