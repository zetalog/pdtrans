use strict;
require "cmfunc.inc";

if (@ARGV < 0) {
    println("Usage: perl sc-asp.pl [count|msg|source] path");
    exit;
}
my $cflag = ($ARGV[0] eq 'count') ? 1:0;
my $mflag = ($ARGV[0] eq 'msg') ? 1:0;
my $sflag = ($ARGV[0] eq 'source') ? 1:0;

my $dir = $ARGV[1];

my %umsg;

println('FileName,Lines,JPWordsCount') if $cflag;
my $l = 0;
my $jp = 0;
if (-d $dir) {
    ($l, $jp) = countdir($dir);
} elsif (-f $dir) {
    ($l, $jp) = countfile($dir);
} else {
    println('['.$dir.'] is not folder or file.');
}
println($dir.','.$l.','.$jp) if $cflag;

my $i = 0;
foreach (keys(%umsg)) {
    println('"'.$_.'"') if $mflag;
    $i++;
}
println('Unique Message:'.$i) if $cflag;

sub countdir {
    my ($dir) = @_;
    opendir(DIR, $dir);
    my @files = readdir(DIR);
    my @lens;
    my @jpmsgs;
    foreach (@files) {
        next if ($_ eq '.' || $_ eq '..' || /Help/ || $_ eq 'PMS' || $_ eq 'PMS.SJIS');
        my $l = 0;
        my $jp = 0;
        if (-d $dir."\\".$_) {
            ($l, $jp) = countdir($dir."\\".$_);
            next;
        }
        
        next if (!/\.asp$/ && !/\.inc$/ && !/\.vbs$/ && !/\.js$/);
        my $file = $dir."\\".$_;
        my ($l, $jp) = countfile($file);
        push(@lens, $l);
        push(@jpmsgs, $jp);
        printarr($file, $l, $jp) if ($cflag && $jp > 0);
    }
    return (sum(@lens), sum(@jpmsgs));
}

sub countfile {
    my ($file) = @_;
    open(SF, $file);
    my $l = 0;
    my $jp = 0;
    my ($line, $wline, @tmp, $jflag, $msg, $ml, $char);

    while (<SF>) {
        $l++;
        next if ($_ eq '');
        next if (/^([\s|\t]*)$/);
        next if (/^([\s|\t]*)\'(.*)$/);
        next if (/^([\s|\t]*)<--(.*)$/);
        $line = $_;
        $wline = $line;
        chomp($wline);
        chomp($line);
        $line =~ s/\'([^\']*)$//;
        $ml = length($line);
        $jflag = 0;
        for (my $i = 0; $i < $ml; $i++) {
            $char = substr($line, $i, 1);
            if (ord($char) > 128) {
                if ($sflag) {
                    @tmp = split('"', $wline);
                    $wline = join('""', @tmp);
                    printarr(($file, $l, '"'.$wline.'"'));
                    $jp++;
                    last;
                } elsif ($mflag) {
                    $msg .= $char;
                    $jflag = 1;
                }
            } elsif ($mflag && $jflag) {
                if ($char eq '"' || $char eq '<' ||
                    $char eq "\'" || $char eq "&") {
                    $umsg{$msg}++;
                    $msg = '';
                    $jflag = 0;
                } else {
                    $msg .= $char;
                }
            }
        }
        if ($mflag && $jflag) {
            $umsg{$msg}++;
            $msg = '';
        }
    }
    return($l, $jp);
}
