#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cstddef>      /* std::size_t */
#include <stdlib.h>     /* atoi */
#include <sstream>
#include <map>
#include <vector>

using namespace std;

struct module_meta
{
    int min;
    int max;

    module_meta() {
        min = -1;
        max = -1;
    }
};

struct symbol_meta {
    int address;
    int module;
    bool is_used;
    bool is_multi_defined;

    symbol_meta() {
        address = -1;
        module = -1;
        is_used = 1;
        is_multi_defined = 0;
    }
    symbol_meta(int a, int m) {
        address = a;
        module = m;
        is_used = 0;
        is_multi_defined = 0;
    }
};

bool size_checker(int x) { return ((x%1000)<512); }

int stoi(string str) { return atoi(str.c_str()); }
string to_string(int a) {
    stringstream ss;
    ss << a;
    return ss.str();
}

void __parseerror(int errcode, int num_line, int offset) {
    static string errstr[] = {
        "NUM_EXPECTED", // Number expect
        "SYM_EXPECTED", // Symbol Expected
        "ADDR_EXPECTED", // Addressing Expected which is A/E/I/R
        "SYM_TOO_LONG", // Symbol Name is too long
        "TO_MANY_DEF_IN_MODULE", // > 16
        "TO_MANY_USE_IN_MODULE", // > 16
        "TO_MANY_INSTR" // total num_instr exceeds memory size (512)
    };
    cout << "Parse Error line " << num_line << " offset " << offset+1 << ": " << errstr[errcode] << endl;
}

bool get_nonempty_line(ifstream& infile, string& s, int& num_line, size_t& start) {
    string tmp_s = "";
    while (tmp_s.empty()) {
        if (infile.eof()) return 0;
        if (getline(infile, tmp_s)) {
            start = 0;
            ++num_line;
        }
    }
    s = tmp_s;
    return 1;
}

bool parse_token(ifstream& infile, string& s, string& token, size_t& start, int exp, int abs_address, int& num_line) {
    size_t t1, t2;
    bool rt_val = 1;
    string tmp = s.substr(start,string::npos);
    t1 = tmp.find_first_not_of(" \t");
    if (start==s.size() or t1 == string::npos) {
        if (get_nonempty_line(infile, s, num_line, start)) {
            tmp = s;
            t1 = tmp.find_first_not_of(" \t");

            tmp = tmp.substr(t1, string::npos);
            t2 = tmp.find_first_of(" \t");
            token = tmp.substr(0,t2);
        }
        else { //no next line
            token = "";
            t1 = 0;
            rt_val = 0;
        }
    }
    else {
        tmp = tmp.substr(t1, string::npos);
        t2 = tmp.find_first_of(" \t");
        token = tmp.substr(0,t2);
    }
    switch (exp) {
        int offset;
        case 0: //number
            offset = token.find_first_not_of("0123456789");
            if (offset != string::npos or token.empty()) {
                offset = start+t1;
                __parseerror(0,num_line,offset);
                return 0;
            }
            break;
        case 1: //symbol
            offset = token.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
            if (offset != 0) {
                offset = start+t1;
                __parseerror(1,num_line,offset);
                return 0;
            }
            offset = token.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
            if (offset != string::npos) {
                offset = start+t1;
                __parseerror(1,num_line,offset);
                return 0;
            }
            if (token.size()>16) {
                offset = start+t1;
                __parseerror(3,num_line,offset);
                return 0;
            }
            break;
        case 2: //address
            offset = token.find_first_not_of("IARE");
            if (offset != string::npos or token.size()!=1) {
                offset = start+t1;
                __parseerror(2,num_line,offset);
                return 0;
            }
            break;
        case 3: //total num_instr
            offset = token.find_first_not_of("0123456789");
            if (offset != string::npos) {
                offset = start+t1;
                __parseerror(0,num_line,offset);
                return 0;
            }
            if (stoi(token)+abs_address>512) {
                offset = start+t1;
                __parseerror(6,num_line,offset);
                return 0;
            }
            break;
        case 4: //num_def
            if (token.empty()) return 0;
            offset = token.find_first_not_of("0123456789");
            if (offset != string::npos) {
                offset = start+t1;
                __parseerror(0,num_line,offset);
                return 0;
            }
            if (stoi(token)>16) {
                offset = start+t1;
                __parseerror(4,num_line,offset);
                return 0;
            }
            break;
        case 5: //num_use
            offset = token.find_first_not_of("0123456789");
            if (offset != string::npos or token.empty()) {
                offset = start+t1;
                __parseerror(0,num_line,offset);
                return 0;
            }
            if (stoi(token)>16) {
                offset = start+t1;
                __parseerror(5,num_line,offset);
                return 0;
            }
            break;
        default:
            break;
    }
    if (t2 == string::npos) start = s.size();
    else start += (t1+t2+1);
    return rt_val;
}

