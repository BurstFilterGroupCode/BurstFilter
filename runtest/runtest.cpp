#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

void run_and_wait(const char *s) {
    printf("Start running %s\n", s);
    pid_t pid;
    pid = fork();
    assert(pid != -1);
    if(pid == 0) {
        system(s);
        exit(0);
    }
    int status;
    pid_t wt = waitpid(pid, &status, WUNTRACED);
    assert(wt == pid);
    printf("Finish running %s\n", s);
}

double get_res(const char *resfile) {
    FILE *fres = fopen(resfile, "r");
    double res;
    fscanf(fres, "%lf", &res);
    return res;
}

pair <double, int> get_ress (const char *resfile) {
    FILE *fres = fopen(resfile, "r");
    pair <double, int> ress;
    fscanf(fres, "%lf %d", &ress.first, &ress.second);
    return ress;
}

void print_list(double *t, const char *pr, int len, const char *name) {
    printf("%s",name);
    printf(" = [");
    for(int i = 0; i < len; i++) {
        printf(pr, t[i]);
        if(i == len - 1) {
            printf("]\n");
        }
        else printf(", ");
    }
}

void print_list(int *t, int len, const char *name) {
    printf("%s",name);
    printf(" = [");
    for(int i = 0; i < len; i++) {
        printf("%d", t[i]);
        if(i == len - 1) {
            printf("]\n");
        }
        else printf(", ");
    }
}

void print_listf(FILE* f,int *t, int len, const char *name) {
    fprintf(f, "%s",name);
    fprintf(f, " = [");
    for(int i = 0; i < len; i++) {
        fprintf(f, "%d", t[i]);
        if(i == len - 1) {
            fprintf(f, "]\n");
        }
        else fprintf(f, ", ");
    }
    fflush(f);
}

void print_listf(FILE* f,double *t, const char *pr, int len, const char *name) {
    fprintf(f, "%s",name);
    fprintf(f, " = [");
    for(int i = 0; i < len; i++) {
        fprintf(f, pr, t[i]);
        if(i == len - 1) {
            fprintf(f, "]\n");
        }
        else fprintf(f, ", ");
    }
    fflush(f);
}

// #define STRAWMAN
// #define DEFINITION_PARAMS
// #define ACCURACY_TEST
// #define COMPARE_TEST
// #define CACHE_TEST
#define PARAMS_TEST

