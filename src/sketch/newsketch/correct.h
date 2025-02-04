#ifndef CORRECT_H
#define CORRECT_H
#include "param.h"
#include <bits/stdc++.h>
using namespace std;

struct CorrectSketch {
    unordered_map<id_type,double> lsttm;
    unordered_map<id_type,double> val;
    unordered_map<id_type,int> cnt;
    CorrectSketch() {}
    vector<Burstelem> Burst;
    void reportBurst(id_type id, int cnt, double tm) {
        //cout<<id<<' '<<cnt<<' '<<tm<<endl;
        if(cnt>=threshold) {
            Burst.push_back((Burstelem){id, cnt, tm});
        }
    }
    void clearBurst() {Burst.clear();}
    void clearall() {
        clearBurst();
        lsttm.clear();
        val.clear();
        cnt.clear();
    }
    void insert(id_type id, double tm) {
        if(lsttm[id]<=0.000000001) {
            //lsttm[id]=-100000000.0;
            val[id]=-outv;
        }
        //cout<<id<<' '<<tm<<' '<<val[id]<<' '<<lsttm[id]<<endl;
        double lstval = val[id];
        val[id] = val[id] * (1.0 - p) + tm * p;
        val[id] = max(val[id], tm-outv);
        
        if (tm - lsttm [id] < outv && tm - lstval >= lim && tm - val [id] < lim) {
            reportBurst (id, 1, tm);
        }

        // if(tm - val[id] < lim) {
        //     cnt[id]++;
        // }
        // else if(lsttm[id] - lstval < lim) {
        //     reportBurst(id, cnt[id], lsttm[id]);
        //     cnt[id] = 0;
        // }
        lsttm[id] = tm;
    }
    size_t memory_used() {
        size_t ans = 0;
        ans = sizeof(int) + sizeof(double) + sizeof(double) + sizeof(id_type);
        ans = ans * val.size();
        return ans;
    }
};

#endif