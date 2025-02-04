/*
 * run_jitter_test.cpp
 *
 * 运行jitter测试，算法均在cpp中实现
 */

#include "util/BOBHash32.h"
#include "util/read_dataset.h"
#include "sketch/burstsketch/BurstDetector.h"

#include <bits/stdc++.h>
using namespace std;

uint32_t gethash(BOBHash32* bobhash, uint64_t val) {
    return bobhash->run((char *)&val, 8);
}

extern vector<DS_Pack> DS;

map<int, double> lstt;

struct jitter {
    int id;
    double t;
};

// 算法相关参数
const double lim = 1.5e-2; // burst阈值
const double p = 0.9; // 比例系数
const double ouv = 1; // 每个流第一个包默认时间差
const double weak_lim = lim * 1.3; // 插入第二阶段阈值
const double flush_intv = ouv; // strawman刷新间隔

struct Sketch {
    double *a; // 第一阶段
    struct Node {
        int id;
        double val; // 带权平均
    };
    struct Node **b; // 第二阶段
    int N1, N2, S1, S2;
    // 两个阶段的大小，S1表示第一阶段哈希次数，S2表示第二阶段缓存每个组条目个数
    BOBHash32 *b_hash;
    BOBHash32 **a_hash;
    vector<jitter> J;
    void report(int id, double t) {
        J.push_back((jitter){id,t});
    }
    // sz表示字节数，第一个阶段内存占比，vS1和vS2同S1和S2
    void init(int sz, double rt, int vS1, int vS2) {
        if (a != NULL) {
            delete [] a;
            for (int i = 0; i < N2; ++i)
                delete [] b [i];
            delete [] b;
            delete b_hash;
            for (int i = 0; i < S1; ++i)
                delete a_hash [i];
            delete [] a_hash;
        }
        J.clear ();
        S1 = vS1;
        S2 = vS2;
        N1 = (int)(sz * rt / 8);
        N2 = (int)(sz * (1.0 - rt) / 12 / S2);
        // cout<<"jlkfdjkdsljlkds "<<N1<<' '<<N2<<endl;
        //N2 = 50;
        //N1 = 200;
        a = new double[N1];
        memset(a, 0, sizeof(double) * N1);
        b_hash = new BOBHash32(7);
        a_hash = new BOBHash32*[S1];
        for(int i = 0; i < S1; i++) {
            a_hash[i] = new BOBHash32(11 + i);
        }
        b = new Node*[N2];
        for(int i = 0; i < N2; i++) {
            b[i] = new Node[S2];
            memset(b[i], 0, sizeof(Node) * S2);
        }
    }// 改min
    void ins(int id, double t, double val) {
        int posb = gethash(b_hash, id) % N2;
        int vis = 0;
        for(int i = 0; i < S2; i++) {
            if(b[posb][i].id == id) {
                vis = 1;
                if (val >= ouv) {
                    b [posb][i].id = 0;
                    b [posb][i].val = ouv;
                }
                else {
                    double new_val = p * b[posb][i].val + (1 - p) * val;
                    if (b [posb][i].val >= lim && new_val < lim) {
                        report (id, t);
                    }
                    b[posb][i].val = new_val;
                }
                break;
            }
        }
        if(vis == 0) {
            /*static */double v[10], v1[10];
            //vector <double> v;
            //vector <double> v1;
            if (val >= ouv) {
                val = ouv;
            }
            for(int i = 0; i < S1; i++) {
                int posa = gethash(a_hash[i], id) % N1;
                if(a[posa] == 0) a[posa] = ouv;
                //v1.push_back (a [posa]);
                v1[i] = a[posa];
                a[posa] = p * a[posa] + (1 - p) * val;
                //v.push_back (a [posa]);
                v[i] = a[posa];
            }
            
            //sort (v.begin (), v.end ());
            sort(v, v + S1);
            double tmp = v [1] - v [0];
            int pos = 0;
            for (int i = 1; i + 1 < /*(int) v.size ()*/ S1; i++) {
                // if(v[i+1] - v[i] < tmp && v[i] < lim) pos = i, tmp = v[i+1] - v[i];
                if (v [i + 1] - v [i] < tmp) pos = i, tmp = v [i + 1] - v [i];
            }
            double vmn = (v [pos] + v [pos + 1]) / 2;

            //sort (v1.begin (), v1.end ());
            sort(v1, v1 + S1);
            tmp = v1 [1] - v1 [0];
            pos = 0;
            for (int i = 1; i + 1 < /*(int) v1.size ()*/ S1; i++) {
                if (v1 [i + 1] - v1 [i] < tmp) pos = i, tmp = v1 [i + 1] - v1 [i];
            }
            double vmn1 = (v1 [pos] + v1 [pos + 1]) / 2;
            
            // if (vmn < lim) {
            if (vmn1 >= lim && vmn < lim) {
                report (id, t);
            }
            if (vmn < weak_lim) {
                tmp = 0;
                pos = 0;
                for(int i = 0; i < S2; i++) {
                    if (b [posb][i].id == 0) {
                        pos = i;
                        break;
                    }
                    if(b [posb][i].val > tmp) {
                        tmp = b [posb][i].val;
                        pos = i;
                    }
                }
                if (b [posb][pos].id == 0 || b [posb][pos].val > vmn) {
                    b [posb][pos].id = id;
                    b [posb][pos].val = vmn;
                }
            }
        }
    }
}skt;

