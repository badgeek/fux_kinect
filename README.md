Kinect external for puredata

1. INSTALL LIBFREENECT ON FINK

* curl -o libfreenect.info http://fink.cvs.sourceforge.net/viewvc/fink/dists/10.4/unstable/main/finkinfo/libs/libfreenect.info?revision=1.2
* mv libfreenect.info /sw/fink/dists/local/main/finkinfo/libfreenect.info
* fink scanpackages
* fink index
* fink install libfreenect

2. Download Pd-extended source
3. Configure path on Makefile
4. Compile -> make all
5. Deploy -> make deploy

Update!:

precompiled binary for osx snow leopard
https://github.com/downloads/badgeek/fux_kinect/fux_kinect.zip

extract and drop file to extra folder on pd