int main() {

#ifdef STRAWMAN
    if(true)
    {
        run_and_wait("./run_correct 0.1 0.005 1");
        double flush_tm[5] = {0.01, 0.05, 0.1, 0.5, 1};
        int mem[5];
        double f1[5];
        for(int i=0;i<5;i++) {
            char cmdline[1000];
            sprintf(cmdline, "./run_strawman 0.1 0.005 1 %.5lf", flush_tm[i]);
            run_and_wait(cmdline);
            mem[i] = get_res("tmp/strawman_mem.res");
            f1[i] = get_res("tmp/strawman.res");
        }
        print_list((double*)f1, "%.5lf", 5, "strawman_f1");
        print_list((int*)mem, 5, "strawman_mem");
    }
#endif

#ifdef DEFINITION_PARAMS
    //count the burst on different definitions
    if(false)
    {
        //./run_correct 0.1 0.005 50
        double testp[10] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
        int burstcnt[10];
        for(int i = 0; i < 9; i++) {
            char cmdline[1000];
            sprintf(cmdline , "./run_correct %.1lf 0.005 50", testp[i]);
            run_and_wait(cmdline);
            int res = get_res("tmp/burstcount_unique.txt");
            burstcnt[i] = res;
        }
        print_list((double*)testp, "%.1lf", 9, "definition_p");
        print_list((int*)burstcnt, 9, "burstcnt_p");
    }
    if(false)
    {
        double testlim[10] = {0.001, 0.002, 0.005, 0.01, 0.02, 0.05, 0.1};
        int burstcnt[10];
        for(int i = 0; i < 7; i++) {
            char cmdline[1000];
            sprintf(cmdline , "./run_correct 0.1 %.3f 50", testlim[i]);
            run_and_wait(cmdline);
            int res = get_res("tmp/burstcount_unique.txt");
            burstcnt[i] = res;
        }
        print_list((double*)testlim, "%.3lf", 7, "definition_lim");
        print_list((int*)burstcnt, 7, "burstcnt_lim");
    }
    if(false)
    {
        int testnum[10] = {10, 20, 50, 75, 100, 150, 200};
        int burstcnt[10];
        for(int i = 0; i < 7; i++) {
            char cmdline[1000];
            sprintf(cmdline , "./run_correct 0.1 0.005 %d", testnum[i]);
            run_and_wait(cmdline);
            int res = get_res("tmp/burstcount_unique.txt");
            burstcnt[i] = res;
        }
        print_list((int*)testnum, 7, "definition_num");
        print_list((int*)burstcnt, 7, "burstcnt_num");
    }
    if(true) {
        run_and_wait("./run_correct 0.1 0.005 50");
        ifstream fin("tmp/correct.txt");
        int cnt[23];
        memset(cnt, 0, sizeof(cnt));
        unsigned id;
        int packcnt;
        double tm;
        while(fin>>id>>packcnt>>tm) {
            cnt[(int)floor(tm)]++;
        }
        print_list((int*)cnt, 23, "burst_cnt");
    }
#endif

#ifdef ACCURACY_TEST
    if(false)
    {
        run_and_wait("./run_correct 0.1 0.005 1");
        const double memk[10] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
        double f1sc[10];
        double throughput[10];
        for(int i = 0; i < 9; i++) {
            char cmdline[1000];
            sprintf(cmdline , "./run_newsketch 50 %.1f 0.1 0.005 1", memk[i]);
            run_and_wait(cmdline);
            f1sc[i] = get_res("./tmp/newsketch.res");
            throughput[i] = get_res("./tmp/newsketch_throughput.res") / 10000000;
            throughput[i] = 1.0 / throughput[i] / 1000;
        }
        print_list((double*)memk, "%.1f", 9, "mem_stg1_stg2");
        print_list((double*)f1sc, "%.5f", 9, "f1_score_mem_stg1_stg2");
        print_list((double*)throughput, "%.8f", 9, "throughput_mem_stg1_stg2");
    }
    //run_single_test
    if(false)
    {
        run_and_wait("./run_correct 0.1 0.005 1");
        run_and_wait("./run_newsketch 20 0.7 0.1 0.005 1");
        double f1 = get_res("./tmp/newsketch.res");
        printf("f1score = %.5f\n", f1);
        double throughput = get_res("./tmp/newsketch_throughput.res") / 10000000;
        printf("throughput = %.5f\n", 1.0 / throughput / 1000);
    }
    /*
    CAIDA:
    mem = 20, k = 0.7
    stg1 hashcnt = 1, 2, 4, 8
    f1sc = 0.72549, 0.86476, 0.82087, 0.74325
    thrput = 1.36333, 1.09999, 0.87712, 0.66823

    stg2 depth = 2, 4, 8, 16
    f1sc = 0.85600, 0.86476, 0.86973, 0.87129
    thrput = 1.40944, 1.39276, 1.37665, 1.35410

    Yahoo_G4
    stg hashcnt = 1, 2, 4, 8
    f1sc = 0.89792, 0.96217, 0.98476, 0.80002
    thrput = 1.77085, 1.33887, 0.98270, 0.69142

    stg2 depth = 2, 4, 8, 16
    f1sc = 0.96028, 0.96217, 0.96292, 0.96261
    thrput = 1.35520, 1.33887, 1.33245, 1.29719
    */
#endif

#ifdef COMPARE_TEST
    // compare newsketch and topicsketch and other algorithms
    {
        run_and_wait("./run_correct 0.1 0.005 1");
        const int mem[7] = {10, 20, 30, 50, 70, 100, 150};
        double newsketch[7], topicsketch[7], burstsketch[7], burstyevent[7];
        double thpnewsk[7], thptopsk[7], thpbstsk[7], thpbstev[7];
        for(int i = 0; i < 7; i++) {
            char cmdline[1000];
            sprintf(cmdline , "./run_newsketch %d 0.7 0.1 0.005 1", mem[i]);
            run_and_wait(cmdline);
            newsketch[i] = get_res("./tmp/newsketch.res");
            thpnewsk[i] = get_res("./tmp/newsketch_throughput.res") / 200000;
            /*sprintf(cmdline , "./run_topicsketch %d", mem[i]);
            run_and_wait(cmdline);
            topicsketch[i] = get_res("./tmp/topicsketch.res");
            thptopsk[i] = get_res("./tmp/topicsketch_throughput.res") / 200000;
            sprintf(cmdline , "./run_burstsketch %d", mem[i]);
            run_and_wait(cmdline);
            burstsketch[i] = get_res("./tmp/burstsketch.res");
            thpbstsk[i] = get_res("./tmp/burstsketch_throughput.res") / 200000;
            sprintf(cmdline , "./run_burstyevent %d", mem[i]);
            run_and_wait(cmdline);
            burstyevent[i] = get_res("./tmp/burstyevent.res");
            thpbstev[i] = get_res("./tmp/burstyevent_throughput.res") / 200000;*/

            thpnewsk[i] = 1.0 / thpnewsk[i] / 1000;
            /*thptopsk[i] = 1.0 / thptopsk[i] / 1000;
            thpbstsk[i] = 1.0 / thpbstsk[i] / 1000;
            thpbstev[i] = 1.0 / thpbstev[i] / 1000;*/
        }
        print_list((int*)mem, 7, "mem_compare");
        print_list((double*)newsketch, "%.5f", 7, "newsketch_f1_compare");
        /*print_list((double*)topicsketch, "%.5f", 7, "topicsketch_f1_compare");
        print_list((double*)burstsketch, "%.5f", 7, "burstsketch_f1_compare");
        print_list((double*)burstyevent, "%.5f", 7, "burstyevent_f1_compare");*/

        print_list((double*)thpnewsk, "%.5f", 7, "newsketch_thp_compare");
        /*print_list((double*)thptopsk, "%.5f", 7, "topicsketch_thp_compare");
        print_list((double*)thpbstsk, "%.5f", 7, "burstsketch_thp_compare");
        print_list((double*)thpbstev, "%.5f", 7, "burstyevent_thp_compare");*/
    }
#endif

#ifdef CACHE_TEST
    {
        const int mem[20] = {1, 2, 5, 7, 10, 20, 30, 50, 70, 100, 200, 500, 1000};
        int cachecnt[20];
        pair <double, int> res_lru[20];
        pair <double, int> res_lfu[20];
        pair <double, int> res_sketch[20];
        for(int i = 0; i < 13; i++) {
            char cmdline[1000];
            sprintf(cmdline , "./run_newsketch %d 0.7 0.9 0.1 1", mem[i]);
            run_and_wait(cmdline);
            int cachesz = get_res("tmp/cachesz.tmp");
            cachecnt[i] = cachesz;
            run_and_wait("./chk_cache tmp/newsketch_cache.log >tmp/newsketch_cache.res");
            res_sketch[i] = get_ress("tmp/newsketch_cache.res");

            sprintf(cmdline , "./chk_lru %d >tmp/lru_cache.res", cachesz);
            run_and_wait(cmdline);
            res_lru[i] = get_ress("tmp/lru_cache.res");

            sprintf(cmdline , "./chk_lfu %d >tmp/lfu_cache.res", cachesz);
            run_and_wait(cmdline);
            res_lfu[i] = get_ress("tmp/lfu_cache.res");
            printf("%d %.5f %.5f %.5f %d %d %d\n", cachesz, res_lfu[i].first, res_lru[i].first, res_sketch[i].first,
                res_lfu[i].second, res_lru[i].second, res_sketch[i].second);
        }
        // print_list((int*)cachecnt, 13, "cache_cnt");
        // print_list((double*)res_sketch, "%.5f", 13, "sketch_hitrt");
        // print_list((double*)res_lru, "%.5f", 13, "lru_hitrt");
        // print_list((double*)res_lfu, "%.5f", 13, "lfu_hitrt");
    }
#endif

#ifdef PARAMS_TEST
    if(false)
    {
        run_and_wait("./run_correct 0.1 0.005 1");
        const double memk[10] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
        const int mem[20] = {1, 2, 5, 7, 10, 20, 30, 50, 70, 100, 200, 500, 1000};
        double f1sc[100];
        double throughput[100];
        FILE* f = fopen("tmp/param_test_memk.res", "w");
        for(int i = 0; i < 9; i++) {
            fprintf(f, "memk = %.1f\n", memk[i]);
            fflush(f);
            char cmdline[1000];
            for(int j = 0; j < 13; j++)
            {
                sprintf(cmdline , "./run_newsketch %d %.1f 0.1 0.005 0.01 1", mem[j], memk[i]);
                run_and_wait(cmdline);
                f1sc[j] = get_res("./tmp/newsketch.res");
                throughput[j] = get_res("./tmp/newsketch_throughput.res") / 10000000;
                throughput[j] = 1.0 / throughput[j] / 1000;
            }
            print_listf(f, (int*)mem, 13, "mem");
            print_listf(f, (double*)f1sc, "%.5f", 13, "f1_score");
            print_listf(f, (double*)throughput, "%.8f", 13, "throughput");
        }
    }
    if(false)
    {
        const double param[10] = {0.02, 0.04, 0.06, 0.08, 0.10, 0.12, 0.14, 0.16, 0.18};
        const int mem[20] = {1, 2, 5, 7, 10, 20, 30, 50, 70, 100, 200, 500, 1000};
        double f1sc[100];
        double throughput[100];
        FILE* f = fopen("tmp/param_test_alpha.res", "w");
        for(int i = 0; i < 9; i++) {
            char cmdline[1000];
            sprintf(cmdline, "./run_correct %.3f 0.005 1", param[i]);
            run_and_wait(cmdline);
            fprintf(f, "alpha = %.3f\n", param[i]);
            fflush(f);
            for(int j = 0; j < 13; j++)
            {
                sprintf(cmdline , "./run_newsketch %d 0.4 %.3f 0.005 0.01 1", mem[j], param[i]);
                run_and_wait(cmdline);
                f1sc[j] = get_res("./tmp/newsketch.res");
                throughput[j] = get_res("./tmp/newsketch_throughput.res") / 10000000;
                throughput[j] = 1.0 / throughput[j] / 1000;
            }
            print_listf(f, (int*)mem, 13, "mem");
            print_listf(f, (double*)f1sc, "%.5f", 13, "f1_score");
            print_listf(f, (double*)throughput, "%.8f", 13, "throughput");
        }
    }
    if(false)
    {
        run_and_wait("./run_correct 0.1 0.005 1");
        const double param[10] = {1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8};
        const int mem[20] = {1, 2, 5, 7, 10, 20, 30, 50, 70, 100, 200, 500, 1000};
        double f1sc[100];
        double throughput[100];
        FILE* f = fopen("tmp/param_test_weaklim.res", "w");
        for(int i = 0; i < 9; i++) {
            fprintf(f, "weaklim / lim = %.4f\n", param[i]);
            fflush(f);
            char cmdline[1000];
            for(int j = 0; j < 13; j++)
            {
                sprintf(cmdline , "./run_newsketch %d 0.4 0.1 0.005 %.4f 1", mem[j], 0.005 * param[i]);
                run_and_wait(cmdline);
                f1sc[j] = get_res("./tmp/newsketch.res");
                throughput[j] = get_res("./tmp/newsketch_throughput.res") / 10000000;
                throughput[j] = 1.0 / throughput[j] / 1000;
            }
            print_listf(f, (int*)mem, 13, "mem");
            print_listf(f, (double*)f1sc, "%.5f", 13, "f1_score");
            print_listf(f, (double*)throughput, "%.8f", 13, "throughput");
        }
    }
    if(false)
    {
        const double param[10] = {0.002, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008, 0.009, 0.01};
        const int mem[20] = {1, 2, 5, 7, 10, 20, 30, 50, 70, 100, 200, 500, 1000};
        double f1sc[100];
        double throughput[100];
        FILE* f = fopen("tmp/param_test_lim.res", "w");
        for(int i = 0; i < 9; i++) {
            char cmdline[1000];
            sprintf(cmdline, "./run_correct 0.1 %.4f 1", param[i]);
            run_and_wait(cmdline);
            fprintf(f, "lim = %.4f\n", param[i]);
            fflush(f);
            for(int j = 0; j < 13; j++)
            {
                sprintf(cmdline , "./run_newsketch %d 0.4 0.1 %.4f %.4f 1", mem[j], param[i], param[i]* 2);
                run_and_wait(cmdline);
                f1sc[j] = get_res("./tmp/newsketch.res");
                throughput[j] = get_res("./tmp/newsketch_throughput.res") / 10000000;
                throughput[j] = 1.0 / throughput[j] / 1000;
            }
            print_listf(f, (int*)mem, 13, "mem");
            print_listf(f, (double*)f1sc, "%.5f", 13, "f1_score");
            print_listf(f, (double*)throughput, "%.8f", 13, "throughput");
        }
    }
    if(true)
    {
        run_and_wait("./run_correct 0.1 0.005 1");
        const int param[10] = {1, 2, 3, 4, 5, 6, 7};
        const int mem[20] = {1, 2, 5, 7, 10, 20, 30, 50, 70, 100, 200, 500, 1000};
        double f1sc[100];
        double throughput[100];
        FILE* f = fopen("tmp/param_test_hash2.res", "w");
        for(int i = 0; i < 7; i++) {
            fprintf(f, "hash2 = %d\n", param[i]);
            fflush(f);
            char cmdline[1000];
            for(int j = 0; j < 13; j++)
            {
                sprintf(cmdline , "./run_newsketch %d 0.4 0.1 0.005 0.01 %d", mem[j], param[i]);
                run_and_wait(cmdline);
                f1sc[j] = get_res("./tmp/newsketch.res");
                throughput[j] = get_res("./tmp/newsketch_throughput.res") / 10000000;
                throughput[j] = 1.0 / throughput[j] / 1000;
            }
            print_listf(f, (int*)mem, 13, "mem");
            print_listf(f, (double*)f1sc, "%.5f", 13, "f1_score");
            print_listf(f, (double*)throughput, "%.8f", 13, "throughput");
        }
    }
#endif
}