// 定期刷新的unordered_map，刷新需要调用flush进行
struct Strawman {
    unordered_map<int, double> val;
    // unordered_map<int, int> isinburst;
    unordered_map<int, int> visited;
    int mem_lim;
    int szmx;// 过程中使用的最大内存
    vector<jitter> J;
    void report(int id, double t) {
        J.push_back((jitter){id,t});
    }
    void init(int _mem_lim = -1) {
        cout << "???" << endl;
        szmx = 0;
        mem_lim = _mem_lim;
        val.clear ();
        visited.clear ();
        J.clear ();
    }
    void ins(int id, double t, double insval) {
        if (val.find (id) == val.end () && (int) val.size() == mem_lim)
            return;
        visited [id] = 1;
        if (insval >= ouv) {
            val [id] = ouv;
        }
        else {
            double new_val = p * val [id] + (1 - p) * insval;
            if (val [id] >= lim && new_val < lim) {
                report (id, t);
            }
            val [id] = new_val;
        }
        szmx = max(szmx, (int)val.size());
    }
    void flush() {
        for(auto it = visited.begin(); it != visited.end(); ) {
            auto pii = *it;
            if (pii.second == 0) {
                val.erase(pii.first);
                auto itt = next (it);
                visited.erase (it);
                it = itt;
            }
            else {
                it->second = 0;
                it = next (it);
            }
        }
    }
}strm;

/*
设目标空间
调（大概的）刷新间隔，空间比目标略大一点
放不进去就不放
*/

// benchmark相关
// const double testlim = 0.05;

extern vector<pair<int, double> > Correct_Report;

double compare(const vector<jitter> J, FILE *file = NULL, int memory = 0, double mops = 0) {
    map<int, double> Mp;
    for (auto jit : J) {
        if (Mp.find (jit.id) == Mp.end ())
            Mp [jit.id] = jit.t;
    }
    double allt = DS[DS.size() - 1].tm;
    //printf("%.10f\n", allt);
    double sumerr = 0;
    int TP = 0, FP = 0, FN = 0;
    for (auto pii : Correct_Report) {
        if (Mp.find (pii.first) == Mp.end ()) {
            ++FN;
        }
        else {
            ++TP;
            sumerr += fabs (pii.second - Mp [pii.first]);
        }
    }
    FP = Mp.size () - TP;
    sumerr += (FN + FP) * allt;
    double R = (double) TP / (TP + FN), P = (double) TP / (TP + FP), F1 = 2 * R * P / (R + P);
    printf ("TP: %d, FP: %d, FN: %d\n", TP, FP, FN);
    printf ("Recall: %.6lf%%, Precision: %.6lf%%, F1 Score: %.6lf%%\n", R * 100, P * 100, F1 * 100);
    if (file != NULL) {
        fprintf (file, "%d,%d,%d,%d,%d,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf\n",
            memory, (int) Mp.size (), TP, FP, FN, F1 * 100, P * 100, R * 100, sumerr / (TP + FP + FN), mops);
    }
    return sumerr / TP / allt;
}

FILE *open_print_header (const char *name) {
    FILE *file = fopen (name, "w");
    assert (file != NULL);
    fprintf (file, "Memory,Report_Count,TP,FP,FN,F1_Score,Precision,Recall,Delta,MOPS\n");
    return file;
}

void test_res (double rt, int S1, int S2) {
    FILE *file = open_print_header ("../netsketch_test/result/jitter_ours.csv");
    for (int sz = 256; sz <= 32768; sz <<= 1) {
        int test_cycles = 10;
        timespec time1, time2;
        double mops = 0;
        for (int c = 0; c < test_cycles; ++c) {
            skt.init (sz, rt, S1, S2);
            lstt.clear ();
            long long resns = 0;
            for (int i = 0; i < (int) DS.size(); i++) {
                int id = DS[i].id;
                double t = DS[i].tm;
                double val = ouv;
                if(lstt.find(id) != lstt.end()) {
                    val = t - lstt[id];
                }
                lstt[id] = t;
                clock_gettime (CLOCK_MONOTONIC, &time1); 
                skt.ins(id, t, val);
                clock_gettime (CLOCK_MONOTONIC, &time2);
                resns += (long long) (time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec); 
            }
            double test_mops = (double) 1000.0 * DS.size () / (double) resns;
            mops += (test_mops - mops) / (c + 1);
        }
        compare (skt.J, file, sz, mops);
    }
    fclose (file);
    return;
}

