<img src="https://user-images.githubusercontent.com/47194770/78506209-e73ca880-772c-11ea-9fc1-790713527bc9.png" alt="Peregrine" width="70%">

# Peregrine: A Pattern-Aware Graph Mining System

![tests](https://github.com/pdclab/peregrine/workflows/tests/badge.svg)

Peregrine is an efficient, single-machine system for performing data mining tasks on large graphs. Some graph mining applications include:
* Finding frequent subgraphs
* Generating the motif/graphlet distribution
* Finding all occurrences of a subgraph

Peregrine is highly programmable, so you can easily develop your own graph mining applications using its novel, declarative, graph-pattern-centric API.
To write a Peregrine program, you describe which graph patterns you are interested in mining, and what to do with each occurrence of those patterns. You provide the _what_ and the runtime handles the _how_.

For full details, you can read our paper published in [EuroSys 2020](https://dl.acm.org/doi/abs/10.1145/3342195.3387548) or the longer version on [arXiv](https://arxiv.org/abs/2004.02369). For a deeper look at optimizations under the hood, check out our article in the [2021 ACM Operating Systems Review](https://dl.acm.org/doi/abs/10.1145/3469379.3469381).

For an in-depth summary, watch the video presentation:

[![EuroSys 2020 Presentation Thumbnail](https://user-images.githubusercontent.com/9058564/87882006-f0871380-c9b1-11ea-9ba0-d632a96b3a42.png)](http://www.youtube.com/watch?v=o3BaYgeR0nQ "Peregrine: A Pattern-Aware Graph Mining System")

**TL;DR:** compared to other state-of-the-art open-source graph mining systems, Peregrine:
* executes up to 700x faster
* consumes up to 100x less memory
* scales to 100x larger data sets
* on 8x fewer machines
* with a simpler, more expressive API

## Table of Contents
1. [Quick start](#1-quick-start)
2. [Writing your own programs](#2-writing-your-own-programs)
3. [Data graphs](#3-data-graphs)
4. [Reproducing our EuroSys 2020 paper results](#4-reproducing-our-eurosys-2020-paper-results)
5. [Contributing](#5-contributing)
6. [Acknowledgements](#6-acknowledgements)
7. [Resources](#7-resources)

## 1. Quick start

Peregrine has been tested on Ubuntu 18.04 and Arch Linux but should work on any
POSIX-y OS. It requires C++20 support (GCC version >= 10.1). Additionally, the
tests require [UnitTest++](https://github.com/unittest-cpp/unittest-cpp).

Ubuntu 18.04 prerequisites:
```
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt install g++-10 libunittest++-dev
```
CAF prerequisite can be installed by following the steps in the documentations [CAF](https://github.com/actor-framework/actor-framework)

To build Peregrine:

```
$ git clone https://github.com/pdclab/Peregrine.git
$ cd Peregrine
$ source tbb2020/bin/tbbvars.sh intel64
$ make -j CC=g++-10
$ bin/test
```

Several sample applications, query patterns, and a sample dataset are released with the code. Calling any of the applications without arguments will show you what they expect:

```
$ bin/count
USAGE: bin/count <data graph> <pattern | #-motifs | #-clique> [# threads]
```

These applications print their results in `<pattern>: <aggregation value>` format.

For example, motif-counting:

```
$ bin/count data/citeseer 3-motifs 8
Counting 3-motifs
Finished reading datagraph: |V| = 3264 |E| = 4536
[...]
All patterns finished after 0.030265s
[2-3][1-3]: 23380
[1-2][1-3][2-3]: 1166
```

The string `[2-3][1-3]` encodes the pattern consisting of edges `(1, 3), (2, 3)`, and 23380 is the number of unique occurrences Peregrine found in the citeseer graph.

Other applications give similar output:

```
$ bin/count data/citeseer 4-clique 8
[...]
All patterns finished after 0.030265s
[3-4][1-2][1-3][1-4][2-3][2-4]: 255
$
$ bin/count data/citeseer query/p1.graph 8
[...]
All patterns finished after 0.003368s
[3-4][1-2][1-3][1-4][2-3]: 3730
```

FSM provides support values instead of counts:

```
$ bin/fsm data/citeseer 3 300 8 # size 3 FSM with support 300
[...]
Frequent patterns:
[1,0-2,0][1,0-3,0][2,0-4,0]: 303
[1,1-2,1][1,1-3,1][2,1-4,1]: 335
Finished in 0.078629s
```

The sample FSM app performs edge-induced FSM by default. For vertex-induced FSM, simply call it as

```
$ bin/fsm data/citeseer 3 300 8 v # vertex-induced
```

The existence-query application simply states whether the desired pattern exists or not:

```
$ bin/existence-query data/citeseer 14-clique 8
[...]
All patterns finished after 0.005509s
[pattern omitted due to length] doesn't exist in data/citeseer
```

The output application stores matches grouped by pattern in either CSV or packed binary format:

```
$ bin/output data/citeseer 3-motifs results bin 8
[...]
all patterns finished after 0.002905s
[1-3](1~2)[2-3]: 23380 matches stored in "results/[1-3](1~2)[2-3]"
[1-2][1-3][2-3]: 1166 matches stored in "results/[1-2][1-3][2-3]"
```

In the CSV format, for a pattern with `n` vertices each line will contain a match written in the form:

```
v1,v2,...,vn
...
```

In the binary format, matches are written as sequences of `n` 4-byte vertex IDs in binary, with no delimiters:

```
bits(v1)bits(v2)...bits(vn)...
```
### Run distributed Peregrine

#### 1. Copy the data graph to the servers, build your program and start up the server as

```
USAGE: bin/count_distributed -s [port]
USAGE: bin/enumerate_distributed -s [port]
USAGE: bin/existence-query-dist -s [port]
USAGE: bin/fsm_distributed -s [port]
```
For example:
$ bin\count_distributed -s 4242

#### 2. On the client side:

Count Distributed :
```
USAGE: bin/count_dsitributed -d  <data graph> -p <pattern | #-motifs | #-clique> -t [#threads] -n [#Nodes] -c(client mode flag)
```
For example:
```
$ bin/count_dsitributed -d  data/citeseer -p 3-motifs -t 8 -n 2 -c
```
Enumerate Distributed :
```
USAGE: bin/enumerate_dsitributed -d  <data graph> -p <pattern | #-motifs | #-clique> -t [#threads] -n [#Nodes] -c(client mode flag)
```
For example:
```
$ bin/enumerate_dsitributed -d  data/citeseer -p 3-motifs -t 8 -n 2 -c
```
Existence Query Distributed :
```
USAGE: bin/existence-query-dist -d  <data graph> -p <pattern | #-clique> -t [#threads] -n [#Nodes] -c(client mode flag)
```
For example:
```
$ bin/existence-query-dist -d  data/citeseer -p 14-clique -t 8 -n 2 -c
```
FSM Distributed :
```
USAGE: bin/fsm_distributed -d  <data graph> -k [#steps] -u [support threshold] -t [#threads] -n [#Nodes] -e(flag for edge stategy) -c(client mode flag)
```
For example:
```
$ bin/fsm_distributed -d data/citeseer -k 3 -u 300 -t 8 -n 2 -e -c
```

