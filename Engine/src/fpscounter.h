/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef FPSCOUNTER_H
#define FPSCOUNTER_H

#include <vector>

// Simple Moving Average over default of 60 samples
class FPSCounter {
public:
    FPSCounter(unsigned int samples = 60) :
        index_(0),
        moving_sum_(0.0),
        num_samples_(samples),
        samples_(samples, 0.0) { }

    double GetAverageFPS(double new_sample) {
        moving_sum_ -= samples_[index_];
        moving_sum_ += new_sample;
        samples_[index_++] = new_sample;
        if (index_ == num_samples_) index_ = 0;
        return moving_sum_ / num_samples_;
    }

protected:
    int index_;
    double moving_sum_;
    int num_samples_;
    std::vector<double> samples_;
};

#endif // FPSCOUNTER_H
