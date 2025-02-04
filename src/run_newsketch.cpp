/*
    run_newsketch.cpp

    tests the result of the newsketch
    requires: ./tmp/correct.txt - the correct burst

    usage: ./run_newsketch memsz(kb) mem_part1/memsz p(ratio) stdlim(burstlim) weaklim threshold hash2
    example: ./run_newsketch 10 0.8 0.1 0.1 0.05 50 1 

*/

#include "util/read_dataset.h"
#include "read_correct.h"
#include "sketch/newsketch/newsketch.h"
#include "sketch/newsketch/param.h"
#include "util/BOBHash32.h"
#include <bits/stdc++.h>
#include "util/time.h"
using namespace std;

unordered_map<string, int> sz;
unordered_map<string, double> tbeg;
unordered_map<string, double> tend;

int cntA,cntB,cntAB;

set<id_type> sa,sb;
map<id_type, int> ma,mb;
int sumerr;
double terr;
map<id_type, double> ta,tb;

// void Compare0(const vector<Burstelem> &A, const vector<Burstelem> &B) {
//     for(auto pii : A) {
//         if(sa.count(pii.id)) puts("Crash");
//         sa.insert(pii.id);
//         ma[pii.id] = pii.cnt;
//         ta[pii.id] = pii.t;
//     }
//     for(auto pii : B) {
//         if(sb.count(pii.id)) puts("Crash");
//         sb.insert(pii.id);
//         mb[pii.id] = pii.cnt;
//         tb[pii.id] = pii.t;
//     }

//     cntA += sa.size();
//     cntB += sb.size();
//     for(auto v : sa) {
//         if(sb.count(v)) {
//             cntAB++;
//             sumerr += abs(ma[v] - mb[v]);
//             terr += fabs(ta[v] - tb[v]);
//         }
//     }
// }


bool cmp(const Burstelem &A, const Burstelem &B) {
    return A.t<B.t;
}

void Compare(vector<Burstelem> &A, vector<Burstelem> &B) {
    if(A.size() < B.size()) swap(A, B);
    sa.clear();sb.clear();
    ma.clear();
    mb.clear();
    ta.clear();
    tb.clear();
    cntA = cntB = cntAB = 0;
    sumerr = terr = 0;
    cntA=A.size();
    cntB=B.size();
    map<id_type, vector<Burstelem> > mp;
    sort(A.begin(),A.end(),cmp);
    for(auto v : A) {
        mp[v.id].push_back(v);
    }
    for(auto v : B) {
        vector<Burstelem> &B = mp[v.id];
        if(B.size() == 0) continue;
        auto it = lower_bound(B.begin(), B.end(), v, cmp);
        auto get = it;
        if(it == B.end()) {
            get = --it;
        }
        else if(it != B.begin()) {
            auto itp = it;
            itp--;
            double at = v.t - itp->t;
            double bt = it->t - v.t;
            if(at < bt) get = itp;
        }
        double dt = fabs(v.t-get->t);
        if(dt > chklim) continue;
        cntAB++;
        terr += dt;
        sumerr += abs(v.cnt - get->cnt);
    }
}

/*
struct Param {
    double _lim;
    double _weaklim;
    double _p;
    double _outv;
    int _threshold;
    double _chklim;
};*/
//需要调的参

extern vector<Burstelem> Tc;
extern vector<DS_Pack> DS;


#include <sys/time.h>

// long long time_ms() {
//     struct timeval tv;
//     long long mstime;
//     gettimeofday(&tv, NULL);
//     mstime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
//     return mstime;
// }


