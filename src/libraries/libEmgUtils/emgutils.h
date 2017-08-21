#ifndef EMGUTILS_H
#define EMGUTILS_H

#include <yarp/sig/Vector.h>
#include <yarp/os/all.h>
#include <string>
#include <stdio.h>
#include <deque>
#include <map>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;

// utils for printing parameters
#define DSCPA(V) cout<<"  "<< #V <<" : "<<V<<endl;
#define DSCPAv(V) cout<<"  "<< #V <<" : "<<V.toString()<<endl;
#define DSCPAs(S,V) cout<<"  "<< S <<" : "<<V.toString()<<endl;
#define DSCPAd(S,V) cout<<"  "<< S <<" : "<<V<<endl;
#define DSCPAstdvec(V)  std::cout << "  " << #V << " :"; for(const auto& vi:V) {std::cout << " " << vi; } std::cout << std::endl;
#define DSCPAstdvecpair(V)  std::cout << "  " << #V << " :"; for(const auto& vi:V) {std::cout << " (" << vi.first <<"," << vi.second << ") "; } std::cout << std::endl;
#define DSCPAstdMap(V)  std::cout << "  " << #V << " :"; for(const auto& vi:V) {std::cout << " id is " << vi.first << " val is "<<vi.second; } std::cout << std::endl;

namespace EmgUtils {

    void readParams(ResourceFinder &rf, string s, std::vector<int> &v);
    void readParams(ResourceFinder &rf, string s, std::vector<std::pair<int, int> > &v);
    void readParams(ResourceFinder &rf, string s, Vector &v, int len);

    void readValue(ResourceFinder &rf, string s, double &v, double vdefault);
    void readValue(ResourceFinder &rf, string s, int &v, int vdefault);
    void readValue(ResourceFinder &rf, string s, string &v, string vdefault);
    void readValue(ResourceFinder &rf, string s, bool &v, bool vdefault);

}
#endif // EMGUTILS_H
