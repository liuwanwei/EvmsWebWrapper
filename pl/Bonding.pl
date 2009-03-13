#!/usr/bin/perl -w
use strict;

my $tmp_file="/tmp/return_value_bonding";

my $ret = -1;

if($#ARGV < 0)
{
	print "Usage: $0 FUNCTION [PARAMETERS]\n";
	exit $ret;
}
elsif($ARGV[0] =~ /^GetAllBondings/i)
{
	#shift(@ARGV);
	$ret = &GetAllBondings();
}
elsif($ARGV[0] =~ /AddBonding/i)
{
	shift(@ARGV);
	$ret = &AddBonding(@ARGV);
}
elsif($ARGV[0] =~ /ModBonding/i)
{
	shift(@ARGV);
	$ret = &ModBonding(@ARGV);
}
elsif($ARGV[0] =~ /DelBonding/i)
{
	shift(@ARGV);
	$ret = $DelBonding(@ARGV);
}
elsif($ARGV[0] =~ /^GetAllEthPorts/i)
{
	#shift(@ARGV);
	$ret = &GetAllEthPorts();
}

exit $ret;


# subroutines definition

sub GetAllBondings()
{
	return 0;
}

sub AddBonding()
{
	return 0;
}

sub ModBonding()
{
	return 0;
}
sub DelBonding()
{
	return 0;
}

sub GetAllEthPorts()
{
	my @interfaces;

	# Get all available ethernet ports on system
	system("ip -o link > $tmp_file");

	open(FH, "<$tmp_file");

	while(<FH>)
	{
		next unless $_ =~ /^[0-9]+: (eth[0-9]+).*link\/ether ([0-9a-f:]+ )/;

		push(@interfaces, $1."\|".$2);
	}

	close(FH);

	# print scalar(@interfaces);

	# Generate Ethernet port's information string.
	# Add append the string to $tmp_file.
	system("rm $tmp_file -f");
	for(my $i = 0; $i < scalar(@interfaces); $i ++)
	{
		#print $interfaces[$i] . "\n";
		system("echo \"$interfaces[$i]\" >> $tmp_file")
	}

	return 0;
}
