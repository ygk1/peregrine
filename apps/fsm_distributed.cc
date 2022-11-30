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
#include "caf/openssl/all.hpp"
#include "Peregrine.hh"

#include "Domain.hh"



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
std::vector<std::pair<SmallGraph,uint64_t>> pattern_count;
std::vector<SmallGraph> global_server_pattern;
std::vector<std::pair<SmallGraph,uint64_t>> pattern_supports;
std::vector<std::pair<SmallGraph, uint64_t>> patterns_compile;
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
    [=](match_atom, std::string data_graph, std::vector<std::pair<SmallGraph,uint64_t>> patterns, uint32_t step, bool edge_strategy,uint32_t nworkers, uint32_t nprocesses, uint32_t start_task){
          const auto view = [](auto &&v) { return v.get_support(); };
          std::vector<Peregrine::SmallGraph> freq_patterns;
          Peregrine::DataGraph dg(data_graph);
          //const auto process = [](auto &&a, auto &&cm) { a.map(cm.pattern, 1); };
          if(step==0){
            global_server_pattern.clear();
            pattern_supports.clear();
            const auto process = [](auto &&a, auto &&cm) {
            uint32_t merge = cm.pattern[0] == cm.pattern[1] ? 0 : 1;
            a.map(cm.pattern, std::make_pair(cm.mapping, merge));
              };
            global_server_pattern = {Peregrine::PatternGenerator::all(2, !edge_strategy, true)};
            global_server_pattern.front().set_labelling(Peregrine::Graph::DISCOVER_LABELS);
            std::vector<std::pair<SmallGraph,uint64_t>> psupp{};
            if(start_task!=0)
              return psupp;
            std::vector<std::pair<SmallGraph, uint64_t>> psupps = match<Pattern, DiscoveryDomain<1>, AT_THE_END, UNSTOPPABLE>(dg, global_server_pattern, process,nworkers, 1,0, view);
            std::vector<uint64_t> result;
         
            for (const auto &[p, v] : psupps)
              {
                std::cout << p << ": " << (int64_t)v << std::endl;
                result.emplace_back(v);
                pattern_supports.emplace_back(std::pair(p,v));
               
              
              }
            
            result.emplace_back(result.size());
          
           return pattern_supports;
          }
          else{
            
            const auto process = [](auto &&a, auto &&cm) {
                    a.map(cm.pattern, cm.mapping);
                  };
            freq_patterns.clear();
            for(int i=0; i<patterns.size(); i++){
              freq_patterns.push_back(patterns[i].first);
            }
            pattern_supports.clear();
            global_server_pattern =  Peregrine::PatternGenerator::extend(freq_patterns, edge_strategy);

            std::vector<SmallGraph> working_patterns;
            for (int i=start_task; i<global_server_pattern.size(); i+=nprocesses)
              working_patterns.push_back(global_server_pattern[i]);
            std::vector<std::pair<SmallGraph, uint64_t>> psupps = match<Pattern, Domain, AT_THE_END, UNSTOPPABLE>(dg, working_patterns, process, nworkers, 1,0, view);
            std::vector<uint64_t> result;
           
            for (const auto &[p, v] : psupps)
              {
                std::cout << p << ": " << (int64_t)v << std::endl;
                result.emplace_back(v);
                pattern_supports.emplace_back(std::pair(p,v));
               
              }
            result.emplace_back(result.size());
           return pattern_supports;
            }
            //return  std::vector<uint64_t>(0);
            },
    [=](match_atom_str, std::string data_graph_name, std::vector<std::pair<SmallGraph,uint64_t>> patterns, uint32_t step, bool edge_strategy,uint32_t nworkers, uint32_t nprocesses, uint32_t start_task){ 
           const auto view = [](auto &&v) { return v.get_support(); };
          std::vector<Peregrine::SmallGraph> freq_patterns;
          Peregrine::DataGraph dg(data_graph_name);
          //const auto process = [](auto &&a, auto &&cm) { a.map(cm.pattern, 1); };
          if(step==0){
            global_server_pattern.clear();
            pattern_supports.clear();
            const auto process = [](auto &&a, auto &&cm) {
            uint32_t merge = cm.pattern[0] == cm.pattern[1] ? 0 : 1;
            a.map(cm.pattern, std::make_pair(cm.mapping, merge));
              };
            global_server_pattern = {Peregrine::PatternGenerator::all(2, !edge_strategy, true)};
            global_server_pattern.front().set_labelling(Peregrine::Graph::DISCOVER_LABELS);
            std::vector<std::pair<SmallGraph,uint64_t>> psupp{};
            if(start_task!=0)
              return psupp;
            std::vector<std::pair<SmallGraph, uint64_t>> psupps = match<Pattern, DiscoveryDomain<1>, AT_THE_END, UNSTOPPABLE>(dg, global_server_pattern, process,nworkers, 1,0, view);
            std::vector<uint64_t> result;
         
            for (const auto &[p, v] : psupps)
              {
                std::cout << p << ": " << (int64_t)v << std::endl;
                result.emplace_back(v);
                pattern_supports.emplace_back(std::pair(p,v));
               
              
              }
            
            result.emplace_back(result.size());
          
           return pattern_supports;
          }
          else{
            
            const auto process = [](auto &&a, auto &&cm) {
                    a.map(cm.pattern, cm.mapping);
                  };
            freq_patterns.clear();
            for(int i=0; i<patterns.size(); i++){
              freq_patterns.push_back(patterns[i].first);
            }
            pattern_supports.clear();
            global_server_pattern =  Peregrine::PatternGenerator::extend(freq_patterns, edge_strategy);

            std::vector<SmallGraph> working_patterns;
            for (int i=start_task; i<global_server_pattern.size(); i+=nprocesses)
              working_patterns.push_back(global_server_pattern[i]);
            std::vector<std::pair<SmallGraph, uint64_t>> psupps = match<Pattern, Domain, AT_THE_END, UNSTOPPABLE>(dg, working_patterns, process, nworkers, 1,0, view);
            std::vector<uint64_t> result;
           
            for (const auto &[p, v] : psupps)
              {
                std::cout << p << ": " << (int64_t)v << std::endl;
                result.emplace_back(v);
                pattern_supports.emplace_back(std::pair(p,v));
               
              }
            result.emplace_back(result.size());
           return pattern_supports;
            }
            //return  std::vector<uint64_t>(0);
            },
  };
}
struct task {
  std::variant<match_atom, match_atom_str> op;
  std::string data_graph; 
  std::vector<std::pair<SmallGraph,uint64_t>> patterns;
  uint32_t step;
  bool edge_strategy;
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
  size_t pattern_support  = 300;
  size_t number_of_fsm = 3;
  size_t nthreads = 1;
  size_t nNodes = 1;
  uint16_t port = 4242;
  bool client_mode = false;
  bool edge_strategy = false;
  std::string host="localhost";

