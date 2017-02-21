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
    gates:
        input inclock;
        output outclock;
}

