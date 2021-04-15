
############################################################################
#
# pd_translator version 1.0 by Lv Zheng.
# Created by:
#		Lv 'Zetalog' Zheng
#		NECAS Shanghai DC
#		http://www.necas.com.cn
# Modified by:
#		Lv Zheng
#		zhengl@sh.necas.nec.com.cn
#
# Initiated:      6/02/2004 Version 1.0
#
# Copyright (c) 2004, NECAS.Com. All rights reserved.
# This software is protected by the national and international copyright 
# laws that pertain to computer software. You may not loan, sell, rent,
# lease, give, sub license, or otherwise transfer the software (or any copy).  
# 
# Disclaimer
#
# By using the script(s), you are supposed to agree and understand that
# the writers are not responsible for any damages caused under any
# conditions due to the malfunction or bugs from the script(s). Please
# use at your own risk.
#
############################################################################

############################################################################
## IT IS ILLEGAL FOR YOU TO VIEW, EDIT, COPY, DELETE, 
## TRANSFER, OR IN ANY WAY MANIPULATE THE CODE BELOW 
## THIS LINE.
############################################################################

################################################
## rc contents replacement plugin
################################################

sub rc_replace_contents {
	my (@contents, %resources) = @_;

	print "add per-ext replacement code here:\n";
	print "file lines has been put into \@contents array\n";
	print "resource strings has been put into \%resoureces hash table\n";
	return @contents;
}

1;
