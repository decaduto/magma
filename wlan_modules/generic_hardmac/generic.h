/*  © 2021 Edoardo Mantovani All Rights Reserved */
#ifndef __GENERIC_HARDMAC_H__
    #define __GENERIC_HARDMAC_H__

    //struct wireless_dev *magma_hardmac_add_interface(struct wiphy *wiphy, const char *name, unsigned char name_assign_type, enum nl80211_iftype type, struct vif_params *params );
    //int magma_hardmac_del_interface(struct wiphy *wiphy, struct wireless_dev *wdev);
    //int magma_hardmac_start_ap_mode(struct wiphy *wiphy, struct net_device *dev, struct cfg80211_ap_settings *settings);
    //int magma_hardmac_stop_ap_mode(struct wiphy *wiphy, struct net_device *dev);
    //int magma_hardmac_scan(struct wiphy *wiphy, struct cfg80211_scan_request *request);


    struct wireless_dev *magma_hardmac_add_interface(struct wiphy *wiphy, const char *name, unsigned char name_assign_type, enum nl80211_iftype type, struct vif_params *params ){



    }


    int magma_hardmac_del_interface(struct wiphy *wiphy, struct wireless_dev *wdev){


    }

    int magma_hardmac_start_ap_mode(struct wiphy *wiphy, struct net_device *dev, struct cfg80211_ap_settings *settings){


    }

    int magma_hardmac_stop_ap_mode(struct wiphy *wiphy, struct net_device *dev){



    }

    int magma_hardmac_scan(struct wiphy *wiphy, struct cfg80211_scan_request *request){



    }




#endif
