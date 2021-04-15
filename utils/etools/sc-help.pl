use strict;
require "cmfunc.inc";

if (@ARGV < 0) {
    println("Usage: perl sc-help.pl [count|msg|source] path");
    exit;
}
my $cflag = ($ARGV[0] eq 'count') ? 1:0;
my $mflag = ($ARGV[0] eq 'msg') ? 1:0;
my $sflag = ($ARGV[0] eq 'source') ? 1:0;

my $dir = $ARGV[1];
my %umsg;

opendir(DIR, $dir);
my @dirs = readdir(DIR);
my @lens;
my @jpmsgs;

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
        next if (!/\.asp$/ && !/\.htm$/);
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
    my ($line, $wline);
    while (<SF>) {
        $l++;
        next if ($_ eq '');
        next if (/^([\s|\t]*)$/);
        $line = $_;
        $wline = $line;
        chomp($wline);
        my $ml = length($line);
        goto nextpart if ($ml < 2 && $line ne '');
        for (my $i = 0; $i < $ml; $i++) {
            if (ord(substr($line, $i, 1)) > 128) {
                if ($sflag) {
                    printarr(($file, $l, $wline));
                    last;
                }
                $jp++;
                $i++;
            }
        }
    }
    return($l, $jp);
}
