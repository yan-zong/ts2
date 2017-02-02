package ts2.im;

simple Clock
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
    gates:
        input inclock;
        output outclock;
}

