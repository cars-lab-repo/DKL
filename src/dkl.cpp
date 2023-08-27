/*
Dual Key Lock Tool
Jordan Maynard
2023
*/

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "circuit.h"

using namespace std;

//Declare global variables, circuits, and .bench file names
circuit c, inputs_c, scanins_c, keys_c, ora_key_c, outputs_c, scanouts_c, gates_c, gates_enc_c, ac_c, fc_c;
string inputs_bench = "inputs.bench";
string keys_bench = "keys.bench";
string outputs_bench = "outputs.bench";
string outputs_final_bench = "outputs_final.bench";
string wires_bench = "wires.bench";
string wires_enc_bench = "wires_enc.bench";
string activation_bench = "modules/activation_counter.bench";
string functional_bench = "modules/functional_counter.bench";
string counters_bench = "counters.bench";
string final_bench, oracle_bench, og_bench;
bool scan_exposed, combinational;
vector<bool> initial_key;
vector<bool> final_key;
char choice;
int key_size;

/**
 * @brief Split original benchmark into subcircuits
 * 
 * @param og_b string original bench file name
 * @return int number of gates (0 if file does not open)
 */
int create_subcircuits(string og_b){
    string line;
    ifstream og(og_b.c_str());
    ofstream in(inputs_bench.c_str());
    ofstream out(outputs_bench.c_str());
    ofstream wires(wires_bench.c_str());

    if(scan_exposed){
        in << "INPUT(SCAN_SEL)" << endl;
    }

    if(og.is_open()){

        int inCount = 0,outCount = 0,gateCount = 0;

        while (getline(og,line)){

            if (line == "") continue;       //skip blank line

            node n;
            int input,output,comment,dff;

            input = line.find("INPUT");
            if(input != string::npos){         //check if line is input
                in << line << endl;
                n.parse_line(line);
                c.add_node(n);
                inputs_c.add_node(n);
                inCount++;
            }

            output = line.find("OUTPUT");
            if(output != string::npos){        //check if line is output
                out << line << endl;
                n.parse_line(line);
                c.add_node(n);
                outputs_c.add_node(n);
                outCount++;
            }

            if(scan_exposed){
                dff = line.find("DFF(");
                if(dff != string::npos){
                    n.parse_line(line);
                    in << "INPUT(" << n.id << "_SCPI)" << endl;
                    c.add_node(node(n.id+"_SCPI", 0, "INPUT"));
                    out << "OUTPUT(" << n.id << ")" << endl;
                    c.add_node(node(n.id, 0, "OUTPUT"));
                    wires << n.id << "M = MUX(SCAN_SEL, " << n.id << "_SCPI, " << n.fanins[0] << ")" << endl;
                    c.add_node(node(n.id+"M", 1, "MUX", {"SCAN_SEL", n.id + "_SCPI", n.fanins[0]}, {}));
                    wires << n.id << " = DFF(" << n.id << "M)" << endl;
                    c.add_node(node(n.id, 1, "DFF", {n.id + "M"}, {}));
                    inCount++;
                    outCount++;
                    gateCount += 2;
                    continue;
                }
            }

            comment = line.find("#");
            
            if((input == string::npos) && (output == string::npos) && (comment == string::npos)){ //check if line is other
                wires << line << endl;
                n.parse_line(line);
                c.add_node(n);
                gates_c.add_node(n);
                gateCount++;
            }
            
        }
        cout << "Num inputs: " << inCount << endl;
        cout << "Num outputs: " << outCount << endl;
        cout << "Num gates: " << gateCount << endl;
        c.populate_fanouts();
        inputs_c.populate_fanouts();
        outputs_c.populate_fanouts();
        gates_c.populate_fanouts();
        //cout << " UNSORTED Circuit: " << endl << "~~~~~~~~~~~~~~" << endl;
        //c.to_string();
        c.sort_circuit();
        //cout << endl << endl << " SORTED Circuit: " << endl << "~~~~~~~~~~~~~~" << endl;
        //c.to_string();
        return gateCount;
    }
    return 0;
}

/**
 * @brief Create desired number of keys and write them to keys file
 */
void create_keys(){
    node n;
    string key_name;
    for(int i = 0; i<key_size; i++){
        key_name = "keyinput" + to_string(i);
        n = node(key_name, false, "INPUT");
        n.write_to_bench(keys_bench);
        keys_c.add_node(n);
    }
    cout << endl << "Keys have been created!" << endl;
}

/**
 * @brief Let user choose correct initial and final key sequences
 * 
 * @param k initial or final key
 */
