#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

using namespace std;

class node{
    public:
        string id;              //unique name for each node
        bool gate;              //true if gate, false if wire
        string type;            //type of gate (and, or, nand, dff, etc) or wire (input, output)
        vector<string> fanins;  //vector of fanin ids
        vector<string> fanouts; //vector of fanout ids

        node();
        node(string i, bool g, string t);
        node(string i, bool g, string t, vector<string> fi, vector<string> fo);

        node parse_line(string l);
        void write_to_bench(string f);
        void add_fanin(string f){fanins.push_back(f);};
        void add_fanout(string f){fanouts.push_back(f);};
        void remove_fanin(string f);
        void remove_fanout(string f);
        void to_string();
        
};

class circuit{
    public:
        vector<node> nodes; //vector of nodes

        circuit();

        void populate_fanouts();
        node * find_node_id(string i);
        void add_node(node n){nodes.push_back(n);};
        void remove_node(node n);
        void write_to_bench(string f);
        void to_string();
        void sort_circuit();
};


#endif