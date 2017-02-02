package ts2;

simple Node
{
    parameters:
       @class(Node);
      //volatile double Tsync;
      //  volatile double nsync;
      volatile double Tcamp;
      volatile double rate;
    //  volatile double alpha;
      //volatile double  beta;
      //volatile double Tsync;
      //volatile double s1;
	//  volatile double s2;
	//  volatile double s3;
       int slaveAddrOffset;
       
    gates:
        input in;
        input inclock;
        input inevent;
        output out;
        output outclock;
        
        input upperGateIn; 
        output upperGateOut;
}


