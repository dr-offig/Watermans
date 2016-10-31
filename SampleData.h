/*
 * SampleData.h
 *
 *  Created on: Nov 5, 2014
 *      Author: Victor Zappi
 */

#ifndef SAMPLEDATA_H_
#define SAMPLEDATA_H_

// User defined structure to pass between main and rendered complex data retrieved from file
struct SampleData {
	float *samples;	// Samples in file
	int sampleLen;	// Total number of samples
};



#endif /* SAMPLEDATA_H_ */
