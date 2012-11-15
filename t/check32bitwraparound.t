#!/usr/bin/perl

use strict;
use warnings;

use Test::Simple tests=>1;
use Set::IntervalTree;

my $tree = Set::IntervalTree->new();
my $min  = 1;
my $max  = 2;
my $test = 2_147_483_648;              # 2^31 - Shouldn't match but does

$tree->insert( "$min - $max", $min, $max );

my $f = $tree->fetch($test,$test);
ok($f && !@$f);
