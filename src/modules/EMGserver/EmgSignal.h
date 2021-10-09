/*
 * Emg Signal
 *
 * Author: Waldez Gomes
 * email:  waldezjr14@gmail.com
 *
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

#ifndef EMG_SIGNAL_H
#define EMG_SIGNAL_H

#include "EmgTcp.h"
#include <yarp/os/all.h>
#include <stdio.h>
#include <algorithm>
#include <cmath>


// #define RMS_WIN_SIZE 112
#define RMS_WIN_SIZE 560
// #define RMS_WIN_SIZE 200


/**
 * @brief The EmgSignal class processess the emg read from the EmgTcp class
 */
class EmgSignal
{
public:
	EmgSignal();
	//~EmgSignal();

	/**
	 * @brief      Determines if configured.
	 *
	 * @return     True if configured, False otherwise.
	 */
	inline bool isConfigured() {return configured_; } 

	/**
	 * @brief      Sets the sample.
	 *
	 * @param[in]  sample  The sample
	 * @param[in]  ite     The ite
	 */
	void setSample(EmgData sample, int ite);

	/**
	 * @brief      { function_description }
	 *
	 * @return     { returns the rms value of all the 16 sensors }
	 */
	std::vector<double> rms(std::vector<double> curIn);

	std::vector<double> hpfButterworth(std::vector<double> curIn);

	std::vector<double> butterworth(std::vector<double> curIn);

	std::vector<double> rectify(std::vector<double> curIn);
	
	/**
	 * @brief      { Applies lpf, rectify, then hpf to the signal of all 16 sensors }
	 *
	 * @return     { returns the filtered signal of all 16 sensors }
	 */
	std::vector<double> fullFilter(void);

	/**
	 * @brief      { function_description }
	 */
	void clearRmsWin(void);

	/**
	 * @brief      { function_description }
	 *
	 * @param[in]  emg   The emg
	 *
	 * @return     { description_of_the_return_value }
	 */
	std::vector<double> normalizeEmgData(std::vector<double> emg);

	/**
	 * @brief      { Calculates ICC for the pairs 0-1 2-3 4-5 6-7 8-9 10-11 12-13 14-15
	 * so the sensor pairs  must be properly placed for this function to work }
	 *
	 * @param[in]  normalized  The normalized
	 *
	 * @return     { only EIGHT (8) ICC values }
	 */
	std::vector<double> icc(std::vector<double> normalized);

/**
 * @brief      Change the icc resolution to 'NLevels'.
 * Those are the values thats should be used in the control loop
 *
 * @param[in]  icc      The icc
 * @param[in]  NLevels  The nlevels
 *
 * @return     (even more) Discretized ICC 
 */
	std::vector<double> iccLevel(std::vector<double> icc, int NLevels = 4);
	
private:

	//vector of vectors that store the rms window
	std::vector< std::vector<double> > rmsWins_;


	/**
	 * xv: first 4 positions
	 * yv: last 4 postions
	 */
	std::vector< std::vector<double> > filterBuf_;

	/**
	 * xv: first 4 positions
	 * yv: last 4 postions
	 */
	std::vector< std::vector<double> > hpFilterBuf_;

	double filterGain_;
/**
 * maximum value of contraction for each one of the 16 sensors
 */
	std::vector<double> mvc_;
	std::vector<double> lpfA_,lpfB_, hpfA_, hpfB_;

	//current sample
	EmgData sample_;

	//current iteration (step in time)
	int stepTime_;

	bool configured_ = false;
	
};
#endif
