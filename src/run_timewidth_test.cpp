/*
 * run_timewidth_test.cpp
 *
 * 测试数据包之间时间宽度的阈值，无命令行参数
 */

#include "util/BOBHash32.h"
#include "util/read_dataset.h"
#include "sketch/burstsketch/BurstDetectorLatency.h"

#include<bits/stdc++.h>
using namespace std;

uint32_t gethash(BOBHash32* bobhash, uint64_t val) {
    return bobhash->run((char *)&val, 8);
}


// 相关参数，包括两个阶段大小，哈希次数，比例系数，burst判定阈值
const double p=0.5;
// const double p=0.5;
const double lim=10;
const double weak_lim=1;
const double ouv = 1;
const double flush_intv = 1;

extern vector<DS_Pack> DS;

namespace Sketch_test  {
    
    // const int N1=10000;
    // const int N2=10000;
    const int S1=2;
    const int S2=4;

    int N1, N2;
    double *a;
    // double a[N1];
    struct Node {
        int id;
        double val;
    } **b;
    // }b[N2][S2];
    BOBHash32 **a_bobhash, *b_bobhash;
    vector<pair<int, double> > burstid;
    double v1[S1], v2[S1];
    void init (int _N1, int _N2) {
        delete [] a;
        for (int i = 0; i < N2; ++i)
            delete [] b[i];
        delete [] b;
        if (a_bobhash != NULL) {
            for (int i = 0; i < S1; ++i)
                delete a_bobhash [i];
        }
        delete [] a_bobhash;
        delete b_bobhash;
        burstid.clear ();
        N1 = _N1, N2 = _N2;
        a_bobhash = new BOBHash32*[S1];
        b_bobhash = new BOBHash32(233);
        for(int i = 0; i < S1; i++) {
            a_bobhash[i] = new BOBHash32(i+100);
        }
        a = new double [N1];
        memset (a, 0, sizeof (*a));
        b = new Node *[N2];
        for (int i = 0; i < N2; ++i) {
            b [i] = new Node [S2];
            memset (b [i], 0, sizeof (*b[i]));
        }
    }
    void insert (int j) {
        // cout << i << endl;
        int id = DS[j].id;
        double temp = DS[j].delay;
        int posb=gethash(b_bobhash, id)%N2;
        int vis = 0;
        for(int i = 0; i < S2; i++) if(b[posb][i].id == id) {
            double lsta = b[posb][i].val;
            b[posb][i].val = p * b[posb][i].val + (1 - p) * temp;
            vis = 1;
            if(lsta <= lim && b[posb][i].val > lim) burstid.push_back(make_pair(DS[j].id, DS[j].tm));
        }
        if(!vis) {
            for(int i = 0; i < S1; i++) {
                int posa = gethash(a_bobhash[i], id)%N1;
                double lsta = a[posa];
                if (a [posa] < 0.00000001) {
                    a [posa] = temp;
                }
                a[posa] = p * a[posa] + (1 - p) * temp;
                v1 [i] = lsta, v2 [i] = a [posa];
                // if(fabs(a[posa]-lsta) > lim) {
                //     ins = 1;
                // }
            }
            sort (v1, v1 + S1);
            sort (v2, v2 + S1);
            int pos = 0;
            for (int i = 0; i < S1 - 1; ++i) {
                if (v1 [i + 1] - v1 [i] < v1 [pos + 1] - v1 [pos])
                    pos = i;
            }
            double oldv = (v1 [pos] + v1 [pos + 1]) / 2;
            double newv = (v2 [pos] + v2 [pos + 1]) / 2;
            if (oldv <= lim && newv > lim) {
                burstid.push_back(make_pair(DS[j].id, DS[j].tm));
            }
            if (newv > weak_lim) {
                int pos = 0;
                for(int i = 0; i < S2; i++) {
                    if(b[posb][i].id == 0) {
                        b[posb][i].id = id;
                        b[posb][i].val = newv;
                        pos = -1;
                        break;
                    }
                    if (b [posb][i].val < b [posb][pos].val)
                        pos = i;
                }
                if (pos != -1 && newv > b [posb][pos].val) {
                    b[posb][pos].id = id;
                    b[posb][pos].val = newv;
                }
            }
        }
    }
    // void runtest() {
    //     for(int i = 0; i < (int)DS.size(); i++) {
    //         //cout<<i<<endl;
            