void choose_correct_key(char k){
    vector<bool> *correct_key;
    if(k == 'i') correct_key = &initial_key;
    else correct_key = &final_key;

    bool bit;
    cout << endl << "Enter each bit sequentially:" << endl;
    for(int i = 0; i<key_size; i++){
        cout << "Key bit " << i+1 << ": ";
        cin >> bit;
        (*correct_key).push_back(bit);
    }
    if(k == 'i') cout << endl << "Initial key: ";
    else cout << endl << "Final key: ";
    for(int i = 0; i< (*correct_key).size(); i++){
        cout << (*correct_key)[i];
    }
    cout << endl;
}

/**
 * @brief Randomize correct initial and final key sequences
 * 
 * @param k initial or final key
 */
void randomize_correct_key(char k){
    vector<bool> *correct_key;
    if(k == 'i') correct_key = &initial_key;
    else correct_key = &final_key;

    bool bit;
    for(int i = 0; i<key_size; i++){
        bit = rand() % 2;
        (*correct_key).push_back(bit);
    }
    if(k == 'i') cout << endl << "Initial key: ";
    else cout << endl << "Final key: ";
    for(int i = 0; i< (*correct_key).size(); i++){
        cout << (*correct_key)[i];
    }
    cout << endl;
}

/**
 * @brief Change output ids to correspond with encryption of output gates
 * 
 * @param replace_id vector of id pairs to replace (replace 2nd with 1st)
 */
void modify_outputs(vector<pair<string,string>> replace_id){
    string line;
    int found;
    node n;
    ifstream out(outputs_bench.c_str());
    ofstream out_f(outputs_final_bench.c_str());
    if (out.is_open()){
        while(getline(out,line)){

            for(int i = 0; i<replace_id.size(); i++){ 
                found = line.find(replace_id[i].first);
                if(found != string::npos){
                    out_f << "OUTPUT(" << replace_id[i].second << ")" << endl;
                    break;
                } else if (i == replace_id.size()-1){
                    out_f << line << endl;
                }
            }
        }
    }
}

/**
 * @brief Creates gates and writes them to file wires_enc_bench
 * 
 * @param to_encrypt node pointer to add encryption to
 * @param enc_number counter of logic being added
 */
void add_random_enc(node * to_encrypt, int enc_number, vector<pair<string,string>>& replace_fanin){
    node and_gate1, and_gate2, or_gate, xor_gate;
    int enc_names;
    //pair<string,string> replace_ids;
    enc_names = enc_number * 4;
    enc_names += 18; //Starts with DKA18-21, next is DKA22-25, etc
    string xor_name = (*to_encrypt).id;

    //reassign chosen gate with DKA id and fanout {and1,and2}
    *to_encrypt = node("DKA" + to_string(enc_names), (*to_encrypt).gate, (*to_encrypt).type, (*to_encrypt).fanins, {"DKA"+to_string(enc_names+1), "DKA"+to_string(enc_names+2)});
    //create and1 with fanin(W0,to_encrypt) and fanout(or)
    and_gate1 = node("DKA"+to_string(enc_names+1), true, "AND", {"W0",(*to_encrypt).id}, {"DKA"+to_string(enc_names+3)});
    //create and2 with fanin(W1,to_encrypt) and fanout(or)
    and_gate2 = node("DKA"+to_string(enc_names+2), true, "AND", {"W1",(*to_encrypt).id}, {"DKA"+to_string(enc_names+3)});
    //create or with fanin(and1,and2) and fanout(xor)
    or_gate = node("DKA"+to_string(enc_names+3), true, "OR", {and_gate1.id,and_gate2.id}, {(*to_encrypt).id});

    //create xor with id to_encrypt.id, fanin (KAx, or gate), and fanout (to_encrypt.fanouts)
    if (final_key[enc_number]) xor_gate = node(xor_name, true, "XNOR", {"keyinput"+to_string(enc_number),or_gate.id}, (*to_encrypt).fanouts);
    else xor_gate = node(xor_name, true, "XOR", {"keyinput"+to_string(enc_number),or_gate.id}, (*to_encrypt).fanouts);

    (*to_encrypt).write_to_bench(wires_enc_bench);
    and_gate1.write_to_bench(wires_enc_bench);
    and_gate2.write_to_bench(wires_enc_bench);
    or_gate.write_to_bench(wires_enc_bench);
    xor_gate.write_to_bench(wires_enc_bench);

    gates_enc_c.add_node(*to_encrypt);
    gates_enc_c.add_node(and_gate1);
    gates_enc_c.add_node(and_gate2);
    gates_enc_c.add_node(or_gate);
    gates_enc_c.add_node(xor_gate);
}

