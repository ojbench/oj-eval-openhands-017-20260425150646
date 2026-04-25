#include <bits/stdc++.h>
using namespace std;

static const char* USERS_DB = "users.db";
static const char* TRAINS_DB = "trains.db";

static const char* SEATS_DB = "seats.db";

struct User {
    string username;
    string password;
    string name;
    string mail;
    int privilege = 0;
};

struct Train {
    string id;
    int stationNum = 0;
    vector<string> stations;
    int seatNum = 0;
    vector<int> prices;        // size n-1
    vector<int> travel;        // size n-1
    vector<int> stopover;      // size n-2
    int startH = 0, startM = 0;
    int saleStart = 0; // day index from 06-01
    int saleEnd = 0;
    char type = 'G';
    bool released = false;
};

static vector<string> split_bar(const string &s){
    vector<string> res; size_t pos=0; while(true){ size_t q=s.find('|',pos); string part=s.substr(pos, q==string::npos? string::npos : q-pos); res.push_back(part); if(q==string::npos) break; pos=q+1; }
    return res;
}
static string join_bar(const vector<string> &v){
    string r; for(size_t i=0;i<v.size();++i){ if(i) r.push_back('|'); r += v[i]; } return r;
}
static string join_bar_int(const vector<int> &v){
    string r; for(size_t i=0;i<v.size();++i){ if(i) r.push_back('|'); r += to_string(v[i]); } return r;
}

static void save_users(const unordered_map<string,User> &users){
    ofstream f(USERS_DB);
    if(!f) return;
    for(const auto &kv: users){
        const User &u = kv.second;
        f << u.username << '\t' << u.password << '\t' << u.name << '\t' << u.mail << '\t' << u.privilege << '\n';
    }
}
static void load_users(unordered_map<string,User> &users){
    users.clear();
    ifstream f(USERS_DB);
    if(!f) return;
    string line;
    while(getline(f,line)){
        if(line.empty()) continue;
        // fields separated by tabs
        vector<string> parts; parts.reserve(5);
        size_t pos=0; while(true){ size_t q=line.find('\t',pos); string part=line.substr(pos, q==string::npos? string::npos: q-pos); parts.push_back(part); if(q==string::npos) break; pos=q+1; }
        if(parts.size()<5) continue;
        User u; u.username=parts[0]; u.password=parts[1]; u.name=parts[2]; u.mail=parts[3]; u.privilege=stoi(parts[4]);
        users[u.username]=u;
    }
}

static void save_trains(const unordered_map<string,Train> &trains){
    ofstream f(TRAINS_DB);
    if(!f) return;
    for(const auto &kv: trains){
        const Train &t = kv.second;
        f << t.id << '\t' << t.stationNum << '\t' << t.seatNum << '\t'
          << t.startH << '\t' << t.startM << '\t' << t.saleStart << '\t' << t.saleEnd << '\t'
          << t.type << '\t' << (t.released?1:0) << '\t'
          << join_bar(t.stations) << '\t' << join_bar_int(t.prices) << '\t' << join_bar_int(t.travel) << '\t' << join_bar_int(t.stopover) << '\n';
    }
}
static void load_trains(unordered_map<string,Train> &trains){
    trains.clear();
    ifstream f(TRAINS_DB);
    if(!f) return;
    string line;
    while(getline(f,line)){
        if(line.empty()) continue;
        vector<string> parts; parts.reserve(13);
        size_t pos=0; while(true){ size_t q=line.find('\t',pos); string part=line.substr(pos, q==string::npos? string::npos: q-pos); parts.push_back(part); if(q==string::npos) break; pos=q+1; }
        if(parts.size()<13) continue;
        Train t; t.id=parts[0]; t.stationNum=stoi(parts[1]); t.seatNum=stoi(parts[2]); t.startH=stoi(parts[3]); t.startM=stoi(parts[4]); t.saleStart=stoi(parts[5]); t.saleEnd=stoi(parts[6]); t.type=parts[7].empty()?'G':parts[7][0]; t.released = (parts[8]=="1");
        t.stations = split_bar(parts[9]);
        { vector<string> v = split_bar(parts[10]); t.prices.clear(); for(auto &x:v) if(!x.empty()) t.prices.push_back(stoi(x)); }
struct Order {
    string trainID;
    string fromS;
    string toS;
    int depAbs=0;
    int arrAbs=0;
    int price=0;
    int num=0;
    string status; // success, pending, refunded
    int baseDay=0;
    int fi=0, ti=0; // segment indices
};

        { vector<string> v = split_bar(parts[11]); t.travel.clear(); for(auto &x:v) if(!x.empty()) t.travel.push_back(stoi(x)); }
        { vector<string> v = split_bar(parts[12]); t.stopover.clear(); for(auto &x:v) if(!x.empty()) t.stopover.push_back(stoi(x)); }
        trains[t.id]=t;
    }
}


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

