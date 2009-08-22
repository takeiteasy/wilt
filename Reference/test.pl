#!/usr/bin/perl

sub test_gzip($)
{
	my ($filename)=@_;
	my $tmpname="/tmp/.tmp__gzip__";
	`gzip -c -f -9 '$filename' >'$tmpname'`;
	my $size=-s $tmpname;
	unlink $tmpname;
	my ($filepart)=$filename=~m!([^/])$!;
	return $size-13-length($filepart);
}

sub test_bzip2($)
{
	my ($filename)=@_;
	my $tmpname="/tmp/.tmp__bzip2__";
	`bzip2 -c -9 '$filename' >'$tmpname'`;
	my $size=-s $tmpname;
	unlink $tmpname;
	return $size;
}

sub test_lzma($)
{
	my ($filename)=@_;
	my $tmpname="/tmp/.tmp__lzma__";
	`lzma -c -8 '$filename' >'$tmpname'`;
	my $size=-s $tmpname;
	unlink $tmpname;
	return $size;
}

sub test_wilt($$@)
{
	my ($exe,$filename,$a,$b,$c,$d,$e,$f)=@_;
	my $tmpname="/tmp/.tmp__wilt__";
	`$exe <'$filename' $a $b $c $d $e $f >'$tmpname'`;
	my $size=-s $tmpname;
	unlink $tmpname;
	return $size-7;
}

my ($exe,$file);
if(@ARGV==2)
{
	$exe=$ARGV[0];
	$file=$ARGV[1];
}
else
{
	$exe="./WiltCompress";
	$file=$ARGV[0];
}

my $gzipsize=test_gzip($file);
print "gzip: $gzipsize\n";

my $bzip2size=test_bzip2($file);
print "bzip2: $bzip2size\n";

my $lzmasize=test_lzma($file);
print "LZMA: $lzmasize\n";

my $minaval=1;
my $minsize=10000000000;
for(1..8)
{
	my @args=($_,4,4,4,4,4);
	my $size=test_wilt($exe,$file,@args);
	print "@args: $size\n";
	if($size<$minsize) { $minaval=$_; $minsize=$size; }
}

my $minbval=1;
my $minsize=10000000000;
for(1..8)
{
	my @args=($minaval,$_,4,4,4,4);
	my $size=test_wilt($exe,$file,@args);
	print "@args: $size\n";
	if($size<$minsize) { $minbval=$_; $minsize=$size; }
}

my $mincval=1;
my $minsize=10000000000;
for(1..8)
{
	my @args=($minaval,$minbval,$_,4,4,4);
	my $size=test_wilt($exe,$file,@args);
	print "@args: $size\n";
	if($size<$minsize) { $mincval=$_; $minsize=$size; }
}

my $mindval=1;
my $minsize=10000000000;
for(1..8)
{
	my @args=($minaval,$minbval,$mincval,$_,4,4);
	my $size=test_wilt($exe,$file,@args);
	print "@args: $size\n";
	if($size<$minsize) { $mindval=$_; $minsize=$size; }
}

my $mineval=1;
my $minsize=10000000000;
for(1..8)
{
	my @args=($minaval,$minbval,$mincval,$mindval,$_,4);
	my $size=test_wilt($exe,$file,@args);
	print "@args: $size\n";
	if($size<$minsize) { $mineval=$_; $minsize=$size; }
}

my $minfval=1;
my $minsize=10000000000;
for(1..8)
{
	my @args=($minaval,$minbval,$mincval,$mindval,$mineval,$_);
	my $size=test_wilt($exe,$file,@args);
	print "@args: $size\n";
	if($size<$minsize) { $minfval=$_; $minsize=$size; }
}

print "-----\n";
print "$minaval $minbval $mincval $mindval $mineval $minfval: $minsize\n";
#test_wilt($exe,$file,$minaval,$minbval,$mincval,$mindval,$mineval,$minfval);
printf "gzip: $gzipsize (comparison: %.2f%%)\n",100*$minsize/$gzipsize;
printf "bzip2: $bzip2size (comparison: %.2f%%)\n",100*$minsize/$bzip2size;
printf "lzma: $lzmasize (comparison: %.2f%%)\n",100*$minsize/$lzmasize;

