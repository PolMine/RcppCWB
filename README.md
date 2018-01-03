[![Travis-CI Build Status](https://api.travis-ci.org/PolMine/RcppCWB.svg?branch=master)](https://travis-ci.org/PolMine/RcppCWB)
[![codecov](https://codecov.io/gh/PolMine/RcppCWB/branch/master/graph/badge.svg)](https://codecov.io/gh/PolMine/RcppCWB/branch/master)

# Rcpp backend for the polmineR backage

The package offers a Rcpp implementation of performance critical operations. Precondition is the presence of the Corpus Workbench (CWB) on your system, as C++ code is compiled against the corpus library (CL), a C library that is part of the CWB.

An installation of the CWB is a system requirement for compiling the polmineR.Rcpp package. See the 
[CWB Homepage](http://cwb.sourceforge.net) for instructions how to get and install the CWB.

## Installation on Linux

The C++ code in the package needs to be compiled against an installation of the CWB. On Linux, that may be achieved as follows:

```{sh}
sudo apt-get install libglib2.0-dev libssl-dev libcurl4-openssl-dev


mkdir ~/tmp
cd ~/tmp
wget https://sourceforge.net/projects/cwb/files/cwb/cwb-3.0.0/cwb-3.0.0-linux-x86_64.tar.gz
tar xzfv cwb-3.0.0-linux-x86_64.tar.gz
cd cwb-3.0.0-linux-x86_64
./install-cwb.sh # usually sudoer rights will be necessary

# install Perl module
wget http://cwb.sourceforge.net/temp/Perl-CWB-2.2.102.tar.gz
tar xzfv Perl-CWB-2.2.102.tar.gz
cd CWB-2.2.102
perl Makefile.PL
make
make test
make install # may require sudoer rights

cd ..
rm -r tmp
```

## Installation on MacOS

Needs to be written.

## Installation on Windows

Binaries of the package will be hosted at the PolMine R repository (polmine.sowi.uni-due.de/packages) in due course. For the time being, it is necessary to compile the package:

* Installation of [Rtools](https://cran.r-project.org/bin/windows/Rtools/) is required for compiling on windows.
* The Rtools-directory (usually C:/Rtools) needs to be on the PATH environment variable
* Install the Corpus Worbench (pre-built binaries) on the system (get it [here](https://sourceforge.net/projects/cwb/files/cwb/cwb-3.4-beta/)). The unzipped folder contains an installer script (*.bat-file) that works nicely. Important: Make sure you run the installer with administrator rights.
* Put the bin-subdirectory of the CWB in the Program Files-folder on the PATH (typically "C:\Program Files\CWB\bin").
* Run "R CMD build polmineR.Rcpp" in the folder with the polmineR.Rcpp-folder.
* Run "R CMD INSTALL polmineR.Rcpp_VERSION".



### Windows (64 bit / x86)

At this stage, 64 bit support is still experimental. Apart from an installation of 64 bit R, you will need to install Rtools, available [here](https://cran.r-project.org/bin/windows/Rtools/). Rtools is a collection of tools necessary to build and compile R packages on a Windows machine.

To interface to a core C library of the Corpus Workbench (CWB), you will need an installation of a 64 bit AND a 32 bit version of the CWB. 

The "official" 32 bit version of the CWB is available [here](https://sourceforge.net/projects/cwb/files/cwb/cwb-3.4-beta/). Installation instructions are available at the [CWB Website](http://cwb.sourceforge.net/beta.php). The 32 bit version should be installed in the directory "C:\Program Files", with admin rights.

The 64 bit version, prepared by Andreas Blaette, is available [here](http://polmine.sowi.uni-due.de/public/?dir=CWB). Install this 64 bit CWB version to "C:\Program Files (x86)". In the unzipped downloaded zip file, you will find a bat file that will do the installation. Take care that you run the file with administrator rights. Without these rights, no files will be copied.

The interface to the Corpus Workbench is the package polmineR.Rcpp, [available at GitHub](https://www.github.com/PolMine/polminer.Rcpp). If you use git, you can clone that repository, otherwise, you can download a zip file.

The downloaded zip file needs to be unzipped again. Then, in the directory with the 'polmineR.Rcpp'-directory, run:

```{sh, eval = FALSE}
R CMD build polmineR.Rcpp
R CMD INSTALL polmineR.Rcpp_0.1.0.tar.gz
```

If you read closely what is going on during the compilation, you will see a few warnings that libraries are not found. If creating the package is not aborted, nothing is wrong. R CMD build will look for the 64 bit files in the directory with the 32 bit dlls first and discover that they do not work for 64 bit, only then will it move to the correct location.

One polmineR.Rcpp is installed, proceed with the instructions for installing polmineR in a 32 bit context. Future binary releases of the polmineR.Rcpp package may make things easier. Anyway, the proof of concept is there that polmineR will work on a 64 bit Windows machine too.

Finally, you need to make sure that polmineR will interface to CWB indexed corpora using polmineR.Rcpp, and not with rcqp (the default). To set the interface accordingly:

```{r, eval = FALSE}
setCorpusWorkbenchInterface("Rcpp")
```

