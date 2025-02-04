/*
 * run_strawman.cpp
 * 
 * usage: ./run_strawman p stdLim threshold flush_dt
 * 分别表示比例系数，阈值，包个数（默认为1），刷新时间间隔
 * 
 * 运行strawman算法，将准确率输出到 tmp/strawman.res，内存占用输出到 tmp/strawman.res
 * 使用定期刷新unordered_map，内存由刷新频率决定
 */

#include <bits/stdc++.h>
using namespace std;

#include "util/read_dataset.h"
#include "sketch/newsketch/param.h"
#include "read_correct.h"

// set<id_type> sa,sb;
// map<id_type, int> ma,mb;
// int sumerr;
// double terr;
// map<id_type, double> ta,tb;

extern vector<Burstelem> Tc;
extern vector<DS_Pack> DS;

// int cntA, cntB, cntAB;

// bool cmp(const Burstelem &A, const Burstelem &B) {
//     return A.t<B.t;
// }

// void Compare(vector<Burstelem> &A, vector<Burstelem> &B) {
//     if(A.size() < B.size())swap(A, B);
//     sa.clear();sb.clear();
//     ma.clear();
//     mb.clear();
//     ta.clear();
//     tb.clear();
//     cntA = cntB = cntAB = 0;
//     sumerr = terr = 0;
//     cntA=A.size();
//     cntB=B.size();
//     map<id_type, vector<Burstelem> > mp;
//     sort(A.begin(),A.end(),cmp);
//     for(auto v : A) {
//         mp[v.id].push_back(v);
//     }
//     for(auto v : B) {
//         vector<Burstelem> &B = mp[v.id];
//         if(B.size() == 0) continue;
//         auto it = lower_bound(B.begin(), B.end(), v, cmp);
//         auto get = it;
//         if(it == B.end()) {
//             get = --it;
//         }
//         else if(it != B.begin()) {
//             auto itp = it;
//             itp--;
//             double at = v.t - itp->t;
//             double bt = it->t - v.t;
//             if(at < bt) get = itp;
//         }
//         double dt = fabs(v.t-get->t);
//         if(dt > chkLim) continue;
//         cntAB++;
//         terr += dt;
//         sumerr += abs(v.cnt - get->cnt);
//     }
// }

const double myp=0.9;
// const double p=0.5;
const double Lim=0.005;
const double ouv = 0.1;
double flush_intv = 0.001;

// // 算法主题，刷新需要手动调用flush进行
// struct StrawmanSketch {
//     unordered_map<id_type,double> lsttm;
//     unordered_map<id_type,double> val;
//     unordered_map<id_type,int> cnt;
//     unordered_map<id_type,bool> vised;
//     int max_sz;
//     StrawmanSketch() {max_sz = 0;}
//     vector<Burstelem> Burst;
//     void reportBurst(id_type id, int cnt, double tm) {
//         //cout<<id<<' '<<cnt<<' '<<tm<<endl;
//         if(cnt>=threshold) {
//             Burst.push_back((Burstelem){id, cnt, tm});
//         }
//     }
//     void clearBurst() {Burst.clear();}
//     void clearall() {
//         clearBurst();
//         lsttm.clear();
//         val.clear();
//         cnt.clear();
//     }
//     void insert(id_type id, double tm) {
//         if(lsttm[id]<=0.000000001) {
//             //lsttm[id]=-100000000.0;
//             val[id]=-outv;
//             cnt[id]=0;
//         }
//         //cout<<id<<' '<<tm<<' '<<val[id]<<' '<<lsttm[id]<<endl;
//         double lstval = val[id];
//         val[id] = val[id] * (1.0 - p) + tm * p;
//         val[id] = max(val[id], tm-outv);
//         vised[id] = 1;
        
//         if(tm - val[id] < stdLim) {
//             cnt[id]++;
//         }
//         else if(lsttm[id] - lstval < stdLim) {
//             reportBurst(id, cnt[id], lsttm[id]);
//             cnt[id] = 0;
//         }
//         lsttm[id] = tm;

//         max_sz = max(max_sz, (int)vised.size());
//     }
//     size_t memory_used() {
//         size_t ans = 0;
//         ans = sizeof(int) + sizeof(double) + sizeof(double) + sizeof(id_type);
//         ans = ans * val.size();
//         return ans;
//     }
//     void flush() {
//         //puts("Start Flushing");
//         //cout<<vised.size()<<endl;
//         vector<id_type> Rm;
//         for(unordered_map<id_type,bool>::iterator it = vised.begin(); it != vised.end(); it++) {
//             if(it->second == 0) {
//                 Rm.push_back(it->first);
//             }
//             it->second = 0;
//         }
//         //cout<<Rm.size()<<endl;
//         for(int i = 0; i < (int) Rm.size(); i++) {
//             //cout<<"erasing "<<Rm[i]<<endl;
//             id_type rmid = Rm[i];
            
//             assert(lsttm.find(rmid) != lsttm.end());
//             assert(val.find(rmid) != val.end());
//             assert(cnt.find(rmid) != cnt.end());
//             assert(vised.find(rmid) != vised.end());

//             lsttm.erase(lsttm.find(rmid));
//             val.erase(val.find(rmid));
//             cnt.erase(cnt.find(rmid));
//             vised.erase(vised.find(rmid));
//         }
//         //cout<<vised.size()<<endl;
//         //puts("End Flushing");
//     }
//     int mem_used() {
//         int tmp = sizeof(int) + sizeof(double) + sizeof(double) + sizeof(id_type) + 1;
//         return tmp * max_sz;
//     }
// };

