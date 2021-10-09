#include "EmgSignal.h"

EmgSignal::EmgSignal(){

    std::vector< std::vector<double> > aux(16, std::vector<double>(RMS_WIN_SIZE,0));

    rmsWins_ = aux;

    // std::vector< std::vector<double> > aux2(16, std::vector<double>(8,0));
    std::vector< std::vector<double> > aux2(16, std::vector<double>(6,0)); // new filter is of second order

    filterBuf_ = aux2;
    hpFilterBuf_ = aux2;

    mvc_ = std::vector<double>(16,1);

    mvc_[0] = 0.0012;
    mvc_[1] = 0.0003;

    // b = [4.94793813e-05 9.89587626e-05 4.94793813e-05]
    // a = [ 1.         -1.98000568  0.9802036 ]
    lpfA_ = std::vector<double>(3,1);
    lpfB_ = std::vector<double>(3,1);

    hpfA_ = std::vector<double>(3,1);
    hpfB_ = std::vector<double>(3,1);

    // setting low pass filter
    lpfA_[0] = 1;
    lpfA_[1] = -1.98000568;
    lpfA_[2] = 0.9802036;
    
    lpfB_[0] = 4.94793813e-05;
    lpfB_[1] = 9.89587626e-05;
    lpfB_[2] = 4.94793813e-05;

    //setting high pass filter
    // b = [ 0.98020358 -1.96040716  0.98020358]
    // a = [ 1.         -1.96001523  0.9607991 ]
    hpfA_[0] = 1;
    hpfA_[1] = -1.96001523;
    hpfA_[2] = 0.9607991;

    hpfB_[0] = 0.98020358;
    hpfB_[1] = -1.96040716;
    hpfB_[2] = 0.98020358;

}

std::vector<double> EmgSignal::butterworth(std::vector<double> curIn){

    int k=0;
    std::vector<double> filteredVal(16,0) ;
    for (std::vector< std::vector<double> >::iterator it = filterBuf_.begin() ; it != filterBuf_.end(); ++it){

        double* xv = &((*it)[0]);
        double* yv = &((*it)[4]);


        //lpf like in Peternel 2017

        // first you shift the entries
        xv[0] = xv[1];//x[n-2]
        xv[1] = xv[2];//x[n-1]
        xv[2] = curIn[k];//x[n]

        yv[0] = yv[1];//y[n-2]
        yv[1] = yv[2];//y[n-1]

        yv[2] = lpfB_[0] * xv[2] + lpfB_[1] * xv[1] + lpfB_[1] * xv[0] 
                    -lpfA_[1] * yv[1] - lpfA_[2] * yv[0]; 

        filteredVal[k] = yv[2] / lpfA_[0];

        k++;
// Coefficients from Python - sci py
// f_s = 1111    # Sample frequency in Hz
// f_c = 2.5     # Cut-off frequency in Hz
// order = 2    # Order of the butterworth filter
    }
    return filteredVal;
}

std::vector<double> EmgSignal::hpfButterworth(std::vector<double> curIn){
    int k=0;
    std::vector<double> filteredVal(16,0) ;
    for (std::vector< std::vector<double> >::iterator it = hpFilterBuf_.begin() ; it != hpFilterBuf_.end(); ++it){

        double* xv = &((*it)[0]);
        double* yv = &((*it)[4]);


        //hpf like in Peternel 2017

        // first you shift the entries
        xv[0] = xv[1];//x[n-2]
        xv[1] = xv[2];//x[n-1]
        xv[2] = curIn[k];//x[n]

        yv[0] = yv[1];//y[n-2]
        yv[1] = yv[2];//y[n-1]

        yv[2] = hpfB_[0] * xv[2] + hpfB_[1] * xv[1] + hpfB_[1] * xv[0] 
                    -hpfA_[1] * yv[1] - hpfA_[2] * yv[0]; 

        filteredVal[k] = yv[2] / hpfA_[0];

        k++;
// Coefficients from Python - sci py
// f_s = 1111    # Sample frequency in Hz
// f_c = 5     # Cut-off frequency in Hz
// order = 2    # Order of the butterworth filter
    }
    return filteredVal;
}

std::vector<double> EmgSignal::rectify(std::vector<double> curIn){
    for(int i =0; i<=curIn.size(); i++){
        if(curIn[i]<0.0){
            curIn[i] = -curIn[i];
        }
    }

    return curIn;
}