bool get_token(string s, string& token, size_t& start) {
    if (start == s.size()) return 0;
    string tmp = s.substr(start,string::npos);
    size_t t1 = tmp.find_first_not_of(" \t");
    if (t1 == string::npos) {
        ++start;
        return 0;
    }
    tmp = tmp.substr(t1, string::npos);
    size_t t2 = tmp.find_first_of(" \t");
    token = tmp.substr(0,t2);

    if (t2 == string::npos) start = s.size();
    else start += (t1+t2+1);
    return 1;
}

bool pass_one(ifstream& infile, map<string,symbol_meta>& def_list, map<int,module_meta>& module_list) {
    string s = "", buf, symbol, type;
    int abs_address = 0, num_line = 0, n, i, m = 0, use_cnt;
    bool status;
    size_t start = 0;
    while (1) {
        //definition list
        status = parse_token(infile, s, buf, start, 4, abs_address, num_line);
        if (buf=="") break; //touch EOF
        if (!status) return 0;
        else {
            n = stoi(buf);
            ++m;
            module_meta mt;
            mt.min = abs_address;
            module_list[m] = mt;
            i = 0;
        }

        while (i<2*n) {
            if(i%2==0) { //symbol
                status = parse_token(infile, s, buf, start, 1, abs_address, num_line);
                if (status) {
                    symbol = buf;
                    ++i;
                }
                else return 0;
            }
            else { //number
                status = parse_token(infile, s, buf, start, 0, abs_address, num_line);
                if (status) {
                    ++i;
                    if (def_list.find(symbol)==def_list.end()) {
                        symbol_meta mt(stoi(buf)+abs_address, m);
                        def_list[symbol] = mt;
                    }
                    else if (def_list[symbol].address==-1) {
                        def_list[symbol].address = stoi(buf)+abs_address;
                        def_list[symbol].module = m;
                    }
                    else {
                        def_list[symbol].is_multi_defined = 1;
                    }
                }
                else return 0;
            }
        }

        //use list
        status = parse_token(infile, s, buf, start, 5, abs_address, num_line);
        if (!status) return 0;
        else {
            n = stoi(buf);
            i = 0;
        }
        use_cnt = n;
        string * use_list = new string[use_cnt];

        while (i<n) {
            status = parse_token(infile, s, buf, start, 1, abs_address, num_line);
            if(status) { //symbol
                use_list[i] = buf;
                ++i;
            }
            else return 0;
        }

        //program text
        status = parse_token(infile, s, buf, start, 3, abs_address, num_line);

        if (!status) return 0;
        else {
            n = stoi(buf);
            i = 0;
        }

        while (i<2*n) {
            if(i%2==0) { //inst
                status = parse_token(infile, s, buf, start, 2, abs_address, num_line);
                if (status) {
                    ++i;
                    type = buf;
                }
                else return 0;
            }
            else { //number
                status = parse_token(infile, s, buf, start, 0, abs_address, num_line);
                if (status) {
                    ++i;
                    if (type == "E") {
                        int order = stoi(buf.substr(1,-1));
                        if (order>=use_cnt) continue;
                        symbol = use_list[order];
                        if (def_list.find(symbol)==def_list.end()) {
                            symbol_meta mt;
                            def_list[symbol] = mt;
                        }
                        else def_list[symbol].is_used = 1;
                    }
                }
                else return 0;
            }
        }
        abs_address += n;
        module_list[m].max = abs_address;
        delete[] use_list;
    }
    return 1;
}

