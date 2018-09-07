#include "emgutils.h"
namespace EmgUtils {

    void readValue(ResourceFinder &rf, string s, double &v, double vdefault)
    {
        if(rf.check(s.c_str()))
        {
            v = rf.find(s.c_str()).asDouble();
        }
        else
        {
            v = vdefault;
            cout<<"Could not find parameters for "<<s<<endl
                <<"Setting default "<<vdefault<<endl;
        }
    }
    //---------------------------------------------------------
    void readParams(ResourceFinder &rf, string s, std::vector<int> &v)
    {
        if(rf.check(s.c_str()))
        {
            Bottle &grp = rf.findGroup(s.c_str());
            for (int i=0; !grp.get(1+i).isNull(); i++)
                v.push_back( grp.get(1+i).asInt() );
        }
        else
        {
            cout<<"Could not find parameters for "<<s<<endl
                <<"Setting everything to zero by default"<<endl;
        }
    }

    void readParams(ResourceFinder &rf, string s, std::vector<std::pair<int,int>> &v)
    {
        if(rf.check(s.c_str()))
        {
            Bottle &grp = rf.findGroup(s.c_str());
            for (int i=0; !grp.get(2+i).isNull(); i=i+2){

                std::pair<int,int> aux;
                aux.first = grp.get(1+i).asInt();
                aux.second = grp.get(2+i).asInt();
                v.push_back(aux);

            }
        }
        else
        {
            cout<<"Could not find parameters for "<<s<<endl
                <<"Setting everything to zero by default"<<endl;
        }
    }
    //---------------------------------------------------------
    void readParams(ResourceFinder &rf, string s, Vector &v, int len)
    {
        v.resize(len,0.0);
        cout << "s: " << s << endl;
        if(rf.check(s.c_str()))
        {
            Bottle &grp = rf.findGroup(s.c_str());
            for (int i=0; i<len; i++)
                v[i]=grp.get(1+i).asDouble();
            DSCPAs(s,v);
        }
        else
        {
            cout<<"Could not find parameters for "<<s<<endl
                <<"Setting everything to zero by default"<<endl;
        }
    }

    //---------------------------------------------------------

    void readValue(ResourceFinder &rf, string s, int &v, int vdefault)
    {
        if(rf.check(s.c_str()))
        {
            v = rf.find(s.c_str()).asInt();
        }
        else
        {
            v = vdefault;
            cout<<"Could not find parameters for "<<s<<endl
                <<"Setting default "<<vdefault<<endl;
        }
    }

    void readValue(ResourceFinder &rf, string s, string &v, string vdefault)
    {
        if(rf.check(s.c_str()))
        {
            v = rf.find(s.c_str()).asString();
        }
        else
        {
            v = vdefault;
            cout<<"Could not find parameters for "<<s<<endl
                <<"Setting default "<<vdefault<<endl;
        }
    }

    void readValue(ResourceFinder &rf, string s, bool &v, bool vdefault)
    {
        if(rf.check(s.c_str()))
        {
            v = rf.find(s.c_str()).asBool();
        }
        else
        {
            v = vdefault;
            cout<<"Could not find parameters for "<<s<<endl
                <<"Setting default "<<vdefault<<endl;
        }
    }

}


