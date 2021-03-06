# Conditional Modeling Toolkit

![samples](https://raw.github.com/lucastheis/cmt/develop/media/samples.jpg)

Fast implementations of several probabilistic models. Examples:

* MCGSM (mixture of conditional Gaussian scale mixtures; Theis et al., 2012)
* MCBM (mixture of conditional Boltzmann machines)
* FVBN (fully-visible belief network; Neal, 1992)
* GLM (generalized linear model; Nelder & Wedderburn, 1972)
* MLR (multinomial logistic regression)
* STM (spike-triggered mixture model; Theis et al., 2013)

## Requirements


### Python Interface
* Python >= 2.7.0
* NumPy >= 1.6.1
* automake >= 1.11.0
* libtool >= 2.4.0
* Pillow >= 2.6.1 (optional)

### Matlab Interface
* Matlab >= R2009b
* MEX compatible compiler

I have tested the code with the above versions, but older versions might also work.

## Python Example

```python
from cmt.models import MCGSM
from cmt.transforms import WhiteningPreconditioner

# load data
input, output = load('data')

# preprocessing
wt = WhiteningPreconditioner(input, output)

# fit a conditional model to predict outputs from inputs
model = MCGSM(dim_in=input.shape[0],
              dim_out=output.shape[0],
              num_components=8,
              num_scales=6,
              num_features=40)
model.initialize(*wt(input, output))
model.train(*wt(input, output), parameters={
        'max_iter': 1000,
        'threshold': 1e-5})

# evaluate log-likelihood [nats] on the training data
loglik = model.loglikelihood(*wt(input, output)) + wt.logjacobian(input, output)
```

## Matlab Example

```matlab
% load the data
data = load('data.mat')

% fit a generalized linear model to the data
model = cmt.GLM(10, cmt.ExponentialFunction, cmt.Poisson));
model.train(data.input, data.output, 'maxIter', 1000, 'threshold', 1e-5);

```

## Python Interface Installation

### Linux

Make sure autoconf, automake and libtool are installed.

	apt-get install autoconf automake libtool

Go to `./code/liblbfgs` and execute the following:

	./autogen.sh
	./configure --enable-sse2
	make CFLAGS="-fPIC"

Once the L-BFGS library is compiled, go back to the root directory and execute:

	python setup.py build
	python setup.py install

### Mac OS X

First, make sure you have recent versions of automake and libtool installed. The versions that come
with Xcode 4.3 didn't work for me. You can use [Homebrew](http://mxcl.github.com/homebrew/) to install
newer ones.

	brew install autoconf automake libtool
	brew link autoconf automake libtool

Next, go to `./code/liblbfgs` and execute the following:

	./autogen.sh
	./configure --disable-dependency-tracking --enable-sse2
	make CFLAGS="-arch x86_64 -arch i386"

Once the L-BFGS library is compiled, go back to the root directory and execute:

	python setup.py build
	python setup.py install

## Matlab Interface Installation

Normally it should be enough to download one of the prebuild binaries of our cmt matlab interface. If you
want to build it yourself, the following instructions are for you.

### Build lbfgs library 
The first step is always to follow the above instructions to build liblbfgs for your platform.

On Windows this is done by opening `code\liblbfgs\lbfgs.sln` in Visual Studio. In Visual Studio, make
sure to select the "Release" configuration and the right platform (probably "x64"). Then
build the "lib" project by selecting it with a right click and choosing "build". The resulting file `lbfgs.lib` should
be found in the subfolder `x64\Release` (or "Release" for x86 systems, in which case you have to change that path in the setup script).

### Building the mex interface in Matlab

Next open Matlab and make sure a valid mex compiler can be found:

	mex -setup

If that is not the case, please follow the official MathWorks documentation to install a supported compiler 
and check again.

Then execute `setup.m` from the root folder of cmt in Matlab:

	setup

The distribute folder should now contain all the files needed to run the CMT toolbox from within matlab. Add
it to your matlab path to use it.