void test_strm () {
    FILE *file = open_print_header ("../netsketch_test/result/jitter_strawman.csv");
    for (int sz = 312; sz <= 10000; sz <<= 1) {
        int test_cycles = 10;
        timespec time1, time2;
        double lstflush = 0;
        double mops = 0;
        for (int c = 0; c < test_cycles; ++c) {
            strm.init (sz);
            lstt.clear ();
            long long resns = 0;
            for (int i = 0; i < (int) DS.size(); i++) {
                int id = DS[i].id;
                double t = DS[i].tm;
                double val = ouv;
                if(lstt.find(id) != lstt.end()) {
                    val = t - lstt[id];
                }
                lstt[id] = t;
                clock_gettime (CLOCK_MONOTONIC, &time1); 
                strm.ins(id, t, val);
                if(DS[i].tm - lstflush > flush_intv) {
                    lstflush = DS[i].tm;
                    strm.flush();
                }
                clock_gettime (CLOCK_MONOTONIC, &time2);
                resns += (long long) (time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec); 
            }
            double test_mops = (double) 1000.0 * DS.size () / (double) resns;
            mops += (test_mops - mops) / (c + 1);
        }
        compare (strm.J, file, sz * 13, mops);
    }
    fclose (file);
    return;
}

map<int,pair<double,double> > Windows;

const double tlen = 1;

const double threshold = tlen / lim / 5;

void test_Burstsketch()
{
    FILE *file = open_print_header ("../netsketch_test/result/jitter_burstsketch.csv");
    for (int sz = 1; sz <= 16384; sz <<= 1) {
        int mem = sz; // the size of memory
		double l = 5; // the ratio of the Running Track threshold to the burst threshold
		double r12 = 3.75; // the ratio of the size of Stage 1 to the size of Stage 2
		double screen_layer_threshold = l * threshold; // Running Track threshold
		int log_size = mem * 1024 / (12 * r12 + 20) / bucket_size; // number of buckets in Stage 2
		int screen_layer_size = log_size * r12 * bucket_size; // number of buckets in Stage 1
		int total_memory = screen_layer_size * (sizeof (uint32_t) + sizeof (uint64_t)) + 
		log_size * (3 * sizeof (uint32_t) + sizeof (uint64_t)) * bucket_size;
        int test_cycles = 10;
        timespec time1, time2;
        double mops = 0;
        for (int c = 0; c < test_cycles; ++c) {
			BurstDetector A(screen_layer_size, int(screen_layer_threshold), log_size, int(threshold));
			Windows.clear ();
            lstt.clear();
            int window = 0, cnt = 0;
            double lst_t = 0;
            long long resns = 0;
            // cerr<<"size="<<DS.size()<<endl;
            for (int i = 0; i < (int) DS.size(); i++) {
                uint64_t id = DS[i].id;
                double tt = DS[i].tm;
                // if(i%10000==0)cerr<<tt<<endl;
                clock_gettime (CLOCK_MONOTONIC, &time1); 
                if(cnt == 0 || tt - lst_t >= tlen)
                {
                    window++;
                    Windows[window].first = tt;
                    lst_t = tt;
                }
                cnt++;
                // if(i%10000==0)cerr<<window<<endl;
                Windows[window].second=tt;
                A.insert(id, window);   
                clock_gettime (CLOCK_MONOTONIC, &time2);
                resns += (long long) (time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec); 
            }
            double test_mops = (double) 1000.0 * DS.size () / (double) resns;
            mops += (test_mops - mops) / (c + 1);
            if (c == test_cycles - 1) {
                vector<jitter> J;
                for(int i = 0; i < (int) A.log.Record.size(); i++) {
                    struct Burst bst = A.log.Record[i];
                    pair<double,double> window_range = Windows[bst.start_window];
                    uint32_t id = bst.flow_id;
                    J.push_back((jitter){id,window_range.first});
                }
                compare (J, file, total_memory, mops);
            }
        }
    }
    fclose (file);
    return;
}

int main() {
    init_dataset(10000000);
    cout<<"Finished dataset reading\n";
    // test_res (0.5, 4, 4);
    // test_strm ();
    test_Burstsketch();
    return 0;
    /*
    // run jitter
    skt.init(20000, 0.5, 4, 4);
    strm.init(10000);
    // strm.init(800000 / (sizeof(double) + 1 + sizeof(int)));
    cout<<DS.size()<<endl;
    printf("%.10f\n", DS[DS.size() - 1].tm);
    double lstflush = 0;
    for(int i = 0; i < (int)DS.size(); i++) {
        int id = DS[i].id;
        double t = DS[i].tm;
        double val = ouv;
        if(lstt.find(id) != lstt.end()) {
            val = t - lstt[id];
        }
        lstt[id] = t;
        skt.ins(id, t, val);
        strm.ins(id, t, val);
        if(DS[i].tm - lstflush > flush_intv) {
            lstflush = DS[i].tm;
            strm.flush();
        }
        if(i%100000 == 0) cout<<i<<' '<<id<<' '<<t<<' '<<val<<endl;
    }
    cout<<"Finished Processing\n";
    cout<<strm.J.size()<<endl;
    printf("%.10f\n", compare(strm.J) * DS.size());
    cout<<"max size "<<strm.szmx<<endl;
    cout<<skt.J.size()<<endl;
    printf("%.10f\n", compare(skt.J) * DS.size());
    */
}
