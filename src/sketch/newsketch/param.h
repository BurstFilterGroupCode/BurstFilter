/*
 * param.h
 *
 * 算法基本参数
 */

#ifndef PARAM_H
#define PARAM_H
typedef unsigned id_type;


extern double lim;
extern double weaklim;
extern double p;
extern double outv;
extern int threshold;
extern double chklim;
extern int hash2;

struct Param {
    double _lim; // Burst阈值（时间带权平均的阈值）
    double _weaklim;// stage1 加入 stage2 的阈值
    double _p;      // 比例系数
    double _outv;   // 时间最小值（不小于该值）
    int _threshold; // Burst的最小大小（包个数）
    double _chklim; // check要求的时间差（benchmark相关）
    int Size_stage1;// stage1 大小
    int Size_stage2;// stage2 大小
    int _hash2;     // stage2 hash函数个数
};
//需要调的参

void load_param(struct Param x);

struct Burstelem {
    id_type id;
    int cnt;
    double t;
};

#endif