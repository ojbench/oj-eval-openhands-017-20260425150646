#include <bits/stdc++.h>
using namespace std;

struct User {
    string username;
    string password;
    string name;
    string mail;
    int privilege = 0;
};

static inline string trim(const string &s){
    size_t i=0,j=s.size();
    while(i<j && isspace((unsigned char)s[i])) ++i;
    while(j>i && isspace((unsigned char)s[j-1])) --j;
    return s.substr(i,j-i);
}

static vector<string> split_ws(const string &s){
    vector<string> res; string cur; res.reserve(16);
    for(size_t i=0;i<s.size();){
        while(i<s.size() && isspace((unsigned char)s[i])) ++i;
        if(i>=s.size()) break;
        size_t j=i;
        while(j<s.size() && !isspace((unsigned char)s[j])) ++j;
        res.emplace_back(s.substr(i,j-i));
        i=j;
    }
    return res;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    unordered_map<string, User> users;
    unordered_set<string> online;

    string line;
    while (true){
        if(!std::getline(cin, line)) break;
        string t = trim(line);
        if(t.empty()) continue;

        // tokenize
        vector<string> tok = split_ws(t);
        string cmd = tok[0];

        auto get_arg = [&](const string &key)->string{
            for(size_t i=1;i+1<tok.size();++i){
                if(tok[i]==key) return tok[i+1];
            }
            return string();
        };

        if(cmd == "exit"){
            cout << "bye\n";
            return 0;
        }
        else if(cmd == "clean"){
            users.clear();
            online.clear();
            cout << 0 << "\n";
        }
        else if(cmd == "add_user"){
            string c = get_arg("-c");
            string u = get_arg("-u");
            string p = get_arg("-p");
            string n = get_arg("-n");
            string m = get_arg("-m");
            string gstr = get_arg("-g");
            if(users.empty()){
                if(users.count(u)) { cout << -1 << "\n"; continue; }
                User usr; usr.username=u; usr.password=p; usr.name=n; usr.mail=m; usr.privilege=10;
                users[u]=usr; cout << 0 << "\n"; continue;
            }
            if(!users.count(c) || !online.count(c)) { cout << -1 << "\n"; continue; }
            if(users.count(u)) { cout << -1 << "\n"; continue; }
            int g=0; try{ g = stoi(gstr); } catch(...) { cout << -1 << "\n"; continue; }
            if(g >= users[c].privilege) { cout << -1 << "\n"; continue; }
            User usr; usr.username=u; usr.password=p; usr.name=n; usr.mail=m; usr.privilege=g;
            users[u]=usr; cout << 0 << "\n";
        }
        else if(cmd == "login"){
            string u = get_arg("-u");
            string p = get_arg("-p");
            auto it = users.find(u);
            if(it==users.end() || it->second.password!=p || online.count(u)) { cout << -1 << "\n"; }
            else { online.insert(u); cout << 0 << "\n"; }
        }
        else if(cmd == "logout"){
            string u = get_arg("-u");
            if(online.count(u)) { online.erase(u); cout << 0 << "\n"; }
            else cout << -1 << "\n";
        }
        else if(cmd == "query_profile"){
            string c = get_arg("-c");
            string u = get_arg("-u");
            if(!users.count(c) || !online.count(c) || !users.count(u)) { cout << -1 << "\n"; continue; }
            if(!(users[c].privilege > users[u].privilege || c==u)) { cout << -1 << "\n"; continue; }
            User &x = users[u];
            cout << x.username << ' ' << x.name << ' ' << x.mail << ' ' << x.privilege << "\n";
        }
        else if(cmd == "modify_profile"){
            string c = get_arg("-c");
            string u = get_arg("-u");
            if(!users.count(c) || !online.count(c) || !users.count(u)) { cout << -1 << "\n"; continue; }
            if(!(users[c].privilege > users[u].privilege || c==u)) { cout << -1 << "\n"; continue; }
            string np = get_arg("-p");
            string nn = get_arg("-n");
            string nm = get_arg("-m");
            string ngs = get_arg("-g");
            if(!ngs.empty()){
                int ng;
                try{ ng = stoi(ngs);}catch(...){ cout << -1 << "\n"; continue; }
                if(ng >= users[c].privilege) { cout << -1 << "\n"; continue; }
                users[u].privilege = ng;
            }
            if(!np.empty()) users[u].password = np;
            if(!nn.empty()) users[u].name = nn;
            if(!nm.empty()) users[u].mail = nm;
            User &x = users[u];
            cout << x.username << ' ' << x.name << ' ' << x.mail << ' ' << x.privilege << "\n";
        }
        else if(cmd == "query_ticket" || cmd == "query_transfer"){
            cout << 0 << "\n";
        }
        else if(cmd == "query_train"){
            cout << -1 << "\n";
        }
        else if(cmd == "add_train" || cmd == "release_train" || cmd == "delete_train" || cmd == "buy_ticket" || cmd == "query_order" || cmd == "refund_ticket"){
            cout << -1 << "\n";
        }
        else {
            cout << -1 << "\n";
        }
    }
    return 0;
}
