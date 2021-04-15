
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
## Handle an error
################################################

sub error {
	my ($line, $file, $message) = @_;

	print "$file($line): error - $message.\n";
	exit;
}

################################################
## Handle a warning
################################################

sub warning {
	my ($line, $file, $message) = @_;

	print "$file($line): warning - $message.\n";
}

################################################
## Read and return the contents of a file
################################################

sub readfile {
	my $file = shift;
	my ($contents, @contents);

	open FILE, $file or &error(__LINE__, __FILE__, "file $file cannot be open");
	&lockfile('FILE');
	if (wantarray) {
		@contents = <FILE>;
		&unlockfile('FILE');
		close FILE;

		chomp @contents;
		return @contents;
	} else {
		while (<FILE>) { $contents .= $_; }
		&unlockfile('FILE');
		close FILE;

		chomp $contents;
		return $contents;
	}
}

################################################
## Rewrite a file
################################################

sub writefile {
	my ($file, @contents) = @_;

	open FILE, ">$file" or &error(__LINE__, __FILE__, "file $file cannot be open");
	&lockfile('FILE');
	foreach (@contents) { print FILE "$_\n"; }
	&unlockfile('FILE');
	close FILE;

	chmod 0777, $file;	
	return;
}

################################################
## Lock a file
################################################

sub lockfile {
	my $handle = shift;

	my $success = flock 2, $handle if $FLOCK == 1;
	return $success;
}

################################################
## Unlock a file
################################################

sub unlockfile {
	my $handle = shift;

	my $success = flock 8, $handle if $FLOCK == 1;
	return $success;
}

################################################
## Recursively call subroutine
################################################

sub recursive_call {
	my ($dir, $SUB) = @_;

	opendir(DIR, $dir);
	my @files = readdir(DIR);

	foreach (@files) {
		next if ($_ eq '.' || $_ eq '..' || /Help/ || $_ eq 'PMS' || $_ eq 'PMS.SJIS');
		next if (!/\.asp$/ && !/\.inc$/ && !/\.vbs$/ && !/\.js$/);
		if (-d $dir."/".$_) {
			recursive_call($dir."/".$_, $SUB);
			next;
		}

		my $file = $dir."/".$_;
		&$SUB($file);
	}
}

1;
