bnn-hep
=======

A tool to enable Bayesian neural networks (BNN) for high-energy physics. The project is an adaptation of BNN from
[FBM package](http://www.cs.toronto.edu/~radford/fbm.software.html) developed by R.M. Neal, relevant fragments of which
are added to the repository. Key differences with respect to the original framework are the following:
 * Neural networks for problem of binary classification are only considered (no regression, only two classes of events
   are allowed)
 * Customisation is done in a convenient way with the help of a configuraion file. User runs a single executable
 * Events are described in [ROOT](http://root.cern.ch) files
 * Implementation can handle weighted events (though, only physical, sctrictly positive weights are allowed)
 * C++ code that defines a specific BNN is produced as the result of training

Configuration file is parsed with a pruned version of [libconfig](http://www.hyperrealm.com/libconfig/) library. The tool
uses [ROOT](http://root.cern.ch) and [Boost](http://www.boost.org) libraries.

Being a detached tool, bnn-hep cannot compete with a generalised MVA framework in the level of flexibility and convenience
of usage. In addition to it, development is complicated by the fact that original FBM framework is coded in pure C. For
this reason no active development of the tool is expected, but only bugfixes and minor improvements. In a longer time
scale I hope to reimplement BNN in [TMVA package](http://tmva.sourceforge.net/).


# How to install bnn-hep: 

1. Copy to workdir:
```
cd workdir
git clone link
```

2. Activate env: 
```
source env.sh
```

3. Clean and build project:
```
./make-clean
./make-all 
```

# How to use bnn-hep: 
```
mkdir build
cd build
bnn-hep /path/tWb.cfg
```

  + tWb.cfg main parametrs:
  ```
  task-name = "BNN_tWb_8"; name of network and finish .hpp file
  file-name = "/scratch/pvolkov/tWb/samples_new/13tev__tWb_DR1__GG_NneEeuUdDbB.root"; sample path
  number-events = ["25000", "50%"]; number of evnents <= persent of file 
  event-list-file = "qcd2_trainEvents.txt";
  ```

  + Signal and background can have few samples:
  ```
  ensemble-size = 8; numbers of networks
  burn-in = 7; lenght of trainning
  ```

  + After trainning ```BNN_tWb_8.hpp``` and ```BNN_tWb_8C.py``` must be written!

# How to use dnn: 

1. Open new lxplus terminal:
```
cd workdir/bnn-hep/test
```

2. Activate env: 
```
source dnn_env.sh 
```

3. Train model: 
```
python trainModel.py --taskName BNN_tWb_8 --layers 3 --externalEepoch 10 --internalEpoch 100 --modelName testModel
```

+ taskName - BNN_tWb_8 same the bnn taskName.
+ Network architecture can be changed in ```model_arch.py```!