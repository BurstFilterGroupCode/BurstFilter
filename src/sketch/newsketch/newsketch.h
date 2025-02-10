#ifndef MYNEW2_H
#define MYNEW2_H
#include "param.h"
#include "../../util/cache_log.h"
#include <bits/stdc++.h>
using namespace std;

#define CHECK_LOG

struct Mydouble {
    int val;
    Mydouble() {}
    Mydouble(int x, int NO_USE) {val = x;}
    Mydouble(double x) {
        val = x * 1000000;
    }
    Mydouble operator +(const Mydouble &t)const {return Mydouble(val + t.val, 0);}
    Mydouble operator -(const Mydouble &t)const {return Mydouble(val - t.val, 0);}
    Mydouble operator -()const {return Mydouble(-val, 0);}
    Mydouble operator *(const Mydouble &t)const {return Mydouble(round(1ll * val * t.val / 1000000), 0);}
    bool operator <(const Mydouble &t)const {return val < t.val;}
    bool operator <=(const Mydouble &t)const {return val <= t.val;}
    double todouble() {return val / 1000000.0;}
};

static Mydouble myLIM = Mydouble(lim);
static Mydouble myWeakLIM = Mydouble(weaklim);
static Mydouble myp = Mydouble(p);
static const int Hash1 = hash1;
static const int Hash2 = 1;
static const Mydouble inf = Mydouble(1000);
static Mydouble myoutv = Mydouble(outv);

#include "../../util/BOBHash32.h"

static uint32_t gethash(BOBHash32* bobhash, uint64_t val) {
    // cerr << "?????????? " << bobhash << ' ' << val << endl;
    return bobhash->run((char *)&val, 8);
}

struct Node {
    id_type id;
    Mydouble tval, lstt;
    /**
     * Actually, this CNT could be replaced by a boolean variable.
     * Our sketch doesn't depend on its exact value.
     * So we would count it as 1 bit when calculating memory consumption.
     */
    int cnt;
    void ins(Mydouble tmv) {
        cnt++;
        tval = (Mydouble(1.0) - myp) * tval + tmv * myp;
    }
};

struct SetOfNodes {
    static const int S=16;
    Node a[S];
    SetOfNodes() {
        for(int i=0;i<S;i++) a[i].cnt=-1;
    }
    int find(id_type id) {
        for(int i=0;i<S;i++) {
            if(a[i].cnt>=0&&a[i].id==id) return i;
        }
        return -1;
    }
    pair<int, id_type> ins(id_type id, Mydouble tval, Mydouble tm) {
        for(int i=0;i<S;i++) if(a[i].cnt==-1) {
            a[i].cnt = 1;
            a[i].tval = tval;
            a[i].lstt = tm;
            a[i].id = id;
            return make_pair(1, 0);
        }
        Mydouble mntv = tval;
        int mnpos = -1;
        for(int i=0;i<S;i++) if(a[i].tval < mntv) mntv = a[i].tval, mnpos = i;
        if(mnpos != -1) {
            id_type ans = a[mnpos].id;
            a[mnpos].cnt = 1;
            a[mnpos].tval = tval;
            a[mnpos].lstt = tm;
            a[mnpos].id = id;
            return make_pair(2, ans);
        }
        return make_pair(0,0);
    }
    int remove(int id) {
        for(int i=0;i<S;i++) if(a[i].cnt>=0&&(int)a[i].id==id) {
            int ans=a[i].cnt;
            a[i].cnt=-1;
            return ans;
        }
        return 0;
    }
};


struct NewSketch {
    vector<Cache_Log> Log;
    void load_params() {
        myLIM = Mydouble(lim);
        myWeakLIM = Mydouble(weaklim);
        myp = Mydouble(p);
        myoutv = Mydouble(outv);
    }
    BOBHash32 **stg1_bobhash, **stg2_bobhash;
    // double curtime;

