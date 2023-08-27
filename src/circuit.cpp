/*
Circuit class
Jordan Maynard
2023
*/

#include "circuit.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
using namespace std;

/**
 * @brief Construct a new node::node object with no parameters
 */
node::node(){
    id = "";
    gate = false;
    type = "";
    fanins = {};
    fanouts = {};
}

/**
 * @brief Construct a new node::node object
 * 
 * @param i string id
 * @param g bool is gate
 * @param t string node type
 */
node::node(string i, bool g, string t){
    id = i;
    gate = g;
    type = t;
    fanins = {};
    fanouts = {};
}

/**
 * @brief Construct a new node::node object
 * 
 * @param i string id
 * @param g bool is gate
 * @param t string node type
 * @param fi vector<string> fanins
 * @param fo vector<string> fanouts
 */
node::node(string i, bool g, string t, vector<string> fi, vector<string> fo){
    id = i;
    gate = g;
    type = t;
    fanins = fi;
    fanouts = fo;
}

/**
 * @brief Parse line to create node
 * 
 * @param l string line to parse
 * @return node object of parsed line
 */
node node::parse_line(string l){
    int in, out, comment, openpar, closepar, eq, comma;
    string fanin_str, fanins_parsed;
    in = l.find("INPUT");
    out = l.find("OUTPUT");
    openpar = l.find("(");
    closepar = l.find(")");
    comment = l.find("#");
    if(comment != string::npos || l == ""){
        return node();
    }
    if(in != string::npos || out != string::npos){         //check if line is wire
        gate = false;
        if (openpar == 5){
            type = "INPUT";
        } else {
            type = "OUTPUT";
        }
        id = l.substr(openpar+1,closepar-openpar-1);
    } else {
        gate = true; //node is a gate
        eq = l.find(" = ");
        id = l.substr(0, eq); //get gate id
        type = l.substr(eq+3,openpar-(eq+3)); //get gate type
        fanin_str = l.substr(openpar+1,closepar-openpar-1); //get unparsed fanins
        //cout << fanin_str << endl;
        while ((comma = fanin_str.find(", ")) != string::npos){ //parse fanins by commas
            fanins_parsed = fanin_str.substr(0,comma);
            fanins.push_back(fanins_parsed);
            //cout << fanins_parsed << "--";
            fanin_str.erase(0, comma+2);
        }
        fanins.push_back(fanin_str);
    }
    return node();

}

/**
 * @brief Write node to file in correct format
 * 
 * @param f file to write to
 */
void node::write_to_bench(string f){
    ofstream outfile(f.c_str(), fstream::app);
    if(gate){
        outfile << id << " = " << type << "(";
        for(int i = 0; i<fanins.size(); i++){
            if(i!=0) outfile << ", ";
            outfile << fanins[i];
        }
        outfile << ")" << endl;
    } else {
        outfile << type << "(" << id << ")" << endl;
    }
}

/**
 * @brief remove specified fanin to node
 * 
 * @param f fanin to remove
 */
void node::remove_fanin(string f){
    int pos;
    for(int i = 0; i < fanins.size(); i++){
        if(fanins[i] == f){
            pos = i;
            break;
        }
    }
    fanins.erase(fanins.begin()+pos);
}

/**
 * @brief remove specified fanout to node
 * 
 * @param f fanout to remove
 */
void node::remove_fanout(string f){
    int pos;
    for(int i = 0; i < fanouts.size(); i++){
        if(fanouts[i] == f){
            pos = i;
            break;
        }
    }
    fanouts.erase(fanouts.begin()+pos);
 }

/**
 * @brief Prints all node information
 */
void node::to_string(){
    cout << "ID: " << id << endl;
    cout << "Gate: " << gate << endl;
    cout << "Type: " << type << endl;
    if (gate == 1){
        cout << "Fanins: ";
        for (int i = 0; i < fanins.size(); i++){
            if (i != 0) cout << ", ";
            cout << fanins[i];
        }
        cout << endl;
    }
    cout << "Fanouts: ";
    for (int i = 0; i < fanouts.size(); i++){
        if (i != 0) cout << ", ";
        cout << fanouts[i];
    }
    cout << endl;
}

/**
 * @brief Construct a new circuit::circuit object with no parameters
 */
circuit::circuit(){
    nodes = {};
}

/**
 * @brief Loop through nodes in circuit to populate each node fanout
 */
void circuit::populate_fanouts(){
    node * n;
    node search;
    for (int i = 0; i<nodes.size(); i++){
        n = &nodes[i];
        /* cout << "Current node: ";
        (*n).to_string();
        cout << endl; */
        for (int j = 0; j<nodes.size(); j++){
            search = nodes[j];
            for (int k = 0; k<search.fanins.size(); k++){
                //cout << search.fanins[k] << " == " << (*n).id << "? ";
                if (search.fanins[k] == (*n).id) (*n).add_fanout(search.id);
            }
        }
    }
}

/**
 * @brief find and return a node from the circuit
 * 
 * @param i node id
 * @return node* returns node pointer if found, empty if not found
 */
node * circuit::find_node_id(string i){
    node * search;
    for (int j = 0; j<nodes.size(); j++){
        search = &nodes[j];
        if(((*search).id == i) & (*search).gate){
            return search;
        }
    }
    cout << "Node with id " << i << " not found in circuit." << endl;
    node * empty;
    return empty;
}

/**
 * @brief remove node from circuit
 * 
 * @param n node to remove
 */
void circuit::remove_node(node n){
    for(int i = 0; i<this->nodes.size(); i++){
        if((this->nodes[i].id == n.id) & (this->nodes[i].type == n.type)){
            this->nodes.erase(this->nodes.begin()+i,this->nodes.begin()+i+1);
        }
    }
}

/**
 * @brief write all nodes in circuit to .bench file
 * 
 * @param f file to write to
 */
void circuit::write_to_bench(string f){
    for(int i = 0; i<nodes.size(); i++){
        nodes[i].write_to_bench(f);
    }
}

/**
 * @brief Print every node in the circuit
 */
void circuit::to_string(){
    for(int i = 0; i<nodes.size(); i++){
        nodes[i].to_string(); 
        cout << "~~~~~~~~~~~~~~" << endl;
    }
}

/**
 * @brief Order circuit this way: inputs, outputs, gates
 * 
 */
void circuit::sort_circuit(){
    vector<node> inputs, outputs, gates;
    for(int i = 0; i<this->nodes.size(); i++){
        if (this->nodes[i].type == "INPUT"){
            inputs.push_back(this->nodes[i]);
        } else if (this->nodes[i].type == "OUTPUT"){
            outputs.push_back(this->nodes[i]);
        } else {
            gates.push_back(this->nodes[i]);
        }
    }
    for(int i = 0; i<inputs.size(); i++){
        this->nodes[i] = inputs[i];
    }
    for(int i = 0; i<outputs.size(); i++){
        this->nodes[i + inputs.size()] = outputs[i];
    }
    for(int i = 0; i<gates.size(); i++){
        this->nodes[i+inputs.size()+outputs.size()] = gates[i];
    }
}