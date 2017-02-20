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
        double RegisterThreshold @unit(s) = default(1000ms);        
        double FrameDuration @unit(s) = default(1000ms);
    gates:
        input inclock;
        output outclock;
}

