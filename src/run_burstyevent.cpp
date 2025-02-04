/*
	run_burstyevent.cpp

	运行cm-pbe1，输出它的时间效率
	将准确率输出到 tmp/burstyevent.res，将吞吐量输出到 tmp/burstyevent_throughput.res
	（内存小时时间效率极低）
	usage: ./run_burstyevent mem(KB)

*/
#include "sketch/burstyevent/BurstyEvent.h"
#include "sketch/newsketch/param.h"
#include "read_correct.h"
#include "util/read_dataset.h"
#include "util/time.h"

#include <iostream>
#include <cstdio>
#include <cmath>
using namespace std;
uint64_t fid[11000000];
map<uint64_t, int> R[1000];

const int BB=40000;

double _tm[10001000];

extern vector<Burstelem> Tc;
extern vector<DS_Pack> DS;

int main(int argc, char **argv)
{
	//int threshold = 50;
	int mem = 5000;
	// sscanf(argv[1], "%d", &mem);
	int b = 50;
	BurstySketch A(3, mem * 1024 / b / 8 / 3);
	int window = 0, cnt = 0;

	const int N = 10000000;
	// const int N = 200000;
    init_dataset(N);
	long long sttime = time_ms();
	for(int i = 0; i < N; i++)
	{
		if (i % 1000 == 0)
			cerr << i << ' ' << DS[i].id << ' ' << DS[i].tm << endl;
		uint64_t id = DS[i].id; // caida
		_tm[i] = DS[i].tm;
		//if(i>=200000) continue;
		fid[i] = id;
		cnt++;
        if(cnt > BB)
        {
        	cnt = 0;
        	window++;
        }
        A.insert(id, i);
    }
	puts("end of insertion");
    A.cal(b, N);
	long long edtime = time_ms();
	puts("end of calculation");
	read_correct();

	double end_time;
	uint64_t flow_id;
    int total = 0;
    int correct = 0, total2 = 0;
    for(int i = 0; i < 200000; i++)
    {
    	if(A.query(fid[i], i) - 2 * A.query(fid[i], i - BB) + A.query(fid[i], i - BB * 2) >= 200)
    	{
    		if(R[i/BB][fid[i]] == 0)
				total++;
			R[i/BB][fid[i]] = 1;
    	}
    }
	for(int i = 0; i < (int)Tc.size(); i++) {
		end_time = Tc[i].t;
		flow_id = Tc[i].id;
		int pos=lower_bound(_tm,_tm+200000,end_time)-_tm;
		if(pos>200000) continue;
		total2++;
		if(R[pos/BB][flow_id]) {
			correct++;
		}
		/*if(A.query(flow_id, pos) - 2 * A.query(flow_id, pos - BB) + A.query(flow_id, pos - BB*2) >= 50) {
			correct++;
		}*/
	}
	double pr = 1.0 * correct / total;
	double rr = 1.0 * correct / total2;
	double f1 = 2 * pr * rr / (pr + rr);
	FILE *f1res = fopen("tmp/burstyevent.res", "w");
    fprintf(f1res, "%.10f\n", f1);
    fclose(f1res);

	FILE* fthroughput = fopen("tmp/burstyevent_throughput.res", "w");
    fprintf(fthroughput, "%lld\n", edtime - sttime);
    fclose(fthroughput);
	//cout<<pr<<" "<<rr<<endl;
	//printf("%.5lf\n",f1);
	//cout<<correct<<" "<<total2<<" "<<total<<endl;
	return 0;
}
