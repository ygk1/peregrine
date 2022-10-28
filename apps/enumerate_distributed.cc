#include "caf/config.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <array>
#include <cassert>
#include <functional>
#include <iostream>
#include <sstream>
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
auto t1 = utils::get_timestamp();
auto t2 = utils::get_timestamp();
double time_taken = 0.0;
uint32_t number_of_server = 0;
uint32_t done_server = 0;
std::vector<uint64_t> pattern_count;
bool is_directory(const std::string &path)
{
   struct stat statbuf;
   if (stat(path.c_str(), &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}
behavior count_act(event_based_actor* self){
  //std::cout <<"spawned" <<std::endl;
  self->set_default_handler(print_and_drop);
  return{
    [=](match_atom, std::string data_graph, std::string pattern_name, uint32_t nworkers, uint32_t nprocesses, uint32_t start_task){
      std::cout<<"Counting\n"; SmallGraph G(data_graph); 
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

      const auto process = [](auto &&a, auto &&cm) { a.map(cm.pattern, 1); };
      std::vector<uint64_t> counts;
      std::vector<std::pair<SmallGraph, uint64_t>> res = match<Pattern, uint64_t, AT_THE_END, UNSTOPPABLE>(G,patterns, nworkers, process,nprocesses, start_task);
      std::cout << "Result size = " <<sizeof(res)<< std::endl;
      for (const auto &[p, v] : res)
      {
        std::cout << p << ": " << (int64_t)v << std::endl;
        counts.emplace_back(v);
      }
      std::string a("Done!\n");
      std::cout<<"Size of message to be sent "<< a.size()<<std::endl;
      return counts;
            },
    [=](match_atom_str, std::string data_graph_name, std::string pattern_name, uint32_t nworkers, uint32_t nprocesses, uint32_t start_task){ 
      std::cout<<"Counting 2\n"; 
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

      const auto process = [](auto &&a, auto &&cm) { a.map(cm.pattern, 1); };
      std::vector<uint64_t> counts;
      std::vector<std::pair<SmallGraph, uint64_t>> res =  match<Pattern, uint64_t, AT_THE_END, UNSTOPPABLE>(data_graph_name,patterns, nworkers, process,nprocesses, start_task);
      std::cout << "Result size = " <<sizeof(res)<< std::endl;
      for (const auto &[p, v] : res)
      {
        std::cout << p << ": " << (int64_t)v << std::endl;
        counts.emplace_back(v);
      }
     std::string a("Done!\n");
      std::cout<<"Size of message to be sent "<< a.size()<<std::endl;
      return counts;
            },
  };
}

struct task {
  std::variant<match_atom, match_atom_str> op;
  std::string data_graph; 
  std::string patterns; 
  uint32_t nworkers;
  uint32_t nprocesses;
  uint32_t start_task;
};
struct state {
  strong_actor_ptr current_server;
  std::vector<task> tasks;
};

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
void connecting(stateful_actor<state>* self,  const std::string& host, uint16_t port);
behavior unconnected(stateful_actor<state>* self);
behavior taskmapping_actor(stateful_actor<state>* self, const actor& server);
void count_client(actor_system& system, const config& cfg);

behavior init(stateful_actor<state>* self) {
  // transition to `unconnected` on server failure
  self->set_down_handler([=](const down_msg& dm) {
    if (dm.source == self->state.current_server) {
      aout(self) << "*** lost connection to server" << std::endl;
      self->state.current_server = nullptr;
      self->become(unconnected(self));
    }
  });
  return unconnected(self);
}
behavior unconnected(stateful_actor<state>* self) {
  return {
    [=](match_atom op, std::string data_graph,  std::string patterns, uint32_t nworkers, uint32_t nprocesses, uint32_t start_task){
        //self->state.tasks.emplace_back(task{op, data_graph, patterns, nworkers, nprocesses, start_task});
    },
    [=](match_atom_str op, std::string data_graph, std::string patterns, uint32_t nworkers, uint32_t nprocesses, uint32_t start_task){
        //self->state.tasks.emplace_back(task{op, data_graph, patterns, nworkers, nprocesses, start_task});
    },
    [=](connect_atom, const std::string& host, uint16_t port) {
      connecting(self, host, port);
    },
  };
}
void connecting(stateful_actor<state>* self,  const std::string& host, uint16_t port) {

  
  self->state.current_server = nullptr;
  std::vector<std::pair<SmallGraph, uint64_t>> result;
  //auto a1;
  
  // use request().await() to suspend regular behavior until MM responded
  auto mm = self->system().middleman().actor_handle();
  self->request(mm, infinite, connect_atom_v, host, port)
    .await(
      [=](const node_id&, strong_actor_ptr serv,
          const std::set<std::string>& ifs) {
        if (!serv) {
          aout(self) << R"(*** no server found at ")" << host << R"(":)" << port
                     << std::endl;
          return;
        }
        if (!ifs.empty()) {
          aout(self) << R"(*** typed actor found at ")" << host << R"(":)"
                     << port << ", but expected an untyped actor " << std::endl;
          return;
        }
        aout(self) << "*** successfully connected to server" << std::endl;
        number_of_server++;
        //self->state.current_server = serv;
        self->state.current_server = serv;
        auto hdl = actor_cast<actor>(serv);
        self->monitor(hdl);
        self->become(taskmapping_actor(self,hdl));
      },
      [=](const error& err) {
        aout(self) << R"(*** cannot connect to ")" << host << R"(":)" << port
                   << " => " << to_string(err) << std::endl;
        self->become(unconnected(self));
      });
}
behavior taskmapping_actor(stateful_actor<state>* self, const actor& server){

  auto send_task = [=](auto op,std::string data_graph, std::string patterns, uint32_t nthreads, uint32_t nNodes, uint32_t start_task){    
    self->request(server,infinite, op, data_graph, patterns, nthreads, nNodes, start_task)
      .then(
        // [=](std::vector<std::pair<SmallGraph, uint64_t>> res){
        //   for (const auto &[p, v] : res)
        //     {
        //       std::cout << p << ": " << v << std::endl;
        //     }
        // },
        [=](std::vector<uint64_t> res){
          int i=0;
          for (const auto &v : res)
            {
              //std::cout << v << std::endl;
              pattern_count[i]+=v;
              i++;
            }
          std::cout<<"Results from server received from server "<<done_server<<std::endl <<std::flush;
          done_server++;
          if(done_server==number_of_server)
            t2 = utils::get_timestamp(); 
        },
       
        [=](const error& err){
          // simply try again by enqueueing the task to the mailbox again
          std::cout<<"Didn't get reply from server: "<< to_string(err)<<std::endl;
          self->send(self, op, data_graph, patterns, nthreads, nNodes);
         
        });
  };
  for (auto& x : self->state.tasks) {
    
    auto f = [&](auto op) { send_task(op, x.data_graph, x.patterns, x.nworkers, x.nprocesses, x.start_task); };
    std::visit(f, x.op);
  }
  self->state.tasks.clear();
  return {
    [=](match_atom op, std::string data_graph, std::string patterns, uint32_t nthreads, uint32_t nNodes, uint32_t start_task) { send_task(op, data_graph, patterns, nthreads, nNodes, start_task); },
    [=](match_atom_str op, std::string data_graph, std::string patterns, uint32_t nthreads, uint32_t nNodes, uint32_t start_task) {  send_task(op, data_graph, patterns, nthreads, nNodes, start_task); },
    [=](connect_atom, const std::string& host, uint16_t port) {
      connecting(self, host, port);
    },
  };
    //result = Peregrine::count(data_graph_name, patterns, nthreads);
  }
void count_client(actor_system& system, const config& cfg) {
  // send "Hello World!" to our buddy ...
  //init_global_meta_objects<id_block::Peregrine>();
  
  
  std::string data_graph_name(cfg.data_graph_name);
  std::string pattern_name(cfg.pattern_name);
  uint32_t nthreads = (cfg.nthreads);
  uint32_t nNodes = (cfg.nNodes);
  uint16_t port = cfg.port;
  std::string host(cfg.host);
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
  for (int i=0; i<patterns.size(); i++)
    pattern_count.emplace_back(0);
  std::cout<<data_graph_name<<" "<<pattern_name<<" "<<nthreads<<" "<<nNodes<<std::endl;
  
  auto usage = [] {
    std::cout << "Usage:" << std::endl
         << "  quit                  : terminates the program" << std::endl
         << "  connect <host> <port> : connects to a remote actor" << std::endl
         << "  start work now            : to start work" << std::endl
         << std::endl;
  };
  usage();
  bool done = false;
  auto a1=system.spawn(init);
  
  // auto a3=system.spawn(count_act);
  // auto a2=system.spawn(taskmapping_actor, a3);
  std::vector<std::pair<std::string, std::string>> host_port;
  //anon_send(a1, connect_atom_v, cfg.host, cfg.port);
  if (!cfg.host.empty() && cfg.port > 0);
    //anon_send(a1, connect_atom_v, cfg.host, cfg.port);
  else
    std::cout << "*** no server received via config, "
         << R"(please use "connect <host> <port>" before using the calculator)"
         << std::endl;
  message_handler eval{
    [&](const std::string& cmd) {
      if (cmd == "start"){
          t1 = utils::get_timestamp();
          for(const auto &[h, p] : host_port){
            char* end = nullptr;
            auto lport = strtoul(p.c_str(), &end, 10);
            if (end != p.c_str() + p.size())
              std::cout << R"(")" << p << R"(" is not an unsigned integer)" << std::endl;
            else if (lport > std::numeric_limits<uint16_t>::max())
              std::cout << R"(")" << p << R"(" > )"
                  << std::numeric_limits<uint16_t>::max() << std::endl;
            else{
              
              anon_send(a1, connect_atom_v, move(h),
                        static_cast<uint16_t>(lport));
              number_of_server++;                
              if (is_directory(data_graph_name))
              {
                std::cout<<"number of server = "<<number_of_server<<std::endl;
                anon_send(a1, match_atom_str_v,data_graph_name,pattern_name,nthreads,nNodes,(number_of_server-1));
                //anon_send(a2, match_atom_str_v,data_graph_name,patterns,nthreads,nNodes);
              }
              else
              {
                //SmallGraph G(data_graph_name);
                std::cout<<"number of server = "<<number_of_server<<std::endl;
                anon_send(a1, match_atom_v, data_graph_name , pattern_name, nthreads, nNodes,(number_of_server-1));
                //anon_send(a2, match_atom_str_v,data_graph_name,patterns,nthreads,nNodes);   
              }
              
               
                
              }
            
          }            
            
      }
      else if (cmd == "quit"){
        done=true;
      }
      
      
    },
    [&](std::string& arg0, std::string& arg1, std::string& arg2) {
    
      if (arg0 == "connect") {
        host_port.emplace_back(arg1, arg2);
        //a1=system.spawn(init);
        

      }
      }
    };
    std::string line;
    while (!done && (done_server!=nNodes)) {
      std::getline(std::cin, line);
      line = trim(std::move(line)); // ignore leading and trailing whitespaces
      std::vector<std::string> words;
      split(words, line, is_any_of(" "), token_compress_on);
      auto msg = message_builder(words.begin(), words.end()).move_to_message();
      if (!eval(msg))
        usage();
  }
    //while(done_server!=nNodes);
    
  //anon_send(a1, connect_atom_v, host, port);
  
  time_taken += (t2-t1);
  std::cout<<"End client\n";
  std::cout <<"Time taken = "<< time_taken/1e6<<"s"<<std::endl;
  anon_send_exit(a1, exit_reason::user_shutdown);
  //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  for (int i=0; i<patterns.size(); i++)
    std::cout << patterns[i] << ": " << (int64_t)pattern_count[i] << std::endl;

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
  //count_client(system, cfg);
  std::getline(std::cin, dummy);
  std::cout << "... cya" << std::endl;
  io::unpublish(counter, cfg.port);
  anon_send_exit(counter, exit_reason::user_shutdown);

}
void caf_main(actor_system& system, const config& cfg)
{
  auto f = cfg.client_mode ? count_client : count_server;
  f(system, cfg);
  //caf.middleman.inbound-messages-size = 1024;
  //count_client(system, cfg);
  
}
} //namespace

CAF_MAIN(id_block::Peregrine, io::middleman)