namespace Strawman_test {
    unordered_map<int, double> val;
    unordered_map<int, bool> used;
    unordered_map<int, double> last_time;
    int mxcnt;
    int maxsz;
    double lstflush;
    vector<pair<int, double> > burstid;
    void init (int _maxsz = -1) {
        maxsz = _maxsz;
        mxcnt = 0;
        lstflush = 0;
        val.clear ();
        used.clear ();
        burstid.clear ();
    }
    void flush(double now) {
        for (auto it = used.begin (); it != used.end (); ) {
            auto &pii = *it;
            if (pii.second == 0 || last_time [pii.first] < now - ouv) {
                val.erase (pii.first);
                last_time.erase (pii.first);
                auto itt = next (it);
                used.erase (it);
                it = itt;
            }
            else {
                pii.second = 0;
            }
        }
    }
    void runtest(int ground_truth) {
        mxcnt = 0;
        for(int i = 0; i < (int) DS.size(); i++) {
            if (DS [i].tm - lstflush > flush_intv) {
                if (ground_truth == 0)
                    flush (DS [i].tm);
                lstflush = DS [i].tm;
            }
            if(val.count(DS[i].id) == 0) {
                if ((int) val.size () == maxsz) continue;
                used[DS[i].id] = 1;
                last_time [DS [i].id] = DS [i].tm;
                val[DS[i].id] = DS[i].tm - ouv;
                if (DS[i].tm - val[DS[i].id] < Lim) {
                    burstid.push_back(make_pair(DS[i].id, DS[i].tm));
                }
            }
            else {
                used[DS[i].id] = 1;
                if (last_time [DS [i].id] < DS [i].tm - ouv) {
                    last_time [DS [i].id] = DS [i].tm;
                    val[DS[i].id] = DS[i].tm - ouv;
                    if (DS[i].tm - val[DS[i].id] < Lim) {
                       burstid.push_back(make_pair(DS[i].id, DS[i].tm));
                    }
                    continue;
                }
                double oldval = last_time [DS[i].id] - val[DS[i].id];
                val[DS[i].id] = val[DS[i].id] * myp + DS[i].tm * (1.0 - myp);
                last_time [DS[i].id] = DS[i].tm;
                if(oldval >= Lim && DS [i].tm - val[DS[i].id] < Lim) {
                    burstid.push_back(make_pair(DS[i].id, DS[i].tm));
                }
            }
            mxcnt = max(mxcnt, (int)val.size());
        }
        cerr << "Max count: " << mxcnt << endl;
    }
};

double compare(vector<pair<int, double> > &ours, vector <Burstelem> &ans, FILE *file = NULL, int memory = 0, double mops = 0) {
    map<int, double> Mp;
    set <int> app;
    for (auto &jit : ours) {
        if (Mp.find (jit.first) == Mp.end ())
            Mp [jit.first] = jit.second;
    }
    double allt = DS[DS.size() - 1].tm;
    //printf("%.10f\n", allt);
    double sumerr = 0;
    int TP = 0, FP = 0, FN = 0;
    for (auto &v : ans) {
        if (app.find (v.id) != app.end ()) continue;
        app.insert (v.id);
        if (Mp.find (v.id) == Mp.end ()) {
            ++FN;
        }
        else {
            ++TP;
            sumerr += fabs (v.t - Mp [v.id]);
        }
    }
    FP = Mp.size () - TP;
    sumerr += (FN + FP) * allt;
    double R = (double) TP / (TP + FN), P = (double) TP / (TP + FP), F1 = 2 * R * P / (R + P);
    printf ("TP: %d, FP: %d, FN: %d\n", TP, FP, FN);
    printf ("Recall: %.6lf%%, Precision: %.6lf%%, F1 Score: %.6lf%%\n", R * 100, P * 100, F1 * 100);
    if (file != NULL) {
        fprintf (file, "%d,%d,%d,%d,%d,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf\n",
            memory, (int) Mp.size (), TP, FP, FN, F1 * 100, P * 100, R * 100, sumerr / (TP + FN + FP), mops);
    }
    return sumerr / TP / allt;
}

FILE *open_print_header (const char *name) {
    FILE *file = fopen (name, "w");
    assert (file != NULL);
    fprintf (file, "Memory,Report_Count,TP,FP,FN,F1_Score,Precision,Recall,Delta,MOPS\n");
    return file;
}

void test_strm () {
    FILE *file = open_print_header ("/root/burst/netsketch_test/result/main_strawman.csv");
    flush_intv = 0.0005;
    for (int sz = 250; sz <= 4000; sz <<= 1) {
        flush_intv *= 2;
        int test_cycles = 10;
        timespec time1, time2;
        double mops = 0;
        for (int c = 0; c < test_cycles; ++c) {
            Strawman_test::init (sz);
            long long resns = 0;
            clock_gettime (CLOCK_MONOTONIC, &time1); 
            Strawman_test::runtest (0);
            clock_gettime (CLOCK_MONOTONIC, &time2);
            resns += (long long) (time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec); 
            double test_mops = (double) 1000.0 * DS.size () / (double) resns;
            mops += (test_mops - mops) / (c + 1);
        }
        compare (Strawman_test::burstid, Tc, file, sz * 13, mops);
    }
    fclose (file);
    return;
}



int main(int argc, char **argv) {
    const int N = 10000000;
    init_dataset(N);
    read_correct();
    test_strm ();
}