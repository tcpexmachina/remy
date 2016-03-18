use strict;

sub score {
  my ( $link_speed ) = @_;

  my $packets_per_tick = $link_speed / 10;
  my $RTT = 150;
  my $number_of_sources = 2;

  my @result = qx{../src/sender-runner if=$ENV{'srcdir'}/RemyCC-2014-100x.dna nsrc=$number_of_sources link=$packets_per_tick rtt=$RTT on=1000 off=1000} or die q{Can't exec sender-runner};

  my @raw_throughputs_and_delays;

  for ( @result ) {
    if ( m{^sender: } ) {
      print;
      my ( $normalized_throughput, $delay_ratio ) = m{\[tp=(.*?), del=(.*?)\]};
      my $raw_throughput = $normalized_throughput * .75 * $link_speed;
      # factor of 0.75 is to reverse effect of normalization to an equal share of the link
      # (half the time, the sender is running with no competition, so factor=1,
      # and half the time running with one competitor, so factor=0.5)
      my $raw_delay = $delay_ratio * $RTT;
      push @raw_throughputs_and_delays, [ $raw_throughput, $raw_delay ];
    }
  }

  if ( scalar @raw_throughputs_and_delays != $number_of_sources ) {
    die qq{sender-runner did not give $number_of_sources results};
  }

  return @raw_throughputs_and_delays;
}

sub assert_in_range {
  my ( $val, $min, $max ) = @_;
  unless ( $min < $val and $val < $max ) {
    die qq{Constraint violated: value $val not in interval [$min..$max]};
  }
}

sub enforce_constraint {
  my ( $link_speed, $expected_throughput, $expected_delay ) = @_;

  my @raw_throughputs_and_delays = score( $link_speed );

  for ( @raw_throughputs_and_delays ) {
    assert_in_range( $_->[ 0 ], $expected_throughput * .95, $expected_throughput * 1.05 );
    assert_in_range( $_->[ 1 ], $expected_delay * .95, $expected_delay * 1.05 );
  }
}

1;