void pass_two(ifstream& infile, map<string,symbol_meta>& def_list, map<int,module_meta>& module_list) {
    for (map<string,symbol_meta>::iterator it = def_list.begin(); it != def_list.end(); ++it) {
        if (it->second.address > (module_list[it->second.module].max) && it->second.address!=-1) {
            cout << "Warning: Module " << it->second.module << ": " << it->first << " too big " << it->second.address << " (max=" << module_list[it->second.module].max-1 << ") assume zero relative\n";
            it->second.address = 0 + module_list[it->second.module].min;
        }
    }
    cout << "Symbol Table\n";
    for (map<string,symbol_meta>::iterator it = def_list.begin(); it != def_list.end(); ++it) {
        if (it->second.address!=-1) cout << it->first << "=" << it->second.address;
        if (it->second.is_multi_defined) cout << " Error: This variable is multiple times defined; first value used";
        if (it->second.address!=-1) cout << "\n";
    }

    cout << "\n";
    cout << "Memory Map\n";

    vector<string> warnings;
    int idx = -1;
    string s = "", buf, type, sub;
    int abs_address = 0, num_line = 0, n, i, m = 0, use_cnt;
    size_t start = 0;
    while (1) {
        //definition list
        if (!get_token(s, buf, start)) {
            if (get_nonempty_line(infile, s, num_line, start)) get_token(s, buf, start);
            else break;
        }
        ++m;
        n = stoi(buf);
        i = 0;
        while (i<2*n) {
            if(get_token(s, buf, start)) {
                if (i%2==0 && def_list[buf].is_used==0) {
                    string w = "Warning: Module " + to_string(m) + ": " + buf + " was defined but never used";
                    warnings.push_back(w);
                }
                ++i;
            }
            else get_nonempty_line(infile, s, num_line, start);
        }

        //use list
        if (!get_token(s, buf, start)) {
            get_nonempty_line(infile, s, num_line, start);
            get_token(s, buf, start);
        }
        n = stoi(buf);
        use_cnt = n;
        string * use_list = new string[use_cnt];
        bool* is_local_used = new bool[use_cnt];
        i = 0;
        while (i<n) {
            if(get_token(s, buf, start)) {
                use_list[i] = buf;
                is_local_used[i] = 0;
                ++i;
            }
            else get_nonempty_line(infile, s, num_line, start);
        }
        //program text
        if (!get_token(s, buf, start)) {
            get_nonempty_line(infile, s, num_line, start);
            get_token(s, buf, start);
        }
        n = stoi(buf);
        i = 0;
        while (i<2*n) {
            if(get_token(s, buf, start)) {
                if (i%2==0) { //type
                    type = buf;
                    ++i;
                }
                else {
                    ++idx;
                    ++i;
                    int fill_in_zeros = 4-buf.size();
                    for (int j=0; j<fill_in_zeros; ++j) buf += "0";
                    cout << setfill('0') << setw(3) << idx << ": ";
                    if (type == "I") {
                        if (stoi(buf)>9999) cout << "9999 Error: Illegal immediate value; treated as 9999\n";
                        else cout << buf << endl;
                    }
                    else if (type == "A") {
                        if (size_checker(stoi(buf.substr(1,-1)))) cout << buf << "\n";
                        else {
                            cout << buf[0] << "000";
                            cout << " Error: Absolute address exceeds machine size; zero used\n";
                        }
                    }
                    else if (type == "R") {
                        int tmp = stoi(buf);
                        if (tmp>9999) {
                            cout << "9999 Error: Illegal opcode; treated as 9999\n";
                            continue;
                        }
                        if ((tmp%1000)>n) {
                            tmp = (tmp/1000)*1000;
                            tmp += abs_address;
                            if (size_checker(tmp)) cout << tmp << " Error: Relative address exceeds module size; zero used\n";
                            else {
                                cout << (tmp/1000) << "000";
                                cout << " Error: Absolute address exceeds machine size; zero used\n";
                            }
                        }
                        else {
                            tmp += abs_address;
                            if (size_checker(tmp)) cout << tmp << "\n";
                            else {
                                cout << buf[0] << "000";
                                cout << " Error: Absolute address exceeds machine size; zero used\n";
                            }
                        }

                    }
                    else if (type == "E") {
                        int order = stoi(buf.substr(1,-1));
                        if (order>=use_cnt) {
                            cout << buf << " Error: External address exceeds length of uselist; treated as immediate\n";
                            continue;
                        }
                        string symbol = use_list[order];
                        is_local_used[order] = 1;
                        if (def_list[symbol].address==-1) {
                            cout << buf[0] << "000";
                            cout << " Error: " << symbol << " is not defined; zero used\n";
                            continue;
                        }
                        sub = to_string(def_list[symbol].address);
                        buf = buf.replace(buf.end()-sub.size(),buf.end(),sub);
                        int tmp = stoi(buf.substr(1,-1));
                        if (size_checker(tmp)) cout << buf << "\n";
                        else {
                            cout << buf[0] << "000";
                            cout << " Error: Absolute address exceeds machine size; zero used\n";
                        }

                    }
                }
            }
            else get_nonempty_line(infile, s, num_line, start);
        }
        abs_address += n;
        for (int j = 0; j<use_cnt; ++j) {
            if (!is_local_used[j]) cout << "Warning: Module " << to_string(m) << ": " << use_list[j] << " appeared in the uselist but was not actually used\n";
        }

        delete[] use_list;
        delete[] is_local_used;
    }
    cout << "\n";
    for (vector<string>::iterator it = warnings.begin(); it != warnings.end(); ++it) {
        cout << *it << "\n";
    }
}

int main (int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Usage: ./linker <file_path>" << endl;
        return 0;
    }
    ifstream infile;
    infile.open(argv[1]);

    map<string,symbol_meta> def_list;
    map<int,module_meta> module_list;

    if (!pass_one(infile, def_list, module_list)) {
        infile.close();
        return 0;
    }
    infile.close();
    infile.open(argv[1]);
    pass_two(infile, def_list, module_list);
    infile.close();
    return 0;
}
