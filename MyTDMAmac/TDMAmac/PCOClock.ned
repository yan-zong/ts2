package mytdmamac.TDMAmac;

simple PCOClock
{
    parameters:
        volatile double offset;
        volatile double drift;
        volatile double sigma1;
        volatile double sigma2;
        volatile double sigma3;
        volatile double u3;
        volatile double Tcamp;
        volatile double Tsync;
        volatile double sim_time_limit;
        volatile double alpha;
        volatile double  beta;        
        volatile double RegisterThreshold;        
        volatile double FrameDuration;
        volatile double AdjustParameter;
        // how long is a slot?
        double slotDuration @unit(s) = default(0ms);
    	// schedule the second SYNC from node (i.e., rnode[0])
    	// duration between beacon (first SYNC packet) and second SYNC packet
    	double ScheduleOffset @unit(s) = default(0ms);
    	// @brief this delay consists of transmission delay and propagation delay, see clock.h 
    	double delay @unit(s) = default(0ms);
    	// the parameter of numerator of IIR filter
    	double IIRnum1;
    	double IIRnum2;
    	double IIRnum3;
    	// the parameter of denominator of IIR filter
    	double IIRden1;
    	double IIRden2;
    	double IIRden3;
    	// Clock Correction Algorithm
    	int CorrectionAlgorithm;
    	
    gates:
        input inclock;
        output outclock;
}

