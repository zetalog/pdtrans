#!/usr/local/bin/perl
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

use strict;

eval { require "pd_lib.pl"; };
if ($@) { print "__LINE__(__FILE__): error - Unable to find required pd_lib.pl module.\n"; exit; }

if (@ARGV < 0) {
    println("Usage: perl pd_resgen.pl path");
    exit;
}

my $path = $ARGV[0];
my %english;
my %japanese;
my %chinese;

my $eflag = 1;
my $jflag = 1;
my $cflag = 0;

if (-f $path) {
	$english{'1'} = "\"english\"";
	$japanese{'1'} = "\"japanese\"";
	$chinese{'1'} = "\"chinese\"";

	my ($id, $japanese, $english) = ~/

	&pdr_generate_rc();
} else {
	error(__LINE__, __FILE__, "$path is not a file");
}

################################################
## Generate .rc file from messages
################################################

sub pdr_generate_rc {
	my $lang = "english";

	print "/* Generated rc file by pd_resgen.pl. Do not modify this file directly. */\n";
	print "#include <winres.h>\n\n";

	pdg_print_lang("english", %english) if $eflag;
	pdg_print_lang("japanese", %japanese) if $jflag;
	pdg_print_lang("chinese", %chinese) if $cflag;
}

sub pdg_print_lang {
	my ($lang, %msgs) = @_;
	my ($id, $value);

	if ($lang eq "english") {
		print "LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US\n";
		print "#pragma code_page(1252)\n";
	} elsif ($lang eq "japanese") {
		print "LANGUAGE LANG_JAPANESE, SUBLANG_DEFAULT\n";
		print "#pragma code_page(932)\n";
	} elsif ($lang eq "chinese") {
		print "LANGUAGE LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED\n";
		print "#pragma code_page(936)\n";
	}
	print "STRINGTABLE DISCARDABLE\n";
	print "BEGIN\n";
	foreach $id (keys(%msgs)) {
		$value = $msgs{$id};
		print "    $id                       $value\n";
	}
	print "END\n\n";
}