  config() {
    opt_group{custom_options_, "global"}
      .add(data_graph_name, "data_graph_name,d", "set data_graph_name")
      .add(number_of_fsm, "number_of_fsm,k", "set number_of_fsm")
      .add(pattern_support, "pattern_support,u", "set pattern_support")
      .add(edge_strategy, "edge_strategy,e", "set edge_strategy")
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
    [=](match_atom op, std::string data_graph,  std::vector<std::pair<SmallGraph,uint64_t>> patterns, uint32_t step, bool edge_strategy,uint32_t nworkers, uint32_t nprocesses, uint32_t start_task){
        //self->state.tasks.emplace_back(task{op, data_graph, patterns, nworkers, nprocesses, start_task});
    },
    [=](match_atom_str op, std::string data_graph, std::vector<std::pair<SmallGraph,uint64_t>> patterns, uint32_t step, bool edge_strategy,uint32_t nworkers, uint32_t nprocesses, uint32_t start_task){
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

  auto send_task = [=](auto op,std::string data_graph, std::vector<std::pair<SmallGraph,uint64_t>> patterns, uint32_t step, bool edge_strategy,uint32_t nthreads, uint32_t nNodes, uint32_t start_task){    
    self->request(server,infinite, op, data_graph, patterns, step, edge_strategy,nthreads, nNodes, start_task)
      .then(
        
        [=](std::vector<std::pair<SmallGraph,uint64_t>> res){  
          
          
          uint64_t size = res.size();
          if(done_server==0){
            pattern_count.clear();
            patterns_compile.clear();
            
          }
          int i=0;
          for (const auto &[p,v] : res)
            {
              patterns_compile.emplace_back(std::pair(p,v)); 
              pattern_count.emplace_back(std::pair(p,v));
              i++;
              std::cout << p << ": " << (int64_t)v << std::endl;
            }
           done_server++;
           std::cout<<"Got reply from server "<<done_server<<" Step "<<pattern_count.size()<<std::endl;
        },
        
         
        [=](const error& err){
          // simply try again by enqueueing the task to the mailbox again
          std::cout<<"Didn't get reply from server: "<< to_string(err)<<std::endl;
          self->send(self, op, data_graph, patterns, nthreads, nNodes);
         
        });
  };
  for (auto& x : self->state.tasks) {
    
    auto f = [&](auto op) { send_task(op, x.data_graph, x.patterns,x.step, x.edge_strategy, x.nworkers, x.nprocesses, x.start_task); };
    std::visit(f, x.op);
  }
  self->state.tasks.clear();
  return {
    [=](match_atom op, std::string data_graph, std::vector<std::pair<SmallGraph,uint64_t>> patterns, uint32_t step, bool edge_strategy,uint32_t nthreads, uint32_t nNodes, uint32_t start_task) { send_task(op, data_graph, patterns, step, edge_strategy,nthreads, nNodes, start_task); },
    [=](match_atom_str op, std::string data_graph, std::vector<std::pair<SmallGraph,uint64_t>> patterns, uint32_t step, bool edge_strategy,uint32_t nthreads, uint32_t nNodes, uint32_t start_task) {  send_task(op, data_graph, patterns,  step, edge_strategy, nthreads, nNodes, start_task); },
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
  uint32_t pattern_support = (cfg.pattern_support);
  uint32_t number_of_fsm = (cfg.number_of_fsm);
  uint32_t nthreads = (cfg.nthreads);
  uint32_t nNodes = (cfg.nNodes);
  uint16_t port = cfg.port;
  bool edge_strategy = (cfg.edge_strategy)? Peregrine::PatternGenerator::EDGE_BASED: Peregrine::PatternGenerator::VERTEX_BASED;
  std::string host(cfg.host);
  number_of_server = nNodes;
  
  std::vector<std::pair<SmallGraph,uint64_t>> freq_patterns;
  std::vector<uint64_t> freq_counts;
  std::vector<std::string> hosts;
  std::vector<uint16_t>ports;
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
  auto a2=system.spawn(count_act);
  scoped_actor self{system};
  // auto a3=system.spawn(count_act);
  // auto a2=system.spawn(taskmapping_actor, a3);
  //patterns.front().set_labelling(Peregrine::Graph::DISCOVER_LABELS);
  std::vector<std::pair<std::string, std::string>> host_port;
  //pattern_count.emplace_back(0);
  uint32_t step=0;
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
          //freq_counts.emplace_back(0);
          //patterns.front().set_labelling(Peregrine::Graph::DISCOVER_LABELS);
          while (step < number_of_fsm)
          {
            
            for(int i=0; i<hosts.size(); i++)
            //for(const auto &[h, p] : host_port)
            {
              
            
                
            anon_send(a1, connect_atom_v, hosts[i],
                      ports[i]);
            
            if (is_directory(data_graph_name))
                {
                  std::cout<<"number of server = "<<number_of_server<<std::endl;
                  anon_send(a1, match_atom_str_v,data_graph_name,freq_patterns, step,edge_strategy,nthreads,nNodes,(uint32_t)i);
                  //anon_send(a2, match_atom_str_v,data_graph_name,patterns,nthreads,nNodes);
                }
                else
                {
                  //SmallGraph G(data_graph_name);
                  std::cout<<"number of server = "<<number_of_server<<std::endl;
                  anon_send(a1, match_atom_v, data_graph_name ,freq_patterns, step,edge_strategy,nthreads, nNodes,(uint32_t)i);
                  //anon_send(a2, match_atom_str_v,data_graph_name,patterns,nthreads,nNodes);   
                }
            }
            while(done_server!=nNodes){
              //std::cout<<"Waiting "<<step<<std::endl;
              usleep(1000);
              
            }
            done_server = 0;
            step += 1;
            //freq_counts.clear();
            freq_patterns.clear();
            for(int i=0; i<pattern_count.size(); i++){
              if(pattern_count[i].second>pattern_support)
                freq_patterns.push_back(pattern_count[i]);
                //freq_patterns.push_back(patterns[i]);
            }
            std::cout<<"Going to loop again "<<step<<std::endl;
            pattern_count.clear();
            
            //patterns = Peregrine::PatternGenerator::extend(freq_patterns, edge_strategy);
            
          }
            t2 = utils::get_timestamp();
            done=true;
      }
      
      else if (cmd == "quit"){
        done=true;
      }
      
      
    },
    [&](std::string& arg0, std::string& arg1, std::string& arg2) {
    
      if (arg0 == "connect") {
        host_port.emplace_back(arg1, arg2);
        //a1=system.spawn(init);
        char* end=nullptr;
        hosts.emplace_back(arg1);
        ports.emplace_back(strtoul(arg2.c_str(), &end, 10));

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
  std::cout<<"Time taken = "<< time_taken/1e6<<"s"<<std::endl;
  for (int i=0; i<freq_patterns.size(); i++)
     std::cout<<freq_patterns[i].first<<" : "<< (int64_t)freq_patterns[i].second<< std::endl;
  anon_send_exit(a1, exit_reason::user_shutdown);
  //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  
  
  
  
  //anon_send(a1, connect_atom_v, host, port);
  
  
 
  //anon_send_exit(a1, exit_reason::user_shutdown);
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

// self->request(a2,infinite, match_atom_str_v, data_graph_name, freq_counts, step, edge_strategy,nthreads, nNodes, (uint32_t)i)
//             .receive(
//             [=](std::vector<std::pair<SmallGraph, uint64_t>> res){  
              
              
//               uint64_t size = res.size();
//               if(done_server==0){
//                 pattern_count.clear();
//                 for(int i=0; i<size; i++){
                  
//                   pattern_count.push_back(0);
//                 }
//               }
//               int i=0;
//               for (const auto &[p,v] : res)
//                 {
                  
//                   pattern_count[i]+=v;
//                   i++;
//                 }
//               done_server++;
//               std::cout<<"Got reply from local machine "<<done_server<<" Step "<<pattern_count.size()<<std::endl;
//             },
//             [=](const error& err){
//           // simply try again by enqueueing the task to the mailbox again
//               std::cout<<"Didn't get reply from server: "<< to_string(err)<<std::endl;
//               //self->send(self, op, data_graph, patterns, nthreads, nNodes);
            
//             });