static int md_to_day(const string &md){
    // mm-dd to index starting from 06-01 -> 0
    if(md.size()!=5 || md[2] != '-') return -1;
    int mm = stoi(md.substr(0,2));
    int dd = stoi(md.substr(3,2));
    int days_before=0;
    if(mm<6 || mm>8) return -1;
    if(mm==6){ days_before = 0; if(dd<1||dd>30) return -1; return (dd-1); }
    if(mm==7){ days_before = 30; if(dd<1||dd>31) return -1; return days_before + (dd-1); }
    // mm==8
    days_before = 30+31; if(dd<1||dd>31) return -1; return days_before + (dd-1);
}

static string day_to_md(int dayIndex){
    int d = dayIndex;
    if(d < 30){ // June
        int dd = d+1; char buf[8]; snprintf(buf,sizeof(buf),"06-%02d", dd); return string(buf);
    }
    d -= 30;
    if(d < 31){ // July
        int dd = d+1; char buf[8]; snprintf(buf,sizeof(buf),"07-%02d", dd); return string(buf);
    }
    d -= 31; // August
    int dd = d+1; char buf[8]; snprintf(buf,sizeof(buf),"08-%02d", dd); return string(buf);
}

static string fmt_time(int totalMinutes){
    if(totalMinutes < 0) return "xx-xx xx:xx";
    int day = totalMinutes / (24*60);
    int rem = totalMinutes % (24*60);
    int hh = rem / 60;
    int mi = rem % 60;
    char buf[17];
    snprintf(buf, sizeof(buf), "%s %02d:%02d", day_to_md(day).c_str(), hh, mi);
    return string(buf);
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    unordered_map<string, User> users;
    unordered_set<string> online;
    unordered_map<string, Train> trains;
    load_users(users);
    load_trains(trains);

    string line;
    while (true){
        if(!std::getline(cin, line)) break;
        string t = trim(line);
        if(t.empty()) continue;
        // strip optional leading [....] prefix and capture it
        string prefix;
        if(!t.empty() && t[0]=='['){
            size_t rb = t.find(']');
            if(rb!=string::npos){
                prefix = t.substr(0, rb+1) + ' ';
                t = trim(t.substr(rb+1));
            }
        }

        // tokenize
        vector<string> tok = split_ws(t);
        if(tok.empty()) continue;
        string cmd = tok[0];

        auto get_arg = [&](const string &key)->string{
            for(size_t i=1;i+1<tok.size();++i){
                if(tok[i]==key) return tok[i+1];
            }
            return string();
        };
        auto out = [&](const string &s){ cout << prefix << s << '\n'; };

        if(cmd == "exit"){
            out("bye");
            return 0;
        }
        else if(cmd == "clean"){
            users.clear();
            online.clear();
            trains.clear();
            save_users(users);
            save_trains(trains);
            out("0");
        }
        else if(cmd == "add_user"){
            string c = get_arg("-c");
            string u = get_arg("-u");
            string p = get_arg("-p");
            string n = get_arg("-n");
            string m = get_arg("-m");
            string gstr = get_arg("-g");
            if(users.empty()){
                if(users.count(u)) { out("-1"); continue; }
                User usr; usr.username=u; usr.password=p; usr.name=n; usr.mail=m; usr.privilege=10;
                users[u]=usr; save_users(users); out("0"); continue;
            }
            if(!users.count(c) || !online.count(c)) { out("-1"); continue; }
            if(users.count(u)) { out("-1"); continue; }
            int g=0; try{ g = stoi(gstr); } catch(...) { out("-1"); continue; }
            if(g < 0 || g > 10) { out("-1"); continue; }
            if(g >= users[c].privilege) { out("-1"); continue; }
            User usr; usr.username=u; usr.password=p; usr.name=n; usr.mail=m; usr.privilege=g;
            users[u]=usr; save_users(users); out("0");
        }
        else if(cmd == "login"){
            string u = get_arg("-u");
            string p = get_arg("-p");
            auto it = users.find(u);
            if(it==users.end() || it->second.password!=p || online.count(u)) { out("-1"); }
            else { online.insert(u); out("0"); }
        }
        else if(cmd == "logout"){
            string u = get_arg("-u");
            if(online.count(u)) { online.erase(u); out("0"); }
            else out("-1");
        }
        else if(cmd == "query_profile"){
            string c = get_arg("-c");
            string u = get_arg("-u");
            if(!users.count(c) || !online.count(c) || !users.count(u)) { out("-1"); continue; }
            if(!(users[c].privilege > users[u].privilege || c==u)) { out("-1"); continue; }
            User &x = users[u];
            out(x.username + ' ' + x.name + ' ' + x.mail + ' ' + to_string(x.privilege));
        }
        else if(cmd == "modify_profile"){
            string c = get_arg("-c");
            string u = get_arg("-u");
            if(!users.count(c) || !online.count(c) || !users.count(u)) { out("-1"); continue; }
            if(!(users[c].privilege > users[u].privilege || c==u)) { out("-1"); continue; }
            string np = get_arg("-p");
            string nn = get_arg("-n");
            string nm = get_arg("-m");
            string ngs = get_arg("-g");
            if(!ngs.empty()){
                int ng;
                try{ ng = stoi(ngs);}catch(...){ out("-1"); continue; }
                if(ng >= users[c].privilege) { out("-1"); continue; }
                users[u].privilege = ng;
            }
            if(!np.empty()) users[u].password = np;
            if(!nn.empty()) users[u].name = nn;
            if(!nm.empty()) users[u].mail = nm;
            save_users(users);
            User &x = users[u];
            out(x.username + ' ' + x.name + ' ' + x.mail + ' ' + to_string(x.privilege));
        }
        else if(cmd == "add_train"){
            string id = get_arg("-i");
            if(trains.count(id)) { out("-1"); continue; }
            string nstr = get_arg("-n");
            string mstr = get_arg("-m");
            string sstr = get_arg("-s");
            string pstr = get_arg("-p");
            string xstr = get_arg("-x");
            string tstr = get_arg("-t");
            string ostr = get_arg("-o");
            string dstr = get_arg("-d");
            string ystr = get_arg("-y");
            Train tr; tr.id = id;
            try{
                tr.stationNum = stoi(nstr);
                tr.seatNum = stoi(mstr);
            }catch(...){ out("-1"); continue; }
            if(tr.stationNum < 2) { out("-1"); continue; }
            // stations
            {
                tr.stations.clear();
                size_t pos=0; while(true){
                    size_t q = sstr.find('|', pos);
                    string part = sstr.substr(pos, q==string::npos? string::npos : q-pos);
                    tr.stations.push_back(part);
                    if(q==string::npos) break; pos = q+1;
                }
                if((int)tr.stations.size() != tr.stationNum) { out("-1"); continue; }
            }
            auto parseInts = [&](const string &z, vector<int> &out){
                out.clear(); size_t pos=0; if(z.empty()) return; while(true){ size_t q=z.find('|',pos); string part=z.substr(pos, q==string::npos? string::npos: q-pos); if(!part.empty()) out.push_back(stoi(part)); else out.push_back(0); if(q==string::npos) break; pos=q+1; }
            };
            try{
                parseInts(pstr, tr.prices);
                parseInts(tstr, tr.travel);
                if(ostr == "_") tr.stopover.clear();
                else parseInts(ostr, tr.stopover);
            }catch(...){ out("-1"); continue; }
            if((int)tr.prices.size() != tr.stationNum-1 || (int)tr.travel.size() != tr.stationNum-1) { out("-1"); continue; }
            if(tr.stationNum==2){ if(!tr.stopover.empty()) { out("-1"); continue; } }
            else{ if((int)tr.stopover.size() != tr.stationNum-2) { out("-1"); continue; } }
            // start time
            if(xstr.size()!=5 || xstr[2] != ':'){ out("-1"); continue; }
            try{ tr.startH = stoi(xstr.substr(0,2)); tr.startM = stoi(xstr.substr(3,2)); }catch(...){ out("-1"); continue; }
            if(tr.startH<0||tr.startH>=24||tr.startM<0||tr.startM>=60){ out("-1"); continue; }
            // sale date
            {
                size_t bar = dstr.find('|'); if(bar==string::npos){ out("-1"); continue; }
                string a = dstr.substr(0,bar); string b = dstr.substr(bar+1);
                int da = md_to_day(a); int db = md_to_day(b);
                if(da<0||db<0||da>db){ out("-1"); continue; }
                tr.saleStart = da; tr.saleEnd = db;
            }
            tr.type = ystr.empty()? 'G' : ystr[0];
            tr.released = false;
            trains[id] = tr;
            save_trains(trains);
            out("0");
        }
        else if(cmd == "release_train"){
            string id = get_arg("-i");
            auto it = trains.find(id);
            if(it==trains.end() || it->second.released){ out("-1"); }
            else { it->second.released = true; save_trains(trains); out("0"); }
        }
        else if(cmd == "delete_train"){
            string id = get_arg("-i");
            auto it = trains.find(id);
            if(it==trains.end() || it->second.released){ out("-1"); }
            else { trains.erase(it); save_trains(trains); out("0"); }
        }
        else if(cmd == "query_train"){
            string id = get_arg("-i");
            string d = get_arg("-d");
            auto it = trains.find(id);
            if(it==trains.end()){ out("-1"); continue; }
            int day = md_to_day(d);
            if(day < 0) { out("-1"); continue; }
            Train &tr = it->second;
            if(day < tr.saleStart || day > tr.saleEnd){ out("-1"); continue; }
            // output
            out(tr.id + ' ' + string(1,tr.type));
            int base = day*24*60 + tr.startH*60 + tr.startM; // depart from station 0 at this time
            int n = tr.stationNum;
            vector<int> cumTravel(n,0), cumStop(n,0), cumPrice(n,0);
            for(int i=1;i<n;i++) cumPrice[i] = cumPrice[i-1] + tr.prices[i-1];
            for(int i=1;i<n;i++){
                cumTravel[i] = cumTravel[i-1] + tr.travel[i-1];
                if(i>=2) cumStop[i] = cumStop[i-1] + tr.stopover[i-2];
            }
            for(int i=0;i<n;i++){
                string arr="xx-xx xx:xx", dep="xx-xx xx:xx";
                int seat = (i==n-1? -1 : tr.seatNum);
                if(i==0){
                    dep = fmt_time(base);
                } else if(i==n-1){
                    int at = base + cumTravel[i] + cumStop[i];
                    arr = fmt_time(at);
                } else {
                    int at = base + cumTravel[i] + cumStop[i];
                    int dt = at + tr.stopover[i-1];
                    arr = fmt_time(at);
                    dep = fmt_time(dt);
                }
                string lineOut = tr.stations[i] + ' ' + arr + " -> " + dep + ' ' + to_string(cumPrice[i]) + ' ';
                if(i==n-1) lineOut += 'x'; else lineOut += to_string(seat);
                out(lineOut);
            }
        }
        else if(cmd == "query_ticket"){
            string S = get_arg("-s");
            string T = get_arg("-t");
            string D = get_arg("-d");
            string pflag = get_arg("-p");
            int dayS = md_to_day(D);
            if(dayS < 0){ out("0"); continue; }
            struct R { string id, fromS, toS; int dep, arr, price, seat; };
            vector<R> res;
            for(auto &kv : trains){
                auto &tr = kv.second;
                if(!tr.released) continue; // only after release
                int n = tr.stationNum;
                int si=-1, ti=-1;
                for(int i=0;i<n;i++) if(tr.stations[i]==S){ si=i; break; }
                for(int i=0;i<n;i++) if(tr.stations[i]==T){ ti=i; break; }
                if(si==-1 || ti==-1 || si>=ti) continue;
                // precompute cum arrays
                vector<int> cumTravel(n,0), cumStop(n,0), cumPrice(n,0);
                for(int i=1;i<n;i++) cumPrice[i] = cumPrice[i-1] + tr.prices[i-1];
                for(int i=1;i<n;i++){
                    cumTravel[i] = cumTravel[i-1] + tr.travel[i-1];
                    if(i>=2) cumStop[i] = cumStop[i-1] + tr.stopover[i-2];
                }
                int depart_offset = tr.startH*60 + tr.startM + cumTravel[si] + cumStop[si] + (si>0 ? tr.stopover[si-1] : 0);
                int baseDay = dayS - (depart_offset / (24*60));
                if(baseDay < tr.saleStart || baseDay > tr.saleEnd) continue;
                int base = baseDay*24*60 + tr.startH*60 + tr.startM;
                int depAbs = base + cumTravel[si] + cumStop[si];
                if(si>0) depAbs += tr.stopover[si-1]; // leaving time at si
                int arrAbs = base + cumTravel[ti] + cumStop[ti];
                int price = cumPrice[ti] - cumPrice[si];
                int seat = tr.seatNum;
                res.push_back({tr.id, S, T, depAbs, arrAbs, price, seat});
            }
            if(pflag=="time"){
                sort(res.begin(), res.end(), [&](const R&a,const R&b){
                    long long ta = (long long)a.arr - a.dep;
                    long long tb = (long long)b.arr - b.dep;
                    if(ta!=tb) return ta<tb;
                    if(a.price!=b.price) return a.price<b.price;
                    return a.id < b.id;
                });
            }else{
                sort(res.begin(), res.end(), [&](const R&a,const R&b){
                    if(a.price!=b.price) return a.price<b.price;
                    long long ta = (long long)a.arr - a.dep;
                    long long tb = (long long)b.arr - b.dep;
                    if(ta!=tb) return ta<tb;
                    return a.id < b.id;
                });
            }
            out(to_string((int)res.size()));
            for(auto &r: res){
                string lineOut = r.id + ' ' + r.fromS + ' ' + fmt_time(r.dep) + " -> " + r.toS + ' ' + fmt_time(r.arr) + ' ' + to_string(r.price) + ' ' + to_string(r.seat);
                out(lineOut);
            }
        }
        else if(cmd == "query_transfer"){
            string S = get_arg("-s");
            string T = get_arg("-t");
            string D = get_arg("-d");
            string pflag = get_arg("-p");
            int dayS = md_to_day(D);
            if(dayS < 0){ out("0"); continue; }
            struct R { string id1,id2, s1, x, t2; int d1,a1,d2,a2, price, seat; long long key_time, key_cost; } best;
            bool has=false;
            // Precompute station indices maps for faster lookup
            for(auto &akv : trains){
                auto &A = akv.second; if(!A.released) continue;
                int nA=A.stationNum; int si=-1;
                for(int i=0;i<nA;i++) if(A.stations[i]==S){ si=i; break; }
                if(si==-1) continue;
                // cum arrays A
                vector<int> cumTA(nA,0), cumSA(nA,0), cumPA(nA,0);
                for(int i=1;i<nA;i++) cumPA[i]=cumPA[i-1]+A.prices[i-1];
                for(int i=1;i<nA;i++){ cumTA[i]=cumTA[i-1]+A.travel[i-1]; if(i>=2) cumSA[i]=cumSA[i-1]+A.stopover[i-2]; }
                int offDepA = A.startH*60 + A.startM + cumTA[si] + cumSA[si] + (si>0?A.stopover[si-1]:0);
                int baseA = dayS - (offDepA / (24*60));
                if(baseA < A.saleStart || baseA > A.saleEnd) continue;
                int baseAbsA = baseA*24*60 + A.startH*60 + A.startM;
                int depAbsA = baseAbsA + cumTA[si] + cumSA[si] + (si>0?A.stopover[si-1]:0);
                for(int xi=si+1; xi<nA; ++xi){
                    // arrival at X on A
                    int arrAX = baseAbsA + cumTA[xi] + cumSA[xi];
                    int priceA = cumPA[xi] - cumPA[si];
                    // search B trains
                    for(auto &bkv : trains){
                        auto &B = bkv.second; if(!B.released) continue; if(B.id==A.id) continue;
                        int nB=B.stationNum; int xj=-1, tj=-1;
                        for(int j=0;j<nB;j++) if(B.stations[j]==A.stations[xi]){ xj=j; break; }
                        if(xj==-1) continue;
                        for(int j=0;j<nB;j++) if(B.stations[j]==T){ tj=j; break; }
                        if(tj==-1 || xj>=tj) continue;
                        // cum arrays B
                        vector<int> cumTB(nB,0), cumSB(nB,0), cumPB(nB,0);
                        for(int j=1;j<nB;j++) cumPB[j]=cumPB[j-1]+B.prices[j-1];
                        for(int j=1;j<nB;j++){ cumTB[j]=cumTB[j-1]+B.travel[j-1]; if(j>=2) cumSB[j]=cumSB[j-1]+B.stopover[j-2]; }
                        int offDepBX = B.startH*60 + B.startM + cumTB[xj] + cumSB[xj] + (xj>0?B.stopover[xj-1]:0);
                        long long diff = (long long)arrAX - (long long)offDepBX;
                        long long baseB = diff>0 ? ( (diff + 1440 - 1)/1440 ) : 0;
                        int baseBi = (int)baseB;
                        if(baseBi < B.saleStart) baseBi = B.saleStart;
                        if(baseBi > B.saleEnd) continue;
                        int baseAbsB = baseBi*24*60 + B.startH*60 + B.startM;
                        int depAbsB = baseAbsB + cumTB[xj] + cumSB[xj] + (xj>0?B.stopover[xj-1]:0);
                        if(depAbsB < arrAX) continue; // ensure depart after arrival
                        int arrBT = baseAbsB + cumTB[tj] + cumSB[tj];
                        int priceB = cumPB[tj] - cumPB[xj];
                        int seat = min(A.seatNum, B.seatNum);
                        long long totalTime = (long long)(arrAX - depAbsA) + (long long)(arrBT - depAbsB) + (long long)(depAbsB - arrAX);
                        int totalPrice = priceA + priceB;
                        bool better=false;
                        if(!has){ better=true; }
                        else if(pflag=="time"){
                            if(totalTime < best.key_time) better=true;
                            else if(totalTime==best.key_time && totalPrice < (int)best.key_cost) better=true;
                        } else {
                            if(totalPrice < (int)best.key_cost) better=true;
                            else if(totalPrice==(int)best.key_cost && totalTime < best.key_time) better=true;
                        }
                        if(better){
                            has=true;
                            best.id1=A.id; best.id2=B.id; best.s1=S; best.x=A.stations[xi]; best.t2=T;
                            best.d1=depAbsA; best.a1=arrAX; best.d2=depAbsB; best.a2=arrBT;
                            best.price=totalPrice; best.seat=seat; best.key_time=totalTime; best.key_cost=totalPrice;
                        }
                    }
                }
            }
            if(!has){ out("0"); }
            else{
                string line1 = best.id1 + ' ' + best.s1 + ' ' + fmt_time(best.d1) + " -> " + best.x + ' ' + fmt_time(best.a1) + ' ' + to_string(best.price - 0) + ' ' + to_string(best.seat);
                // For output, the price on first line should be price from s to x; reconstruct
                // Recompute price1 by querying A quickly
                int price1=0; int price2=best.price;
                // As we didn't store it, recompute by scanning again (small scale)
                {
                    auto itA = trains.find(best.id1);
                    if(itA!=trains.end()){
                        auto &A=itA->second; int si=-1, xi=-1; for(int i=0;i<A.stationNum;i++){ if(A.stations[i]==best.s1){si=i;} if(A.stations[i]==best.x){xi=i;} }
                        if(si!=-1 && xi!=-1){ int sum=0; for(int i=si;i<xi;i++) sum+=A.prices[i]; price1=sum; }
                    }
                }
                price2 -= price1;
                string line1fix = best.id1 + ' ' + best.s1 + ' ' + fmt_time(best.d1) + " -> " + best.x + ' ' + fmt_time(best.a1) + ' ' + to_string(price1) + ' ' + to_string(best.seat);
                string line2 = best.id2 + ' ' + best.x + ' ' + fmt_time(best.d2) + " -> " + best.t2 + ' ' + fmt_time(best.a2) + ' ' + to_string(price2) + ' ' + to_string(best.seat);
                out(line1fix);
                out(line2);
            }
        }
        else if(cmd == "buy_ticket" || cmd == "query_order" || cmd == "refund_ticket"){
            out("-1");
        }
        else {
            out("-1");
        }
    }
    return 0;
}
