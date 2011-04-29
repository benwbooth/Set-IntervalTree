package Set::IntervalTree;

use 5.006001;
use strict;
use warnings;
use Carp;

require Exporter;
use AutoLoader;

our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Set::IntervalTree ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
	
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	
);

our $VERSION = '0.02';

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.

    my $constname;
    our $AUTOLOAD;
    ($constname = $AUTOLOAD) =~ s/.*:://;
    croak "&Set::IntervalTree::constant not defined!" if $constname eq 'constant';
    my ($error, $val) = constant($constname);
    if ($error) { croak $error; }
    {
	no strict 'refs';
	# Fixed between 5.005_53 and 5.005_61
#XXX	if ($] >= 5.00561) {
#XXX	    *$AUTOLOAD = sub () { $val };
#XXX	}
#XXX	else {
	    *$AUTOLOAD = sub { $val };
#XXX	}
    }
    goto &$AUTOLOAD;
}

require XSLoader;
XSLoader::load('Set::IntervalTree', $VERSION);

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Set::IntervalTree - Perform range-based lookups on sets of ranges.

=head1 SYNOPSIS

  use Set::IntervalTree;
  my $tree = Set::IntervalTree->new;
  $tree->insert("ID1",100,200);
  $tree->insert(2,50,100);
  $tree->insert({id=>3},520,700);
  $tree->insert($some_obj,1000,1100);

  my $results = $tree->fetch(400,800);
  my $window = $tree->window(100,200);
  print scalar(@$results)." intervals found.\n";

=head1 DESCRIPTION

Set::IntervalTree uses Interval Trees to store and efficiently 
look up ranges using a range-based lookup.

=head1 EXPORTS

Nothing.

=head1 METHODS

my $tree = Set::IntervalTree->new;

  Creates a new interval tree object.

$tree->insert($object, $low, $high);

  Insert a range into the interval tree and associate it with a 
  perl scalar.

  $object can be any perl scalar. This is what will be returned by fetch().
  $low is the lower bound of the range.
  $high is the upper bound of the range.

  Ranges are represented as both-ends closed intervals.

my $results = $tree->fetch($low, $high)

  Return an arrayref of perl objects whose ranges overlap 
  the specified range.

  $low is the lower bound of the region to query.
  $high is the upper bound of the region to query.

my $results = $tree->window($low, $high)

  Return an arrayref of perl objects whose ranges are completely contained
  witin the specified range.

  $low is the lower bound of the region to query.
  $high is the upper bound of the region to query.

=head1 Limitations

I still haven't implemented a $tree->remove() method to remove ranges 
from a tree.

A $tree->print() serialization method might be useful for debugging.

=head1 SEE ALSO

The source code for this module contains a reusable template-based 
C++ header for Interval trees that might be useful.

=head1 AUTHOR

Ben Booth, E<lt>bbooth@E<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2010 by Ben Booth

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.10.1 or,
at your option, any later version of Perl 5 you may have available.


=cut
