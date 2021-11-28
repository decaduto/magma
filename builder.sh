sudo perl Makefile_gen.pl MagMV.c
sudo modprobe cfg80211
sudo modprobe mac80211
sudo make
sudo insmod MagMV.ko

