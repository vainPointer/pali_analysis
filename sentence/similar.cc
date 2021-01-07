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

#define LSTHRD 0.35 // threshold for longvshort
#define JATHRD 0.69 // threshold for jaccard_similarity

int pertask = 0;
int remaintask = 0;
vector<vector<int> > sharedmem;
FILE *output;

vector<int> split(const string& s, const string& delim);
void loaddata(char *datafile, vector<vector<int> > &mem);
inline float jaccard_similarity(vector<int> a, vector<int> b);

int getNcpu();
void *worker(void* args);

int main() {
    time_t start = time(NULL);
    output = fopen("../tmp/pali_sentence_similar.txt", "w");
    char filename[] = "../tmp/pali_text_index.txt";
    loaddata(filename, sharedmem);
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
            if (jaccard_score > JATHRD) {
                fprintf(output, "%d,%d,%f\n", i+1, j+1, jaccard_score);                
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
    // The following is not the main functional of split
    sort(tokens.begin(), tokens.end());
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
    int len_a = a.size();
    int len_b = b.size();
    float longvshort = ((float) len_a ) / (len_a + len_b);
    if (longvshort < LSTHRD || longvshort > 1-LSTHRD) {
        return 0;
    }

    int i = 0, j = 0;
    float jaccard;
    while ( (i < len_a) && (j < len_b) ) {
        if      ( a[i] < b[j] ) i++;
        else if ( b[j] < a[i] ) j++;
        else { jaccard += 1; i++; j++; }
    }
    if (jaccard) {
        return jaccard / (len_a + len_b - jaccard);
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

