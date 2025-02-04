/*
    run_correct.cpp

    计算正确的burst，输出到 tmp/correct.txt 中
    
    usage: ./run_correct p(ratio) stdlim(burstlim) threshold
    分别为比例系数，burst加权平均的阈值，stage1到stage2的阈值，包个数的要求（论文中定义为1，即没有要求）

*/

#include "util/read_dataset.h"
#include "sketch/newsketch/correct.h"
#include "sketch/newsketch/param.h"
#include <bits/stdc++.h>
#include "util/BOBHash32.h"
using namespace std;

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

int main(int argc, char **argv) {
    struct Param P = Param{0.005, 0.01, 0.1, 0.5, 1, 0.05, 50000, 10000};
    // struct Param P = Param{0.005, 0.1, 0.5, 50, 0.05, 50000, 10000};
    if(argc > 1) {
        sscanf(argv[1], "%lf", &P._p);
    }
    if(argc > 2) {
        sscanf(argv[2], "%lf", &P._lim);
    }
    if(argc > 3) {
        sscanf(argv[3], "%d", &P._threshold);
    }
    load_param(P);
    CorrectSketch T;

    const int N = 10000000;
    init_dataset(N);
    cerr << "awsl " << DS.size () << endl;
    cerr << DS [0].id << ' ' << DS [0].tm << endl;
    cerr << DS [DS.size () - 1].id << ' ' << DS [DS.size () - 1].tm << endl;
    for(int i=0;i<N;i++) {
        T.insert(DS[i].id, DS[i].tm);
        if (i % 100000 == 0) {
            cerr << i << ' ' << DS[i].id << ' ' << DS[i].tm << endl;
        }
    }
    cerr << T.Burst.size () << endl;
    FILE* fcorrect = fopen("tmp/correct.txt", "w");
    for(int i = 0; i < (int)T.Burst.size(); i++) {
        fprintf(fcorrect, "%u %d %.10f\n", T.Burst[i].id, T.Burst[i].cnt, T.Burst[i].t);
    }
    fclose(fcorrect);
    cerr << T.Burst.size () << endl;

    FILE* fcount = fopen("tmp/burstcount.txt", "w");
    fprintf(fcount, "%d\n", (int)T.Burst.size());
    fclose(fcount);
    cerr << T.Burst.size () << endl;


    FILE* fcountu = fopen("tmp/burstcount_unique.txt", "w");
    set<unsigned> S;
    for(int i = 0; i < (int)T.Burst.size(); i++) {
        S.insert(T.Burst[i].id);
    }
    fprintf(fcountu, "%d\n", (int)S.size());
    fclose(fcountu);

    cerr << T.Burst.size () << endl;

    printf("hash size: %d\n", (int)T.cnt.size() * 4 + (int)T.lsttm.size() * 8 + (int)T.val.size() * 8);
}