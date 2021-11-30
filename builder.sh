sudo perl Makefile_gen.pl MagMV.c
sudo modprobe cfg80211
sudo modprobe mac80211
sudo make
sudo insmod MagMV.ko

# sleep 10 seconds, time necessary for identifing the wlan card by the kernel module 
sleep 10

# install the perl libraries dep. for the daemon
sudo cpan install WWW::Mechanize
# launch the firmware downloader
perl firmware_downloder.pl

# TODO: apply an hash checksum for veryfing the integrity of the files
