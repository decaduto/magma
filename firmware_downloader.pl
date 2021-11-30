# Written by Edoardo Mantovani, 2020
# Automatic script for downloading automatically from internet the 802.11 firmware blob
# Essentially, once the MagMa detect the wireless adapter model, it sends a DOWNLOAD_REQUEST to the MAGMA_D3M0n, which downloades the firmware requested and
# put it into the 'magma_wlan_fw' appropriate folder

sub BEGIN{
    use strict;
    use warnings;
    no strict 'subs';
    use WWW::Mechanize;
    use LWP::UserAgent; # used for setting the custom UserAgent
    use LWP::Simple qw( getstore );
    my $magma_mecha = WWW::Mechanize->new(cookie_jar = undef); # do not mantain the web cookies
    my $iwlwifi_repo = "https://www.intel.com/content/www/us/en/support/articles/000005511/wireless.html";

    # call the Magma Kernel daemon and ask for the specific wlan fw
    $magma_mecha->get($magma_wlan_specific_repo);
    foreach( $magma_mecha->find_all_links() ){
        if( $_->url =~ ".tgz" ){

        }

    }

}  
