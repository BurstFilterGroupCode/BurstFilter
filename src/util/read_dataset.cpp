/*
 * read_dataset.cpp
 *
 * 数据集的读取，为了方便调参，所有数据集统一将密度线性调整到与CAIDA相同
 * 调用init_dataset(n)即可初始化数据集
 */

#include "read_dataset.h"
#include <bits/stdc++.h>
#include <arpa/inet.h>
#include "BOBHash32.h"
using namespace std;

// 使用下面的define改变数据集类型
/* dataset: CAIDA, SX_STKOVFL, Yahoo_G4, Yahoo_G6, WATER_TEMP, WATER_TEMP_MIXED, JITTER_MIXED_DATA*/
// #define JITTER_MIXED_DATA
#define CAIDA
// #define WATER_TEMP_MIXED
// #define Yahoo_G4

vector<DS_Pack> DS;

#pragma pack(push)
#pragma pack(1)
struct Trace {
  uint8_t bytes[13];
  uint16_t size;
  double tstamp;
}trace;
#pragma pack(pop)

#pragma pack(push, 1)
struct Mixed_Trace {
  uint8_t bytes[13];
  double tstamp;
};
struct Pkt {
  uint32_t srcip;
  uint32_t dstip;
  uint16_t srcpt;
  uint16_t dstpt;
  uint8_t protocol;
  double timestamp;
};
#pragma pack(pop)

inline int cmp(const DS_Pack &a, const DS_Pack &b) {
    return a.tm < b.tm;
}

static const double caida_density = 0.00000215474851131439;

// sample input: 0429.07:59:03.068
inline double get_time(string s) {
    int h = (s[5] - '0') * 10 + s[6] - '0';
    int m = (s[8] - '0') * 10 + s[9] - '0';
    double sec = (s[11] - '0') * 10 + s[12] + (s[13] - '0') * 0.1 + (s[14] - '0') * 0.01 + (s[15] - '0') * 0.01;
    return h * 3600 + m * 60 + sec;
}

vector<pair<int, double> > Correct_Report;
vector<pair<int, double> > Correct_Report_End;