double compare(vector<Burstelem> &ours, vector<Burstelem> &ans, FILE *file = NULL, int memory = 0, double mops = 0) {
    map<int, double> Mp;
    set <int> app;
    for (auto &jit : ours) {
        if (Mp.find (jit.id) == Mp.end ())
            Mp [jit.id] = jit.t;
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
        printf("ready to write results to file\n");
        printf("ready write %d,%d,%d,%d,%d,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf\n",
            memory, (int) Mp.size (), TP, FP, FN, F1 * 100, P * 100, R * 100, sumerr / (TP + FN + FP), mops);
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

void test_res (double rt) {
    FILE *file = open_print_header ("../netsketch_test/result/main_ours.csv");
    for (int sz = 1024; sz <= 131072; sz <<= 1) {
        int test_cycles = 10;
        timespec time1, time2;
        double mops = 0;
        for (int c = 0; c < test_cycles; ++c) {
            NewSketch T(sz * rt, sz * (1 - rt));
            long long resns = 0;
            for (int i = 0; i < (int) DS.size(); i++) {
                // cerr << sz << ' ' << c << ' ' << i << endl;
                clock_gettime (CLOCK_MONOTONIC, &time1); 
                T.main_insert(DS[i].id, DS[i].tm, 0.0);        
                clock_gettime (CLOCK_MONOTONIC, &time2);
                resns += (long long) (time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec); 
            }
            double test_mops = (double) 1000.0 * DS.size () / (double) resns;
            mops += (test_mops - mops) / (c + 1);
            if (c == test_cycles - 1) {
                compare (T.Burst, Tc, file, sz, mops);
            }
        }
        // compare (skt.J, file, sz, mops);
    }
    fclose (file);
    return;
}

int main(int argc, char **argv) {
    struct Param P = Param{0.005, 0.01, 0.1, 0.5, 1, 0.5, 8*1024, 8*1024, 1};
    // struct Param P = Param{0.005, 0.1, 0.5, 50, 0.5, 8*1024, 8*1024};
    if(argc == 2) {
        int SZ;
        sscanf(argv[1], "%d", &SZ);
        P.Size_stage1 = SZ * 1024 * 0.8;
        P.Size_stage2 = SZ * 1024 * 0.2;
    }
    if(argc > 2) {
        int SZ;
        double rt = 0;
        sscanf(argv[1], "%d", &SZ);
        sscanf(argv[2], "%lf", &rt);
        P.Size_stage1 = SZ * 1024 * rt;
        P.Size_stage2 = SZ * 1024 * (1.0 - rt);
    }
    //printf("%d %d\n", P.Size_stage1, P.Size_stage2);
    if(argc > 3) {
        sscanf(argv[3], "%lf", &P._p);
    }
    //printf("%lf\n", P._p);
    if(argc > 5) {
        sscanf(argv[4], "%lf", &P._lim);
        sscanf(argv[5], "%lf", &P._weaklim);
    }
    //printf("%lf\n", P._lim);
    //printf("%lf\n", P._weaklim);
    if(argc > 6) {
        sscanf(argv[6], "%d", &P._threshold);
    }
    if(argc > 7) {
        sscanf(argv[7], "%d", &P._hash2);
    }
    //printf("%d\n", P._threshold);
    load_param(P);
    const int N = 10000000;
    init_dataset(N);
    read_correct();
    // test_res (0.8);
    long long sttime = time_ms();
    NewSketch T(P.Size_stage1, P.Size_stage2);
    for(int i=0;i<N;i++) {
        T.main_insert(DS[i].id, DS[i].tm, 0.0);
    }
    FILE *cachelog = fopen("tmp/newsketch_cache.log", "w");
    for(int i = 0; i < (int)T.Log.size(); i++) {
        switch(T.Log[i].tp) {
            case INSERT:
                fprintf(cachelog, "insert ");
                break;
            case EVICT:
                fprintf(cachelog, "evict ");
                break;
            case VISIT:
                fprintf(cachelog, "visit ");
                break;
        }
        fprintf(cachelog, "%d\n", T.Log[i].id);
    }
    fclose(cachelog);
    long long edtime = time_ms();
    cout<<T.Burst.size()<<' '<<Tc.size()<<endl;
    //puts("end of reading");
    Compare(T.Burst, Tc);

    //cout<<cntA<<' '<<cntB<<' '<<cntAB<<endl;

    double preci = 1.0 * cntAB / cntA;
    double recal = 1.0 * cntAB / cntB;
    double f1 = 2 * preci * recal / (preci + recal);
    printf("%.10f %.10f\n", preci, recal);

    FILE *f1res = fopen("tmp/newsketch.res", "w");
    fprintf(f1res, "%.10f\n", f1);
    fclose(f1res);


    FILE* fthroughput = fopen("tmp/newsketch_throughput.res", "w");
    fprintf(fthroughput, "%lld\n", edtime - sttime);
    fclose(fthroughput);
}