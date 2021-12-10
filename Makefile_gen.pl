# Written by Edoardo Mantovani, 2020
# Automatic script for creating Linux kernel Modules

sub BEGIN{
        use strict;
        use warnings;
        no strict 'subs';

        my $module_name = shift || die("./makefile_gen.pl <module_name>!\n");
        chop($module_name);
        my $stripped_modname = $module_name . "o";
        my $build_par = '$(shell uname -r)/build M=$(shell pwd)';
        my $Makefile_text = 
"obj-m +=$stripped_modname
CFLAGS_$stripped_modname := -O3 -faggressive-loop-optimizations -Wframe-larger-than=4096 -Wno-return-type";
        
        open(MAKEFILE, '>', "Makefile") || die("problem with the Makefile creation\n");
        print MAKEFILE $Makefile_text;
        $Makefile_text = "
all:
	make -C /lib/modules/$build_par modules

clean:
	make -C /lib/modules/$build_par clean        
        ";
        print MAKEFILE $Makefile_text || die("impossible write to file!\n");
        close(MAKEFILE);
}

