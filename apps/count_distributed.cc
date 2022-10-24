#include "caf/config.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>


// #include "caf/actor_ostream.hpp"
// #include "caf/actor_system.hpp"
// #include "caf/caf_main.hpp"
// #include "caf/event_based_actor.hpp"
#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "Peregrine.hh"



using namespace caf;


using namespace Peregrine;
//dynamically defined counter actor

// template <class Inspector>
// bool inspect(Inspector& f, Peregrine::SmallGraph& x) {
//   return f.apply(x);
// }
//using namespace Peregrine;


namespace{
bool is_directory(const std::string &path)
{
   struct stat statbuf;
   if (stat(path.c_str(), &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

class config : public actor_system_config {
public:
  std::string data_graph_name = "data/citeseer";
  std::string pattern_name = "3-motifs";
  size_t nthreads = 1;
  size_t nNodes = 1;
  uint16_t port = 4242;
  bool client_mode = false;
  std::string host="localhost";

  config() {
    opt_group{custom_options_, "global"}
      .add(data_graph_name, "data_graph_name,d", "set data_graph_name")
      .add(pattern_name, "pattern_name,p", "set pattern_name")
      .add(nthreads, "nthreads,t", "set nthreads")
      .add(nNodes, "nNodes,n", "set nNodes")
      .add(port, "port,s", "set port")
      .add(client_mode, "client-mode,c", "enable client mode");
  }
};


behavior count_act(event_based_actor* self){
  std::cout <<"spawned" <<std::endl;
  self->set_default_handler(print_and_drop);
  return{
    [=](count_atom, SmallGraph data_graph, const std::vector<SmallGraph> patterns, uint32_t nworkers, uint32_t nprocesses){ std::cout<<"Counting\n"; return count(data_graph,patterns, nworkers, nprocesses);},
    [=](count_atom_str, std::string data_graph_name, const std::vector<SmallGraph> patterns, uint32_t nworkers, uint32_t nprocesses){ std::cout<<"Counting 2\n"; return count(data_graph_name,patterns, nworkers, nprocesses);}
  };
}
void count_server(actor_system& system, const config& cfg){

  
  auto counter=system.spawn(count_act);

  std::cout << "*** try publish at port " << cfg.port << std::endl;
  auto expected_port = io::publish(counter, cfg.port);
  if (!expected_port) {
    std::cerr << "*** publish failed: " << to_string(expected_port.error())
              << std::endl;
    return;
  }
  std::cout << "*** server successfully published at port " << *expected_port << std::endl
       << "*** press [enter] to quit" << std::endl;
  std::string dummy;
  std::getline(std::cin, dummy);
  std::cout << "... cya" << std::endl;
  anon_send_exit(counter, exit_reason::user_shutdown);

}
void taskmapping_actor(event_based_actor* self, const actor& server, std::string data_graph_name, const std::vector<SmallGraph> patterns, uint32_t nthreads, uint32_t nNodes){
  if (is_directory(data_graph_name))
  {
    //self->send(counter,count_atom_str_v, data_graph_name, patterns, nthreads, nNodes);
    //anon_send(counter, count_atom_str_v, data_graph_name, patterns, nthreads, nNodes);
    self->request(server,std::chrono::seconds(10), count_atom_str_v, data_graph_name, patterns, nthreads, nNodes)
      .then(
        [=](std::vector<std::pair<SmallGraph, uint64_t>> res){
          for (const auto &[p, v] : res)
            {
              std::cout << p << ": " << v << std::endl;
            }
        }, 
        [=](const error&) {
          // simply try again by enqueueing the task to the mailbox again
          std::cout<<"Didn't get reply from server"<<std::endl;
        });
    
    //result = Peregrine::count(data_graph_name, patterns, nthreads);
  }
  else
  {
    Peregrine::SmallGraph G(data_graph_name);
    
    //self->send(counter, count_atom_v, G, patterns, nthreads, nNodes);
    //anon_send(counter, count_atom_v, G, patterns, nthreads, nNodes);
    self->request(server,std::chrono::seconds(10), count_atom_v, G, patterns, nthreads, nNodes)
      .then(
        [=](std::vector<std::pair<SmallGraph, uint64_t>> res){
          for (const auto &[p, v] : res)
            {
              std::cout << p << ": " << v << std::endl;
            }
        },
         [=](const error&) {
          // simply try again by enqueueing the task to the mailbox again
          std::cout<<"Didn't get reply from server"<<std::endl;
        }
        );
     
  }
}
void count_client(actor_system& system, const config& cfg) {
  // send "Hello World!" to our buddy ...
  //init_global_meta_objects<id_block::Peregrine>();
  const std::string data_graph_name(cfg.data_graph_name);
  const std::string pattern_name(cfg.pattern_name);
  size_t nthreads = (cfg.nthreads);
  size_t nNodes = (cfg.nNodes);
  std::cout<<data_graph_name<<" "<<pattern_name<<" "<<nthreads<<" "<<nNodes<<std::endl;
  std::vector<Peregrine::SmallGraph> patterns;
  if (auto end = pattern_name.rfind("motifs"); end != std::string::npos)
  {
    auto k = std::stoul(pattern_name.substr(0, end-1));
    patterns = Peregrine::PatternGenerator::all(k,
        Peregrine::PatternGenerator::VERTEX_BASED,
        Peregrine::PatternGenerator::INCLUDE_ANTI_EDGES);
  }
  else if (auto end = pattern_name.rfind("clique"); end != std::string::npos)
  {
    auto k = std::stoul(pattern_name.substr(0, end-1));
    patterns.emplace_back(Peregrine::PatternGenerator::clique(k));
  }
  else
  {
    patterns.emplace_back(pattern_name);
  }

  std::vector<std::pair<SmallGraph, uint64_t>> result;
  //auto a1;
  
  assert(system.has_middleman());
  auto& mm = system.middleman();
  auto maybe_counter = mm.remote_actor(cfg.host, cfg.port);
  auto counter = (*maybe_counter); // Handle (caf::actor) to the published actor.
  if (maybe_counter)
    std::cout <<"successfully connected to server"<<std::endl;
  else
    std::cerr << "Failed to connect: " << to_string(maybe_counter.error()) << '\n';
  auto a1=system.spawn(taskmapping_actor,counter, data_graph_name, patterns, nthreads, nNodes);
  //self->monitor(counter);
  
  std::cout<<"End client\n";
  anon_send_exit(counter, exit_reason::user_shutdown);
}
void caf_main(actor_system& system, const config& cfg)
{
  auto f = cfg.client_mode ? count_client : count_server;
  f(system, cfg);
  
}
} //namespace

CAF_MAIN(id_block::Peregrine, io::middleman)