/**
 * @brief Decimal to binary converter
 * 
 * @param n decimal number
 * @return string binary number
 */
string dec_to_bin(int n){
    string bin_num;
    while(n > 0){
        bin_num = to_string(n % 2) + bin_num;
        n /= 2;
    }
    return bin_num;
}

/**
 * @brief Creates activation counter based on key size and values
 * Adds logic to activate after specified number of cycles
 * 
 * @param num_cycles number of cycles before activation
 */
void modify_activation(int num_cycles){
    string line, n_cycles_bin, mux_name, fanin;
    int curr_key_bit = 0, find_bit, find_mux, comma, closepar;
    n_cycles_bin = dec_to_bin(num_cycles);
    ifstream act_b(activation_bench.c_str());
    if(act_b.is_open()){
        while(getline(act_b,line)){
            node n = node();

            //stop when counter bit is higher than key size or # of bits in num_cycles
            if(curr_key_bit > n_cycles_bin.length()-1 || curr_key_bit > initial_key.size()-1) break; 
            if(line == "") continue;

            find_bit = line.find("#Bit"); //search for bit# comment

            mux_name = "ACM" + to_string(curr_key_bit) + " ="; 
            find_mux = line.find(mux_name); //search for MUXx1

            if(find_bit != string::npos) curr_key_bit = line[5] - '0'; //if line is counter bit header, change curr_key_bit

            else if((find_mux != string::npos) & initial_key[curr_key_bit]){ //modify necessary lines
                n.parse_line(line);
                fanin = n.fanins[1];
                n.remove_fanin(fanin);
                n.add_fanin(fanin);
                n.write_to_bench(counters_bench);
                ac_c.add_node(n);

            } else { //write all other gates to bench
                n.parse_line(line);
                n.write_to_bench(counters_bench);
                ac_c.add_node(n);
            }
            
        }
    }

    node act_and = node("DKA10", true, "AND");
    for(int i = n_cycles_bin.length()-1; i >= 0; i--){
        string dff_name = "ACFF" + to_string(n_cycles_bin.length()-i-1);
        if(scan_exposed){
            node scan = node(dff_name + "_SCPI", 0, "INPUT");
            scan.write_to_bench(inputs_bench);
            scanins_c.add_node(scan);
            scan = node(dff_name, 0, "OUTPUT");
            scan.write_to_bench(outputs_bench);
            scanouts_c.add_node(scan);
        }
        if(n_cycles_bin[i] == '0'){
            node not_dff = node();
            not_dff = node("n_"+dff_name, true, "NOT");
            not_dff.add_fanin(dff_name);
            not_dff.write_to_bench(counters_bench);
            ac_c.add_node(not_dff);
            act_and.add_fanin(not_dff.id);
        } else {
            act_and.add_fanin(dff_name);
        }
    }
    act_and.write_to_bench(counters_bench);
    ac_c.add_node(act_and);

    cout << endl << "Activation counter successfully modified!" << endl;
}

/**
 * @brief Create circuit for functional counter module
 */
void create_functional_circuit(){
    string line;
    ifstream func_b(functional_bench.c_str());

    if(func_b.is_open()){
        while(getline(func_b,line)){
            node n = node();
            n.parse_line(line);
            fc_c.add_node(n);
        }
    }
}

/**
 * @brief Add fixed key values to oracle
 */
void create_ora_key(){
    string pi0 = inputs_c.nodes[0].id;
    node n = node("n_"+pi0, true, "NOT", {pi0}, {});
    ora_key_c.add_node(n);
    n = node("GND", true, "AND", {pi0,"n_"+pi0}, {});
    ora_key_c.add_node(n);
    n = node("VCC", true, "OR", {pi0,"n_"+pi0}, {});
    ora_key_c.add_node(n);
    n = node("sel", true, "OR", {"W0","W1"}, {});
    ora_key_c.add_node(n);
    for(int i = 0; i < key_size; i++){
        vector<string> fanins = {"sel"};
        if(initial_key[i]) fanins.push_back("VCC");
        else fanins.push_back("GND");
        if(final_key[i]) fanins.push_back("VCC");
        else fanins.push_back("GND");
        n = node("keyinput"+to_string(i), true, "MUX", fanins, {});
        ora_key_c.add_node(n);
    }
}

/**
 * @brief Randomly selects gates to be encrypted
 * 
 * @param num_gates total number of encryptable gates in circuit
 */