std::vector<double> EmgSignal::fullFilter(void){
    // this function assumes that the new sample has already been captured by the time it is called.

    // apply hpf
    std::vector<double> aux(sample_.data.begin(),sample_.data.end());
    std::vector<double> fooFil;
    // fooFil = hpfButterworth(aux);
    // rectify and rms window
    fooFil = rms(aux);
    // apply hpf
    // fooFil = hpfButterworth(fooFil);
    // apply lpf
    fooFil = butterworth(fooFil);
    return fooFil;

}

void EmgSignal::setSample(EmgData sample, int ite){

    sample_ = sample;
    stepTime_ = ite;

    configured_ = true;

}



void EmgSignal::clearRmsWin(){

    rmsWins_.clear();
    std::vector< std::vector<double> > aux(16, std::vector<double>(RMS_WIN_SIZE,0));
    rmsWins_ = aux;

}

std::vector<double> EmgSignal::rms(std::vector<double> curIn){


    // Spin/add RMS window
    int k = 0;// k iterates through the sample,'it' over the rms windows of each sensor (and 'i' through time)
    for (std::vector< std::vector<double> >::iterator it = rmsWins_.begin() ; it != rmsWins_.end(); ++it){

        if((stepTime_+1) <= RMS_WIN_SIZE){
            // (*it)[stepTime_] = sample_.data[k];
            (*it)[stepTime_] = curIn[k];

        }
        else{
            std::rotate((*it).begin(), (*it).begin()+1, (*it).end());
            (*it).pop_back();
            // (*it).push_back(sample_.data[k]);
            (*it).push_back(curIn[k]);
        }

        k++;
    }

    double rmsVal[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    //iterate through the sample...
    int j = 0;
    // for (std::vector<float>::iterator it = sample_.data.begin() ; it != sample_.data.end(); ++it){
    for (std::vector<double>::iterator it = curIn.begin() ; it != curIn.end(); ++it){

        double value = *it;


        if((stepTime_+1) > RMS_WIN_SIZE){
            //compute RMS of rmsWin and insert it into filteredImEmgData
            //
            //Now we shall iterate  through the sample window
            for (std::vector<double>::iterator rmsIt = rmsWins_[j].begin() ; rmsIt != rmsWins_[j].end(); ++rmsIt){
                rmsVal[j] += (*rmsIt)*(*rmsIt);
            }
            rmsVal[j] = rmsVal[j] / rmsWins_[0].size(); //all rmsWins vectors should have the same size
            rmsVal[j] = sqrt(rmsVal[j]);

        }

        //imData.filteredImEmgData.push_back(rmsVal[j]);
        j++;

    }
    std::vector<double> aux(std::begin(rmsVal), std::end(rmsVal));
    return aux;


}

std::vector<double> EmgSignal::normalizeEmgData(std::vector<double> emg){

    int count = 0;
    double aux;
    std::vector<double> normalized;

    for(auto i : emg){

        aux = i / mvc_[count];
        normalized.push_back(aux);

        count++;
    }

    return normalized;

}


std::vector<double> EmgSignal::icc(std::vector<double> normalized){

    std::vector<double> iccVec;
    for (int i = 0; i < 16; i = i+2)
    {
        if( normalized[i] <= normalized[i+1] ){
            iccVec.push_back(normalized[i]);
        }
        else{
            iccVec.push_back(normalized[i+1]);
        }
    }

    return iccVec;


}

std::vector<double> EmgSignal::iccLevel(std::vector<double> icc, int NLevels){


    std::vector<double> iccLevels;

    double div;

    int count = 0;
    for(auto iccValue: icc){

        div = 1 / NLevels;

        if(NLevels == 4){

            if(iccValue <= div){
                iccValue = div;
            }

            else if(iccValue <= 2 * div && iccValue > 1 * div){
                iccValue = 2*div;
            }
            else if(iccValue <= 3 * div && iccValue > 2 * div){
                iccValue = 3*div;
            }
            else if(iccValue <= 4 * div && iccValue > 3 * div){
                iccValue = 4*div;
            }
        }

        if(NLevels == 3){

            if(iccValue <= div){
                iccValue = div;
            }

            else if(iccValue <= 2 * div && iccValue > 1 * div){
                iccValue = 2*div;
            }
            else if(iccValue <= 3 * div && iccValue > 2 * div){
                iccValue = 3*div;
            }

        }

        if(NLevels == 2){

            if(iccValue <= div){
                iccValue = div;
            }

            else if(iccValue <= 2 * div && iccValue > 1 * div){
                iccValue = 2*div;
            }
        }

    }


}
