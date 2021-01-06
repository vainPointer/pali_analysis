#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

using namespace std;

int pertask = 0;
int remaintask = 0;
vector<vector<int> > sharedmem;
FILE *output;

vector<int> split(const string& s, const string& delim);
void loaddata(char *datafile, vector<vector<int> > &mem);
float jaccard_similarity(vector<int> a, vector<int> b);

int getNcpu();
void *worker(void* args);

int main() {
    time_t start = time(NULL);
    char filename[] = "../tmp/pali_text_index.txt";
    loaddata(filename, sharedmem);
    output = fopen("../tmp/pali_sentence_similar.txt", "w");
    time_t end = time(NULL);
    printf("[+] Load %lu sentences in %f sec.\n", \
            sharedmem.size(), difftime(end, start));

    int ncpu = getNcpu();
    pertask = sharedmem.size() / ncpu;
    remaintask = sharedmem.size() % ncpu;
    printf("[+] Have %d processors, each will run %d tasks\n", ncpu, pertask);

    int       *myid = new int[ncpu];
    pthread_t *tids = new pthread_t[ncpu];

    for (int i = 0; i < ncpu; i++) {
        myid[i] = i;
        pthread_create(tids+i, NULL, worker, (void *)&myid[i]);
    }

    pthread_exit(NULL);

    return 0;
}

void *worker(void* args) {
    int myid = *(int*)args;
    int mybeg = pertask * myid;
    int myend = mybeg + pertask;
    if (myid == 3)
        myend += remaintask;

    printf("[+] I'm thread %d, will run tasks %d ~ %d\n", myid, mybeg, myend);
    float jaccard_score;

    int   total   = sharedmem.size();
    float percent = 0.01; 
    time_t start = time(NULL);
    for (int j = 0; j < total; j++) {
        for (int i = mybeg; i < myend; i++) {
            jaccard_score = jaccard_similarity(sharedmem[i], sharedmem[j]);
            if (jaccard_score > 0.6) {
                fprintf(output, "%d,%d,%f\n", i, j, jaccard_score);                
            }
        }
        if ( (float) j / total * 100 > percent ) {
            time_t end = time(NULL);
            printf("[%d] fininsh %f%% in %f sec.\n", \
                    myid, percent, difftime(end, start));
            percent += 0.01;
        }
    }
    return 0;
}

vector<int> split(const string& s, const string& delim = " ") {
    string substring;
    vector<int> tokens;
    string::size_type lastPos = s.find_first_not_of(delim, 0);
    string::size_type pos = s.find_first_of(delim, lastPos);
    while (string::npos != pos || string::npos != lastPos) {
        substring = s.substr(lastPos, pos - lastPos);
        tokens.push_back(atoi(substring.c_str()));
        lastPos = s.find_first_not_of(delim, pos);
        pos = s.find_first_of(delim, lastPos);
    }
    return tokens;
}

void loaddata(char *datafile, vector<vector<int> > &mem) {
    ifstream file; char buf[1024 * 8];
    file.rdbuf()->pubsetbuf(buf, sizeof buf);
    file.open(datafile);

    vector<int> vline;
    for (string sline; getline(file, sline);) {
        vline = split(sline);
        mem.push_back(vline);
    }
}

inline float jaccard_similarity(vector<int> a, vector<int> b) {
    float longvshort;
    longvshort = ((float) a.size()) / ((float) (a.size()+b.size()));
    if (longvshort < 0.35 || longvshort > 0.65) {
        return 0;
    }

    float jaccard;
    vector<int> inter;
    set_intersection(a.begin(), a.end(), b.begin(), b.end(), 
        inserter(inter, inter.begin()));
    jaccard = (float) inter.size();
    if (jaccard) {
        return jaccard / (a.size()+b.size()-jaccard);
    } else {
        return 0;
    }
}

int getNcpu(){ 
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwNumberOfProcessors;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}