void choose_random_gates(int num_gates){
    string line;
    bool add;
    int random_line, current_line=0, num_encrypted=0;
    vector<int> chosen_lines;
    vector<pair<string,string>> replace_fanin;
    ifstream wires(wires_bench.c_str());
    ofstream wires_enc(wires_enc_bench.c_str(), fstream::app);

    for (int i = 0; i < key_size; i++){
        random_line = rand() % num_gates;
        add = true;
        for (int j = 0; j < chosen_lines.size(); j++){
            if (chosen_lines[j] == random_line) {
                i--;
                add = false;
            }
        }
        if (add) chosen_lines.push_back(random_line);
    }

    if(wires.is_open()){
        while(getline(wires, line)){
            node n;
            node * chosen;
            add = false;
            n.parse_line(line);

            for (int i : chosen_lines){
                if (i == current_line) add = true;
            }
            if (add) {
                chosen = c.find_node_id(n.id);
                add_random_enc(chosen, num_encrypted, replace_fanin);
                num_encrypted++;
            } else {
                for(int i = 0; i<replace_fanin.size(); i++){ 
                    for(int j = 0; j < n.fanins.size(); j++){
                        if(replace_fanin[i].first == n.fanins[j]){
                            n.remove_fanin(replace_fanin[i].first);
                            n.add_fanin(replace_fanin[i].second);
                        }
                    }
                }
                n.write_to_bench(wires_enc_bench);
                gates_enc_c.add_node(n);
            }
            //cout << "Gate #" << current_line+1 << " encrypted? " << add << endl;
            current_line++;
        }
    }

    cout << endl << "Random gates have been chosen!" << endl;
}

/**
 * @brief opens scan chain of sequential circuit
 */
void convert_to_comb(){
    //go thru ac_c
    for(int i = 0; i<ac_c.nodes.size(); i++){
        if(ac_c.nodes[i].type == "DFF"){
            node n = node(ac_c.nodes[i].id, false, "INPUT");
            scanins_c.add_node(n);
            n = node(ac_c.nodes[i].fanins[0], false, "OUTPUT");
            scanouts_c.add_node(n);
            ac_c.remove_node(ac_c.nodes[i]);
            i--;
        }
    }
    //go thru fc_c
    for(int i = 0; i<fc_c.nodes.size(); i++){
        if(fc_c.nodes[i].type == "DFF"){
            node n = node(fc_c.nodes[i].id, false, "INPUT");
            scanins_c.add_node(n);
            n = node(fc_c.nodes[i].fanins[0], false, "OUTPUT");
            scanouts_c.add_node(n);
            fc_c.remove_node(fc_c.nodes[i]);
            i--;
        }
    }
    //go thru gates_enc_c
    for(int i = 0; i<gates_enc_c.nodes.size(); i++){
        if(gates_enc_c.nodes[i].type == "DFF"){
            node n = node(gates_enc_c.nodes[i].id, false, "INPUT");
            scanins_c.add_node(n);
            n = node(gates_enc_c.nodes[i].fanins[0], false, "OUTPUT");
            scanouts_c.add_node(n);
            gates_enc_c.remove_node(gates_enc_c.nodes[i]);
            i--;
        }
    }
}

/**
 * @brief Create a final .bench output
 * 
 * @param nc number of cycles
 * @param oracle create oracle?
 */
void create_final(int nc, bool oracle){
    string bench = final_bench;
    if(oracle) bench = oracle_bench;
    ofstream final_b(bench.c_str(), fstream::app);

    final_b << "#Initial key: ";
    for(int i = 0; i< initial_key.size(); i++){
        final_b << initial_key[i];
    }
    final_b << endl << "#Final key: ";
    for(int i = 0; i< final_key.size(); i++){
        final_b << final_key[i];
    }
    final_b << endl << "#Num cycles to activate: " << to_string(nc) << endl;

    final_b << endl << "#Primary Inputs" << endl;
    inputs_c.write_to_bench(bench);

    //split inputs into primary inputs and scan inputs
    if(scan_exposed | combinational){
        final_b << endl << "#Scan Inputs" << endl;
        scanins_c.write_to_bench(bench);
    }

    if(!oracle){
        final_b << endl << "#Key Inputs" << endl;
        keys_c.write_to_bench(bench);
    }

    final_b << endl << "#Primary Outputs" << endl;
    outputs_c.write_to_bench(bench);

    if(scan_exposed | combinational){
        final_b << endl << "#Scan Outputs" << endl;
        scanouts_c.write_to_bench(bench);
    }

    if(oracle){
        final_b << endl << "#Fixed Key Values" << endl;
        ora_key_c.write_to_bench(bench);
    }

    final_b << endl << "#Activation Counter" << endl;
    ac_c.write_to_bench(bench);

    final_b << endl << "#Functional Counter" << endl;
    fc_c.write_to_bench(bench);

    final_b << endl << "#Gates" << endl;
    gates_enc_c.write_to_bench(bench);

    cout << endl << bench << " has been created!" << endl;
}