    Mydouble *stg1_val;
    Mydouble *stg1_lst;
    SetOfNodes *stg2;
    id_type *stg2_id;
    int stg1_size, stg2_size;
    vector<Burstelem> Burst;
    NewSketch() {}
    ~NewSketch() {
        for (int i = 0; i < Hash1; ++i)
            delete stg1_bobhash [i];
        delete [] stg1_bobhash;
        for (int i = 0; i < Hash2; ++i)
            delete stg2_bobhash [i];
        delete [] stg2_bobhash;
        delete [] stg1_val;
        delete [] stg1_lst;
        delete [] stg2;
    }
    size_t memory_used;
    NewSketch(int __stg1_size, int __stg2_size) {
        Burst.clear();
        Log.clear();
        memory_used = 0;
        // curtime = 0;
        load_params();
        stg1_size = __stg1_size / sizeof(Mydouble);
        stg2_size = __stg2_size / (SetOfNodes::S * 9);
        cerr << stg2_size * SetOfNodes::S << endl;
        // stg2_size = __stg2_size / sizeof(SetOfNodes);
        stg1_val = new Mydouble[stg1_size];
        memory_used += sizeof(Mydouble) * stg1_size;
        stg1_lst = new Mydouble[stg1_size];
        // memory_used += sizeof(Mydouble) * stg1_size;
        stg2 = new SetOfNodes[stg2_size];
        memory_used += (SetOfNodes::S * 9) * stg2_size;
        // memory_used += sizeof(SetOfNodes) * stg2_size;
        stg1_bobhash = new BOBHash32*[Hash1];
        stg2_bobhash = new BOBHash32*[Hash2];
        for(int i = 0; i < stg1_size; i++) {
            stg1_val[i] = -myoutv;
            stg1_lst[i] = 0;
        }
        for(int i = 0; i < stg2_size; i++) {
            stg2[i] = SetOfNodes();
        }
        for(int i = 0; i < Hash1; i++) {
            stg1_bobhash[i] = new BOBHash32(10 + i);
        }
        for(int i = 0; i < Hash2; i++) {
            stg2_bobhash[i] = new BOBHash32(30 + i);
        }
        //printf("stg2 size: %d\n", (int)stg2_size);
        ofstream cachesz("tmp/cachesz.tmp");
        cachesz<<(int)stg2_size*SetOfNodes::S<<endl;
        cachesz.close();
        //printf("Cache cnt: %d\n", (int)stg2_size*SetOfNodes::S);
    }

    // void flush_sketch() {
    //     curtime += outv;
    //     for(int i = 0; i < stg1_size; i++) {
    //         stg1_val[i] = max(-myoutv, stg1_val[i] - myoutv);
    //     }
    // }


    // void clearBurst() {
    //     Burst.clear();
    // }

    void reportBurst(id_type id, int cnt, Mydouble lstt) {
        //if(cnt>10)cout<<id<<' '<<cnt<<endl;
        /**
         * This THRESHOLD should always be 1.
         */
        if(cnt>=threshold) {
            Burst.push_back((Burstelem){id, cnt, lstt.todouble() /*+ curtime*/});
        }
    }

    vector<int> stg1_calhash(id_type id) {
        vector<int> ans;
        for(int i = 0; i < Hash1; i++) {
            ans.push_back(gethash(stg1_bobhash[i], id) % stg1_size);
        }
        return ans;
    }

    vector<int> stg2_calhash(id_type id) {
        vector<int> ans;
        for(int i = 0; i < Hash2; i++) {
            // cerr << "?? " << i << ' ' << Hash2 << ' ' << endl;
            ans.push_back(gethash(stg2_bobhash[i], id) % stg2_size);
        }
        // cerr << "!!" << endl;
        return ans;
    }

    Mydouble stg1_lstvisit(id_type id) {
        vector<int> h = stg1_calhash(id);
        Mydouble mnv = inf;
        for(auto x : h) {
            mnv = min(mnv, stg1_lst[x]);
        }
        return mnv;
    }

    pair<Mydouble,Mydouble> stg1_insert(id_type id, Mydouble tm) {
        vector<int> h = stg1_calhash(id);
        Mydouble mnv = inf;
        for(auto x : h) {
            mnv = min(mnv, stg1_val[x]);
        }
        Mydouble cal = mnv * (Mydouble(1.0) - myp) + tm * myp;
        for(auto x : h) {
            stg1_val[x] = max(max(stg1_val[x], tm - myoutv), cal);
            stg1_lst[x] = max(stg1_lst[x], tm);
        }
        return make_pair(mnv, cal);
    }