void init_dataset(int n){
    // CAIDA
    DS.clear();
    BOBHash32* bobhash = new BOBHash32(5);

#ifdef CAIDA

    ifstream fin("../dataset/CAIDA_2018/00.dat");
    while(n--) {
        struct Trace trace;
        fin.read((char *)&trace, 23);
        //string flow((char *)trace.bytes, 13);
        DS_Pack pk;
        pk.tm = trace.tstamp;
        pk.id = bobhash->run((char *)trace.bytes, 13);
        //cout<<pk.id<<endl;
        pk.pksz = trace.size;
        DS.push_back(pk);
    }
    sort(DS.begin(), DS.end(), cmp);
    double t = DS[0].tm;
    for(int i = 0; i < (int)DS.size(); i++) DS[i].tm -= t;

#endif

#ifdef SX_STKOVFL

    ifstream fin("../dataset/sx-stackoverflow.txt");
    while(n--) {
        int st, ed;
        long long tm;
        fin>>st>>ed>>tm;
        pair<int, int> p = make_pair(st, ed);
        DS_Pack pk;
        pk.tm = tm;
        pk.id = bobhash->run((char *)&st, sizeof(int));
        DS.push_back(pk);
    }
    sort(DS.begin(), DS.end(), cmp);
    double t = DS[0].tm;
    for(int i = 0; i < (int)DS.size(); i++) DS[i].tm -= t;
    double density = (DS[DS.size()-1].tm - DS[0].tm) / DS.size();
    for(int i = 0; i < (int)DS.size(); i++) DS[i].tm *= caida_density / density;

#endif

#ifdef Yahoo_G4
    ifstream fin("../dataset/Yahoo/ft-v05.2008-04-29.080000-0700.permuted");
    string sss;
    getline(fin, sss);
    double lim = 10.0;
    while(n--) {
        string Start, End, Sif, SrcIPaddress, SrcP, DIf, DstIPaddress, DstP, P, Fl, Pkts, Octets;
        fin>>Start>>End>>Sif>>SrcIPaddress>>SrcP>>DIf>>DstIPaddress>>DstP>>P>>Fl>>Pkts>>Octets;
        DS_Pack pk;
        pk.tm = get_time(Start);
        //if(n <= 10)cout<<Start<<endl;
        string srcdst = SrcIPaddress+":"+SrcP;
        char str[100];
        for(int i = 0 ; i < (int)srcdst.size(); i++) str[i]=srcdst[i];
        str[srcdst.size()]='\0';
        pk.id = bobhash->run(str, srcdst.size());
        pk.delay = get_time(End) - get_time(Start);
        DS.push_back(pk);
        // sample_line
        // 0429.07:59:03.068       0429.07:59:03.068       24      246.136.192.213 48275   83      118.24.101.126  80      6       0       1       354
        // now we need to process these lines
    }
    sort(DS.begin(), DS.end(), cmp);
    double t = DS[0].tm;
    for(int i = 0; i < (int)DS.size(); i++) DS[i].tm -= t;
    double density = (DS[DS.size()-1].tm - DS[0].tm) / DS.size();
    for(int i = 0; i < (int)DS.size(); i++) DS[i].tm *= caida_density / density;
    map<int,int> sum, cnt, f;
    for(auto &pk:DS) {
        sum[pk.id] += pk.delay;
        cnt[pk.id]++;
    }
    srand(42);
    for(auto &pk:cnt)
    {
        double avg = (double)sum[pk.first] / pk.second;
        double new_avg = (double)rand() / RAND_MAX * 0.2 * lim + 0.9 * lim;
        f[pk.first] = new_avg / avg;
    }
    for(auto &pk:DS) {
        pk.delay *= f[pk.id];
    }
#endif

#ifdef Yahoo_G6
    ifsream fin_1("../../R6/ydata-fp-td-clicks-v1_0.20090501");

    int cnt = 0;
    while(cnt < n) {
        string s;
        fin_1>>s;
        if(s[0] != '|' || s == "|user") continue;
        DS_Pack pk;
        pk.id = 0;
        for(int i = 1; i < (int)s.size(); i++) {
            pk.id = 10 * pk.id + s[i] - '0';
        }
        //cout<<pk.id<<endl;
        pk.tm = cnt;
        DS.push_back(pk);
        cnt++;
    }
    sort(DS.begin(), DS.end(), cmp);
    double t = DS[0].tm;
    for(int i = 0; i < (int)DS.size(); i++) DS[i].tm -= t;
    double density = (DS[DS.size()-1].tm - DS[0].tm) / DS.size();
    for(int i = 0; i < (int)DS.size(); i++) DS[i].tm *= caida_density / density;
#endif

#ifdef WATER_TEMP
    vector<double> val;
    string s;
    ifstream fin("../../water_temp.json");
    getline(fin, s);
    string tmp = "";
    int isinbrack = 0;
    for(int i = 0; i < (int)s.size(); i++) {
        if(s[i]=='}') {
            assert(isinbrack);
            char tmps[100];
            int cntmh=0,stpos=0;
            for(int i = 0; i < (int)tmp.size(); i++) {
                tmps[i]=tmp[i];
                if(tmps[i]=='\"') cntmh++;
                if(cntmh==7) stpos=i+1,cntmh=100000;
            }
            tmps[tmp.size()]='\0';
            double t;
            sscanf(tmps+stpos, "%lf", &t);
            val.push_back(t);
        }
        if(isinbrack) {
            tmp+=s[i];
        }
        if(s[i]=='{') {
            isinbrack = 1;
            tmp="";
        }
    }
    // cout<<val.size()<<endl; 43318
    DS.clear();
    for(int i = 0; i < 433; i++) {
        for(int j = 0; j < 100; j++) {
            DS.push_back(DS_Pack{(double)j,(unsigned)i+1,0,val[i*100+j]});
        }
    }
    sort(DS.begin(), DS.end(), cmp);
    double t = DS[0].tm;
    for(int i = 0; i < (int)DS.size(); i++) DS[i].tm -= t;
    double density = (DS[DS.size()-1].tm - DS[0].tm) / DS.size();
    for(int i = 0; i < (int)DS.size(); i++) DS[i].tm *= caida_density / density;
    /*for(int i = 1; i < 20; i++) {
        printf("%.10f\n", val[i]);
    }*/
#endif

#ifdef WATER_TEMP_MIXED
    vector<double> val;
    string s;
    ifstream fin("../dataset/water_temp.json");
    getline(fin, s);
    string tmp = "";
    int isinbrack = 0;
    for(int i = 0; i < (int)s.size(); i++) {
        if(s[i]=='}') {
            assert(isinbrack);
            char tmps[100];
            int cntmh=0,stpos=0;
            for(int i = 0; i < (int)tmp.size(); i++) {
                tmps[i]=tmp[i];
                if(tmps[i]=='\"') cntmh++;
                if(cntmh==7) stpos=i+1,cntmh=100000;
            }
            tmps[tmp.size()]='\0';
            double t;
            sscanf(tmps+stpos, "%lf", &t);
            if (t > 0)
                val.push_back(t);
        }
        if(isinbrack) {
            tmp+=s[i];
        }
        if(s[i]=='{') {
            isinbrack = 1;
            tmp="";
        }
    }
    // cout<<val.size()<<endl; 43318
    vector<queue<double> > Flows;
    for(int i = 0; i < 4000; i++) {
        int stpos = rand() % val.size();
        queue<double> tmp;
        for(int j = stpos; j < min((int)val.size(), stpos + 4000); j++) {
            tmp.push(val[j]);
        }
        Flows.push_back(tmp);
    }
    set<int> pos;
    for(int i = 0; i < 4000; i++) {
        pos.insert(i);
    }
    while(pos.size()) {
        int x = rand() % 4000;
        while(pos.count(x) == 0) {
            x = rand() % 4000;
        }
        static int tot = 0;
        DS.push_back(DS_Pack{(double)tot, (unsigned)x+1, 0, Flows[x].front()});
        tot++;
        Flows[x].pop();
        if(Flows[x].size() == 0) {
            pos.erase(x);
        }
    }
    sort(DS.begin(), DS.end(), cmp);
    double t = DS[0].tm;
    for(int i = 0; i < (int)DS.size(); i++) DS[i].tm -= t;
    double density = (DS[DS.size()-1].tm - DS[0].tm) / DS.size();
    for(int i = 0; i < (int)DS.size(); i++) DS[i].tm *= caida_density / density;
#endif


#ifdef JITTER_MIXED_DATA
    ifstream fin("../dataset/jitter_dataset/wechat_uniform_1.dat");    
    // ifstream fin("../../jitter_dataset/wechat_1.dat");    
    // ifstream fin("../../jitter_dataset/caida_wechat.dat");    
    ifstream fcor("../dataset/jitter_dataset/ground_truth_uniform_1.txt");
    // ifstream fcor("../../jitter_dataset/ground_truth_1.txt");
    // ifstream fcor("../../jitter_dataset/ground_truth.txt");
    set<int> flid;
    int tmp;
    while(fcor>>tmp) {
        Pkt pk;
        fcor>>pk.srcip>>pk.dstip>>pk.srcpt>>pk.dstpt;
        /*pk.srcip = htonl(pk.srcip);
        pk.dstip = htonl(pk.dstip);
        pk.srcpt = htons(pk.srcpt);
        pk.dstpt = htons(pk.dstpt);*/
        double ta,tb;
        fcor>>ta>>tb;
        flid.insert(bobhash->run((char *)&pk.srcip, 12));
        if(tmp == 1) {
            fcor>>ta>>tb;
            Correct_Report.push_back(make_pair(bobhash->run((char *)&pk.srcip, 12), ta - 1521118800));
            Correct_Report_End.push_back(make_pair(bobhash->run((char *)&pk.srcip, 12), tb - 1521118800));
        }
    }
    int total_package_count = 0;
    int flag = 1;
    while(fin.eof() == 0) {
        struct Mixed_Trace trace;
        fin.read((char *)&trace, 21);
        if(fin.eof()) break;
        ++total_package_count;
        //string flow((char *)trace.bytes, 13);
        DS_Pack pk;
        pk.tm = trace.tstamp;
        static int tot = 0;
        if(pk.tm > 1521118800) pk.tm -= 1521118800;
        //if(++tot % 1000000 == 0) printf("%.10f\n", pk.tm);
        pk.id = bobhash->run((char *)trace.bytes, 12);
        //cout<<pk.id<<endl;
        if(flid.count(pk.id) == 0) {
            if (flag) {
                cout << "Debug: " << total_package_count << endl;
                printf ("%x\n", *trace.bytes);
                flag = 0;
            }
            continue;
        }
        DS.push_back(pk);
        /*static int count = 0;
        if(++count % 100000 == 0) {
            cout<<DS.size()<<endl;
        }*/
    }
    cout << "Total Package Count: " << total_package_count << endl;
    cout<<"DS size: "<<DS.size()<<endl;
    sort(DS.begin(), DS.end(), cmp);
    double t = DS[0].tm;
    for(int i = 0; i < (int)DS.size(); i++) DS[i].tm -= t;
    for(int i = 0; i < (int)Correct_Report.size(); i++) {
        Correct_Report[i].second -= t;
        Correct_Report_End[i].second -= t;
    }
#endif
}