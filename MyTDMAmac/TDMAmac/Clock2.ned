package mytdmamac.TDMAmac;

simple Clock2
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
        // how long is a slot?
        double slotDuration @unit(s) = default(0ms);
    	// schedule the second SYNC from node (i.e., rnode[0])
    	// duration between beacon (first SYNC packet) and second SYNC packet
    	double ScheduleOffset @unit(s) = default(0ms);
    gates:
        input inclock;
        output outclock;
}

