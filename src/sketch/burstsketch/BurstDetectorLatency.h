#ifndef burstdetector_H
#define burstdetector_H
#include<bits/stdc++.h>
#include "../../util/BOBHash32.h"
#include "param.h"
#include "Burst.h"
class ScreenLayer
{
	public:
	int size;
	double m;
	int tot=0;
	pair<double, int>* counter;
	uint64_t* id;
	BOBHash32* bobhash[hash_num];
	ScreenLayer(){};
	ScreenLayer(int _size, double _m)
	{
		size = _size;
		m = _m;
		counter = new pair<double, int>[size];
		id = new uint64_t[size];
		for(int i = 0; i < size; i++)
		{
			id[i] = 0;
			counter[i] = make_pair(0,0);
		}
		for(int i = 0; i < hash_num; i++)
			bobhash[i] = new BOBHash32(i + 103);
	}
	void update()
	{
		for(int i = 0; i < size; i++)
		{
			counter[i] = make_pair(0,0);
			id[i] = 0;
		}
	}
	pair<double, int> lookup(int c)
	{
		return counter[c];
	}
	void clear(int c)
	{
		// the basic version 
		uint64_t flow_id = id[c];
		for(int i = 0; i < hash_num; i++)
		{
			int z = bobhash[i]->run((char *)&flow_id, 8) % size;
			if(id[z] == flow_id)
			{
				id[z] = 0;
				counter[z] = make_pair(0,0);
			}
		}
		/*
		// the optimized version
		counter[c] = 0;
		id[c] = 0;
		*/
	}
	double calc_ratio(pair<double, int> x)
	{
		if(x.second == 0)
			return 0;
		return x.first / x.second;
	}
	int insert(uint64_t flow_id, double weight, bool print_detail, double realtime)
	{
		int ret = -1;
		
		// the basic version
		for(int i = 0; i < hash_num; i++)
		{
			int z = bobhash[i]->run((char *)&flow_id, 8) % size;
			if(id[z] == flow_id)
			{
				counter[z].first += weight;
				counter[z].second ++;
				if(calc_ratio(counter[z]) >= m)
					ret = z;
			}
			else if(id[z] == 0)
			{
				counter[z].first += weight;
				counter[z].second ++;
				id[z] = flow_id;
			}
			else
			{
				counter[z].first -= weight;
				counter[z].second --;
				if(counter[z].first <= 0 || counter[z].second == 0)counter[z]=make_pair(0,0),id[z] = 0;
			}
		}
		
		/*
		int flag = 0;
		// the optimized version
		for(int i = 0; i < hash_num; i++)
		{
			int z = bobhash[i]->run((char *)&flow_id, 8) % size;
			if(id[z] == flow_id)
				flag |= 1;
			else if(id[z] == 0)
				flag |= 2;
		}
		if(flag & 1)
		{
			for(int i = 0; i < hash_num; i++)
			{
				int z = bobhash[i]->run((char *)&flow_id, 8) % size;
				if(id[z] == flow_id)
				{
					counter[z].first += weight;
					counter[z].second ++;
					if(calc_ratio(counter[z]) >= m)
						ret = z;
				}
			}
		}
		else if(flag & 2)
		{
			for(int i = 0; i < hash_num; i++)
			{
				int z = bobhash[i]->run((char *)&flow_id, 8) % size;
				if(id[z] == 0)
				{
					counter[z].first += weight;
					counter[z].second ++;
					id[z] = flow_id;
					break;
				}
			}
		}
		else
		{
			for(int i = 0; i < hash_num; i++)
			{
				int z = bobhash[i]->run((char *)&flow_id, 8) % size;
				counter[z].first -= weight;
				counter[z].second --;
				if(counter[z].first <= 0 || counter[z].second == 0)counter[z]=make_pair(0,0),id[z] = 0;
			}
		}
		*/
		if(ret != -1)tot++;
		return ret;
	}
};
class Log // Stage 2
{
	public:
	BOBHash32* bobhash;
	bool flag;
	int size;
	double m, screen_layer_threshold;
	pair<double, int>** counter[2];
	uint64_t** id;
	uint32_t** timestamp;
	vector<Burst> Record;
	Log(){};
	Log(int _size,double _m, double _screen_layer_threshold)
	{
		flag = 0;
		size = _size;
		m = _m;
		screen_layer_threshold = _screen_layer_threshold;
		Record.clear();
		counter[0] = (pair<double, int> **)new pair<double, int>*[size];
		counter[1] = (pair<double, int> **)new pair<double, int>*[size];
		id = (uint64_t **)new uint64_t*[size];
		timestamp = (uint32_t **)new uint32_t*[size];
		for(int i = 0; i < size; i++)
		{
			counter[0][i] = new pair<double, int>[bucket_size];
			counter[1][i] = new pair<double, int>[bucket_size];
			id[i] = new uint64_t[bucket_size];
			timestamp[i] = new uint32_t[bucket_size];
		}
		for(int i = 0; i < size; i++)
			for(int j = 0; j < bucket_size; j++)
			{
				counter[0][i][j] = make_pair(0,0);
				counter[1][i][j] = make_pair(0,0);
				id[i][j] = 0;
				timestamp[i][j] = -1;
			}
		bobhash = new BOBHash32(1005);
		//printf("Cache cnt: %d\n", (int)(bucket_size * size));
	}
	double calc_ratio(pair<double, int> x)
	{
		if(x.second == 0)
			return 0;
		return x.first / x.second;
	}
	void update(uint32_t time)
	{
		// clear
		for(int i = 0; i < size; i++)
			for(int j = 0; j < bucket_size; j++)
			{
				if(id[i][j] != 0 && timestamp[i][j] != (unsigned)-1 && time - timestamp[i][j] > window_num)
				{
					counter[0][i][j] = make_pair(0,0);
					counter[1][i][j] = make_pair(0,0);
					id[i][j] = 0;
					timestamp[i][j] = -1;
				}
				if(id[i][j] != 0 && calc_ratio(counter[0][i][j]) < screen_layer_threshold && calc_ratio(counter[1][i][j]) < screen_layer_threshold)
				{
					counter[0][i][j] = make_pair(0,0);
					counter[1][i][j] = make_pair(0,0);
					id[i][j] = 0;
					timestamp[i][j] = -1;
				}
			} 
		// find burst
		for(int i = 0; i < size; i++)
			for(int j = 0; j < bucket_size; j++)
			{
				if(id[i][j] == 0)
					continue;
				if(calc_ratio(counter[flag][i][j]) <= calc_ratio(counter[flag ^ 1][i][j]) / lambda && (int)timestamp[i][j] != -1)
				{
					//output burst
					Record.push_back(Burst(timestamp[i][j], time, id[i][j]));
					timestamp[i][j] = -1;
				}
				else if(calc_ratio(counter[flag][i][j]) < m)
					timestamp[i][j] = -1;
				else if(calc_ratio(counter[flag][i][j]) >= lambda * calc_ratio(counter[flag ^ 1][i][j]) && calc_ratio(counter[flag][i][j]) >= m)
					timestamp[i][j] = time;
				
			}
		flag ^= 1;
		for(int i = 0; i < size; i++)
			for(int j = 0; j < bucket_size; j++)
				counter[flag][i][j] = make_pair(0,0);
	}
	bool lookup(uint64_t flow_id, uint32_t flow_time, double weight)
	{	
		int z = bobhash->run((char *)&flow_id, 8) % size;
		for(int j = 0; j < bucket_size; j++)
			if(id[z][j] == flow_id)
			{
				counter[flag][z][j].first += weight;
				counter[flag][z][j].second ++;
				return true;
			}
		return false;
	}
	bool insert(uint64_t flow_id, uint32_t flow_time, pair<double, int> flow_count)
	{
		double mi = oo;
		uint32_t l = 0, f = 0;
		int z = bobhash->run((char *)&flow_id, 8) % size;
		for(int j = 0; j < bucket_size; j++)
		{
			if(id[z][j] == 0)
			{
				l = j;
				break;
			}
			if((int)timestamp[z][j] == -1)
			{ 
				if(f == 0)
				{
					f = 1;
					mi = oo;
				}
				if(calc_ratio(counter[flag][z][j]) < mi)
				{
					mi = calc_ratio(counter[flag][z][j]);
					l = j;
				}
			}
			else if(!f)
			{
				if(calc_ratio(counter[flag][z][j]) < mi)
				{
					mi = calc_ratio(counter[flag][z][j]);
					l = j;
				}
			}
		}
		if(id[z][l] == 0)
		{
			id[z][l] = flow_id;
			timestamp[z][l] = -1;
			counter[flag][z][l] = flow_count; // evict a flow!
			counter[flag ^ 1][z][l] = make_pair(0,0);
			return true;
		}
		if(flow_count > counter[flag][z][l])
		{
			id[z][l] = flow_id;
			timestamp[z][l] = -1;
			counter[flag][z][l] = flow_count; // evict a flow!
			counter[flag ^ 1][z][l] = make_pair(0,0);
			return true;
		}
		return false;
	}
};
class BurstDetector // BurstSketch
{
	public:
	ScreenLayer screen_layer;
	uint64_t last_timestamp;
	Log log;
	int cache_hit;
	BurstDetector(){};
	~BurstDetector()
	{
		cout<<"tot = "<<screen_layer.tot<<endl;
		delete [] screen_layer.counter;
		delete [] screen_layer.id;
		for(int i = 0; i < hash_num; i++)
			delete screen_layer.bobhash[i];
		delete [] log.counter[0];
		delete [] log.counter[1];
		delete [] log.id;
		delete [] log.timestamp;
		delete log.bobhash;
	}
	BurstDetector(int ScreenLayerSize, double ScreenLayerThreshold, int LogSize, double LogThreshold)
	{
		//printf("size=%d",ScreenLayerSize);
		last_timestamp = 0;
		cache_hit = 0;
		// printf("m=%.5f\n",ScreenLayerThreshold);
		screen_layer = ScreenLayer(ScreenLayerSize, ScreenLayerThreshold);
		log = Log(LogSize, LogThreshold, ScreenLayerThreshold);
	}
	void insert(uint64_t id, uint32_t timestamp, double weight, bool print_detail, double realtime)
	{
		if(last_timestamp < timestamp)
		{
			for(uint32_t i = last_timestamp; i < timestamp; i++)
			{
				screen_layer.update();
				log.update(i); 
			}
			last_timestamp = timestamp;
		}
		if(log.lookup(id, timestamp, weight)) {
			cache_hit++;
			return ;
		}
		int ret = screen_layer.insert(id, weight, print_detail, realtime);
		if(ret == -1)
			return ;
		pair<double, int> count = screen_layer.lookup(ret);
		if(log.insert(id, timestamp, count))
			screen_layer.clear(ret);
	}
};
#endif
