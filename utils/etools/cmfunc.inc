sub sum {
    my $sum = 0;
    foreach (@_) {
        $sum += $_;
    }
    return $sum;
}

sub println {
    my ($str) = @_;
    print($str."\n");
}

sub printarr {
    println(join(',', @_));
}

1;