    //     }
    // }
}

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
                val[DS[i].id] = DS[i].delay;
                if (val[DS[i].id] > lim) {
                    burstid.push_back(make_pair(DS[i].id, DS[i].tm));
                }
            }
            else {
                used[DS[i].id] = 1;
                if (last_time [DS [i].id] < DS [i].tm - ouv) {
                    last_time [DS [i].id] = DS [i].tm;
                    val[DS[i].id] = DS[i].delay;
                    if (val[DS[i].id] > lim) {
                       burstid.push_back(make_pair(DS[i].id, DS[i].tm));
                    }
                    continue;
                }
                double oldval = val[DS[i].id];
                val[DS[i].id] = oldval * p + DS[i].delay * (1.0 - p);
                last_time [DS[i].id] = DS[i].tm;
                // if (val [DS[i].id] > lim) {
                //     cerr << i << ' ' << DS[i].id << ' ' << DS[i].delay << ' ' << val[DS[i].id] << endl;
                // }
                if(oldval <= lim && val[DS[i].id] > lim) {
                    burstid.push_back(make_pair(DS[i].id, DS[i].tm));
                }
            }
            mxcnt = max(mxcnt, (int)val.size());
        }
        cerr << "Max count: " << mxcnt << endl;
    }
};

double compare(vector<pair<int, double> > &ours, vector<pair<int, double> > &ans, FILE *file = NULL, int memory = 0, double mops = 0) {
    FILE *fres = fopen("../netsketch_test/result/latency_ours.txt","w");
    for(auto &v:ours) {
        fprintf(fres,"%d %.10f\n",v.first,v.second);
    }
    fclose(fres);
    cerr<<ours.size()<<endl;
    map<int, double> Mp;
    set <int> app;
    for (auto &jit : ours) {
        if (Mp.find (jit.first) == Mp.end ())
            Mp [jit.first] = jit.second;
    }
    cerr<<Mp.size()<<endl;
    double allt = DS[DS.size() - 1].tm;
    //printf("%.10f\n", allt);
    double sumerr = 0;
    int TP = 0, FP = 0, FN = 0;
    for (auto &v : ans) {
        if (app.find (v.first) != app.end ()) continue;
        app.insert (v.first);
        if (Mp.find (v.first) == Mp.end ()) {
            ++FN;
        }
        else {
            ++TP;
            sumerr += fabs (v.second - Mp [v.first]);
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

void test_res (double rt) {
    FILE *file = open_print_header ("../netsketch_test/result/latency_ours.csv");
    Strawman_test::init (-1);
    Strawman_test::runtest (1);
    // for (int sz = 100000000; sz <= 100000000; sz <<= 1) {
    for (int sz = 256; sz <= 32768; sz <<= 1) {
        int _N1 = sz * rt, _N2 = sz * (1 - rt);
        int N1 = _N1 / sizeof (double);
        int N2 = _N2 / ((sizeof (int) + sizeof (double)) * Sketch_test::S2);
        cout << sz << ' ' << N1 << ' ' << N2 << endl;
        int test_cycles = 10;
        timespec time1, time2;
        double mops = 0;
        for (int c = 0; c < test_cycles; ++c) {
            // cout << sz << ' ' << c << ' ' << N1 << ' ' << N2 << endl;
            Sketch_test::init (N1, N2);
            // cout << sz << ' ' << c << endl;
            long long resns = 0;
            for (int i = 0; i < (int) DS.size(); i++) {
                clock_gettime (CLOCK_MONOTONIC, &time1); 
                Sketch_test::insert (i);
                clock_gettime (CLOCK_MONOTONIC, &time2);
                resns += (long long) (time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec); 
            }
            double test_mops = (double) 1000.0 * DS.size () / (double) resns;
            mops += (test_mops - mops) / (c + 1);
        }
        compare (Sketch_test::burstid, Strawman_test::burstid, file, sz, mops);
    }
    fclose (file);
    return;
}

void test_strm () {
    FILE *file = open_print_header ("../netsketch_test/result/latency_strawman.csv");
    Strawman_test::init (-1);
    Strawman_test::runtest (1);
    auto ans = Strawman_test::burstid;
    for (int sz = 25000; sz <= 400000; sz <<= 1) {
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
        compare (Strawman_test::burstid, ans, file, sz * 13, mops);
    }
    fclose (file);
    return;
}

map<int,pair<double,double> > Windows;

const double tlen = 1;

const int window_length = 1;

void test_burstsketch()
{
    FILE *file = open_print_header ("../netsketch_test/result/latency_burstsketch.csv");
    Strawman_test::init (-1);
    Strawman_test::runtest (1);
    for (int sz = 1; sz <= 131072; sz <<= 1) {
        double threshold = lim;
        double l = 1; // the ratio of the Running Track threshold to the burst threshold
		double r12 = 3.75; // the ratio of the size of Stage 1 to the size of Stage 2
		double screen_layer_threshold = l * threshold; // Running Track threshold
        int mem = sz * 1024; // the size of memory
        int log_size = mem / (12 * r12 + 20) / bucket_size; // number of buckets in Stage 2
        log_size = max(1,log_size);
        int screen_layer_size = log_size * r12 * bucket_size; // number of buckets in Stage 1
        int total_memory = screen_layer_size * (sizeof (uint32_t) + sizeof (uint64_t)) + 
        log_size * (3 * sizeof (uint32_t) + sizeof (uint64_t)) * bucket_size;
        int test_cycles = 10;
        timespec time1, time2;
        double lst_t = 0;
        double mops = 0;
        for (int c = 0; c < test_cycles; ++c) {
            BurstDetector A(screen_layer_size, 0, log_size, threshold);
            Windows.clear();
            int window = 0, cnt = 0;
            long long resns = 0;
            for(int i = 0; i < (int) DS.size(); i++) {
                // if(i%1000000==0)cerr<<(i/1000000)<<" / "<< DS.size()/1000000<<endl;
                uint64_t id = DS[i].id;
                double temp = DS[i].delay;
                double tt = DS[i].tm;
                clock_gettime (CLOCK_MONOTONIC, &time1);
                if(cnt == 0 || tt - lst_t >= tlen)
                {
                    window++;
                    Windows[window].first = tt;
                    lst_t = tt;
                }
                cnt++;
                /*
                if(cnt > window_length)
                {
                    cnt = 0;
                    window++;
                    Windows[window].first=DS[i].tm;
                }*/
                Windows[window].second=DS[i].tm;
                A.insert(id, window, temp, 0, DS[i].tm);
                clock_gettime (CLOCK_MONOTONIC, &time2);
                resns += (long long) (time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
            }
            double test_mops = (double) 1000.0 * DS.size () / (double) resns;
            mops += (test_mops - mops) / (c + 1);
            if (c == test_cycles - 1) {
                vector<pair<int, double> > burst;
                for (auto &bst : A.log.Record) {
                    pair<double,double> window_range = Windows[bst.start_window];
                    uint32_t id = bst.flow_id;
                    burst.push_back(make_pair(id, window_range.first));
                }
                compare(burst, Strawman_test::burstid, file, total_memory, mops);
            }
        }
    }
}

void test_wtf()
{
    int total_memory = 1;
    vector<pair<int, double> > burst;
    Strawman_test::init (-1);
    Strawman_test::runtest (1);
    for(auto i:DS)
    {
        if(i.delay >= lim)burst.push_back(make_pair(i.id,i.tm));
    }
    compare(burst, Strawman_test::burstid, NULL, total_memory, 0);
}

int main() {
    init_dataset(10000000);
    cout<<"Initiation Completed\n";
    cout << DS [0].id << ' ' << DS [0].tm << ' ' << DS [0].delay << endl;
    cout << DS [DS.size () - 1].id << ' ' << DS [DS.size () - 1].tm << ' ' << DS [DS.size () - 1].delay << endl;
    // test_res (0.5);
    // test_strm ();
    test_wtf();
    // test_burstsketch();
    // Sketch_test::runtest();
    // Strawman_test::runtest();
    // cout<<Sketch_test::burstid.size()<<endl;
    // cout<<Strawman_test::burstid.size()<<endl;
}