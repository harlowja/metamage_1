package Compile::Driver::Job::Link::Archive;

use     Compile::Driver::Job ;
@ISA = 'Compile::Driver::Job';

use warnings;
use strict;


*lib_filename = \&Compile::Driver::Job::lib_filename;


sub new
{
	my $class = shift;
	
	my %self = @_;
	
	my $module = $self{ FROM };
	
	$self{ PATH } = lib_filename( $module->name );
	
	return bless \%self, $class;
}

sub input_files
{
	my $self = shift;
	
	my $objs = $self->{OBJS};
	
	return @$objs;
}

sub command
{
	my $self = shift;
	
	my $dest = $self->{DEST};
	
	my @input = $self->input_files;
	
	return  if $self->up_to_date( @input );
	
	unlink( $dest );
	
	return qw( ar rcs ), $dest, @input;
}

1;