    // void stg2_clear(id_type id, int report01, Mydouble lstt) {
    //     //return;
    //     vector<int> h = stg2_calhash(id);
    //     /*int mnv = 1e9;
    //     for(auto x : h) {
    //         mnv = min(mnv, stg2_cnt[x]);
    //         //cout<<stg2_cnt[x]<<' ';
    //     }
    //     //cout<<endl;
    //     if(report01) reportBurst(id, mnv, lstt);
    //     for(auto x : h) {
    //         stg2_cnt[x] -= mnv;
    //     }*/
    //     //stg2_mp[id] = 0;
    //     int ans = stg2[h[0]].remove(id);
    //     // if(ans > 0 && report01) {
    //     //     reportBurst(id, ans, lstt);
    //     // }
    // }

    void stg2_insert(id_type id, Mydouble tmv, Mydouble tm) {
        vector<int> h = stg2_calhash(id);
        
        pair<int, id_type> tmp = stg2[h[0]].ins(id, tmv, tm);
    #ifdef CHECK_LOG
        if(tmp.first) {
            Log.push_back((Cache_Log){INSERT, id});
        }
        if(tmp.first == 2) {
            Log.push_back((Cache_Log){EVICT, tmp.second});
        }
    #endif
        //for(auto x : h) stg2_cnt[x]++;
        //stg2_mp[id]++;
    }

    bool try_stg2(id_type id, Mydouble tm) {
        // cerr << "? 3" << endl;
        vector<int> h = stg2_calhash(id);
        int pos = stg2[h[0]].find(id);
        if(pos==-1) return 0;
        if (myoutv < tm - stg2[h[0]].a[pos].lstt) {
            stg2[h[0]].a[pos].lstt = tm;
            stg2[h[0]].a[pos].tval = tm - myoutv;
            return 1;
        }
        Mydouble old_val = stg2[h[0]].a[pos].tval;
        stg2[h[0]].a[pos].ins(tm);
        stg2[h[0]].a[pos].lstt = tm;
        if (myLIM <= tm - old_val && tm - stg2[h[0]].a[pos].tval < myLIM) {
            reportBurst (id, stg2[h[0]].a[pos].cnt, tm);
        }
        // if(myLIM < tm - stg2[h[0]].a[pos].tval) {
        //     //printf("stg2 clear\n");
        //     // stg2_clear(id, 1, stg2[h[0]].a[pos].lstt);
        //     stg2[h[0]].remove(id);
        // #ifdef CHECK_LOG
        //     Log.push_back((Cache_Log){EVICT, id});
        // #endif
        //     return 0;
        // }
        // else stg2[h[0]].a[pos].lstt = tm;
        return 1;
    }

    void main_insert(id_type id, double dtm, double dlsttm) {
        static int i = 0;
        i++;
    #ifdef CHECK_LOG
        Log.push_back((Cache_Log){VISIT, id});
    #endif
        //static ofstream dbg("debug.txt");
        // cerr << "? 1" << endl;
        Mydouble tm = dtm;
        if(try_stg2(id, tm)) {
            //if(id ==  3283706691) dbg<<i<<' '<<id<<' '<<"stg2"<<endl;
            return;
        }
        // cerr << "? 2" << endl;
        //Mydouble lsttm = dlsttm;
        //printf("%d %lf %lf\n",id,tm,lsttm);
        //Mydouble lstt = stg1_lstvisit(id);
        /*if(lstt != lsttm) {
            static int count = 0;
            cout<<(++count)<<endl;
        }*/
        
        pair<Mydouble, Mydouble> val = stg1_insert(id, tm);
        if (myLIM <= tm - val.first && tm - val.second <= myLIM) {
            reportBurst (id, 1, tm);
        }
        //if (id == 3283706691) dbg<<i<<' '<<id<<' '<<"stg1"<<endl;

        /*if(val.first < tm - 10.0) {
            stg2_clear(id, 0);
        }*/
        // time check?
        if(tm - val.second < myWeakLIM) stg2_insert(id, val.second, tm);//,cout<<i<<' '<<id<<' '<<"stg1_to_stg2"<<endl;
        //else cout<<i<<' '<<id<<' '<<"stg1"<<endl;
    }

    // void flushit() {

    // }
};

/*
314663676 35
1403037399 17
3762479688 31
3304600921 14
1141649456 85
----------
537266252 73
3825352984 19
1236646940 29
----------
3810458110 25
3963992500 56
2024036715 21
760513419 19


*/

#endif