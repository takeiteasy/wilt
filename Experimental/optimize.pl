#!/usr/bin/perl

use strict;

my ($exe,$file,$numargs)=@ARGV;
my $origsize=-s $file;

my $minsize=10000000000;

my @args=(4)x$numargs;
foreach my $arg (0..$#args)
{
	my $minval=$args[$arg];
	for(1..8)
	{
		$args[$arg]=$_;
		my $size=test_compressor($exe,$file,@args);
		print "@args: $size\n";
		if($size<$minsize) { $minval=$_; $minsize=$size; }
	}
	$args[$arg]=$minval;
}

print "-----\n";
printf "@args: $minsize, %.2f%%, %.2f\n",100*$minsize/$origsize,8*$minsize/$origsize;

my $gzipsize=test_gzip($file);
my $bzip2size=test_bzip2($file);
my $lzmasize=test_lzma($file);
printf "gzip: $gzipsize, %.2f%%, %.2f (comparison: %.2f%%)\n",100*$gzipsize/$origsize,8*$gzipsize/$origsize,100*$minsize/$gzipsize if $gzipsize;
printf "bzip2: $bzip2size, %.2f%%, %.2f (comparison: %.2f%%)\n",100*$bzip2size/$origsize,8*$bzip2size/$origsize,100*$minsize/$bzip2size if $bzip2size;
printf "lzma: $lzmasize, %.2f%%, %.2f (comparison: %.2f%%)\n",100*$lzmasize/$origsize,8*$lzmasize/$origsize,100*$minsize/$lzmasize if $lzmasize;




sub test_compressor($$@)
{
	my ($exe,$filename,@args)=@_;
	my $tmpname="/tmp/.tmp__compression_test__";
	my $args=join " ",@args;
	`$exe <'$filename' $args >'$tmpname'`;
	my $size=-s $tmpname;
	unlink $tmpname;
	return $size;
}

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