/**
 * @brief Take user inputs related to output options
 */
void user_options(){
    cout << "Enter .bench file name: ";
    cin >> og_bench;
    final_bench = "encrypted/" + og_bench;
    og_bench.append(".bench");

    cout << endl << "Would you like to convert to sequential scan exposed? (Y/N): ";
    cin >> choice;
    if(tolower(choice) == 'y'){
        scan_exposed = true;
        activation_bench = "modules/activation_counter_se.bench";
        functional_bench = "modules/functional_counter_se.bench";
    } else {
        scan_exposed = false;
        cout << endl << "Would you like to convert to combinational scan exposed? (Y/N): ";
        cin >> choice;
        if(tolower(choice) == 'y'){
            combinational = true;
        } else {
            combinational = false;
        }
    }
}

/**
 * @brief Name output files, create functional circuit, add scan inputs if scan exposed
 */
void modify_bench_names(){
    final_bench.append("_enc"+to_string(key_size));
    if(scan_exposed) final_bench.append("_se");
    if(combinational) final_bench.append("_comb");
    oracle_bench = final_bench;

    final_bench.append(".bench");
    oracle_bench.append("_oracle.bench");

    if(scan_exposed){
        node scan = node("W0_SCPI", 0, "INPUT");
        scan.write_to_bench(inputs_bench);
        scan = node("W1_SCPI", 0, "INPUT");
        scan.write_to_bench(inputs_bench);
        scan = node("W0", 0, "OUTPUT");
        scan.write_to_bench(outputs_bench);
        scan = node("W1", 0, "OUTPUT");
        scan.write_to_bench(outputs_bench);
    }
}

/**
 * @brief Create keys based on user specifications
 * 
 * @param num_gates number of gates in og circuit
 */
void user_keys(int num_gates){
    cout << endl << "Enter key size: ";
    cin >> key_size;
    while((key_size > num_gates) | (key_size < 2)){
        cout << endl << "Key size out of range. Please choose a number between 2 and " << num_gates << " inclusive: ";
        cin >> key_size;
    }
    create_keys();

    //Get correct initial and final keys
    cout << endl << "Would you like to choose the correct keys? (Y/N): ";
    cin >> choice;
    if(tolower(choice) == 'y'){
        choose_correct_key('i');
        choose_correct_key('f');
    } else {
        randomize_correct_key('i');
        randomize_correct_key('f');
    }

    if(equal(initial_key.begin(), initial_key.end(), final_key.begin())){
        cout << endl << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        cout << "~WARNING: INITIAL AND FINAL KEYS ARE THE SAME.~" << endl;
        cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"  << endl;
    }
}

/**
 * @brief Get user input for number of cycles to activate
 * 
 * @return int number of cycles
 */
int get_num_cycles(){
    int num_cycles;
    int max_nc = pow(2,key_size) - 1;
    cout << endl << "Enter number of cycles to activate (max=" << to_string(max_nc) << "): ";
    cin >> num_cycles;
    while(num_cycles > max_nc){
        cout << endl << "Number of cycles too large for counter. Please choose a number " << to_string(max_nc) << " or below: ";
        cin >> num_cycles;
    }
    return num_cycles;
}

/**
 * @brief MAIN FUNCTION
 */
int main(){
    srand (time(0));
    int num_cycles, num_gates;

    //Take user inputs and prepare og circuit for encryption
    user_options();
    num_gates = create_subcircuits(og_bench);
    user_keys(num_gates);
    modify_bench_names();
    create_functional_circuit();
    num_cycles = get_num_cycles();
    modify_activation(num_cycles);

    //Create and insert customized encryption logic
    choose_random_gates(num_gates);
    if(combinational) convert_to_comb();
    if(combinational | scan_exposed) create_ora_key();
    create_final(num_cycles, false);
    create_final(num_cycles, true);

    //Get rid of intermediate bench junk
    remove(counters_bench.c_str());
    remove(inputs_bench.c_str());
    remove(keys_bench.c_str());
    remove(outputs_final_bench.c_str());
    remove(outputs_bench.c_str());
    remove(wires_enc_bench.c_str());
    remove(wires_bench.c_str());

    //The end!
    cout << endl << "Success!" << endl << endl;

}
