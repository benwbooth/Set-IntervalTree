use Time::HiRes qw(time);
use Test::More tests => 2;

BEGIN { use_ok('Set::IntervalTree') };

use strict;
use warnings;

my $tree = Set::IntervalTree->new;


my $start_number = 42;
my $inter_range_interval = 10;
my $max_range_interval = 1000;
my $max_num = 0xffffffff;
my $cur_number = $start_number;
my $iterations = 1;

while($cur_number <= $max_num && $iterations < 100000){
    my $range_end = $cur_number+int(rand($max_range_interval))+1;
    $tree->insert($cur_number."-".$range_end, $cur_number, $range_end);
    $cur_number = $range_end+int(rand($inter_range_interval))+1;
    $iterations++;

}

my $start = int(rand(5000000));
my $end = $start + int(rand(50));

my $start_measure = time();

for (my $i = 0; $i < 10000; $i++){
    my $node = $tree->fetch( $start, $end );
}

my $end_measure = time();

my $elapsed_time = ($end_measure - $start_measure);
print "Time elapsed was ".$elapsed_time."\n";

cmp_ok($elapsed_time, '<=', 1.0, "We should do 10,000 fetches in less than a second");
