# Written by Edoardo Mantovani, 2020
# Automatic script for downloading automatically from internet the 802.11 firmware blob
# Essentially, once the MagMa detect the wireless adapter model, it sends a DOWNLOAD_REQUEST to the MAGMA_D3M0n, which downloades the firmware requested and
# put it into the 'magma_wlan_fw' appropriate folder

sub BEGIN{
    use strict;
    use warnings;
    no strict 'subs';
    use WWW::Mechanize;



}  
