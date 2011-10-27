Kinect external for puredata
---------------

## How to Compile

1. Download pd-extended source, install libfreenect using fink
 * curl -o libfreenect.info http://fink.cvs.sourceforge.net/viewvc/fink/dists/10.4/unstable/main/finkinfo/libs/libfreenect.info?revision=1.2
 * mv libfreenect.info /sw/fink/dists/local/main/finkinfo/libfreenect.info
 * fink scanpackages
 * fink index
 * fink install libfreenect
2. Configure path on Makefile
3. Compile -> make all
4. Deploy -> sudo make deploy

## Download Binary!:

precompiled binary for osx snow leopard
https://github.com/downloads/badgeek/fux_kinect/fux_kinect.zip
extract and drop fux_kinect to extra folder inside pd extra (external)  directory

## Author

author is dead - http://manticore.deadmediafm.org
