#!/usr/bin/perl -w
use strict;

my $tmp_file="/tmp/return_value_bonding";
my $bonding_sys_base ="/sys/class/net/";
my $bonding_master   = $bonding_sys_base . "bonding_masters";
my $dir_suffix_mode  = "/bonding/mode";
my $dir_suffix_miimon= "/bonding/miimon";
my $dir_suffix_slaves= "/bonding/slaves";

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
	$ret = &DelBonding(@ARGV);
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
        my @bondings;
        my ($i, $cnt, $name, $slave, $record);

        open (FH, "<$bonding_master") or return -1;

        # Get all bondings' name
        while(<FH>)
        {
                push(@bondings, split / /, $_);
        }

        close(FH);

        # Clear result file before we do things.
        system("rm -f $tmp_file");

        # Get each bonding's details and add them to result file.
        $cnt = scalar(@bondings);
        for($i = 0; $i < $cnt; $i ++)
        {
                #print "$bondings[$i]\n";
                $name = $bondings[$i];
                $result = "device=" . $name;

                # Get basic ehernet informations on device.
                if(0 != system("ifconfig $name > $tmp_file"))
                {
                        next;
                }

                open(FH, "<$tmp_file") or die "$?";
                while(<FH>)
                {
                        if($_ =~ /HWaddr ([0-9a-f:]+)/i)
                        {
                                $result += "|hwaddr=" . $1;
                        }
                        elsif($_ =~ /inet addr:([0-9\.]+)  Bcast:([0-9\.]+)  Mask:([0-9\.]+)/)
                        {
                                $result += "|ip=" . $1 . "|bcase=". $2 . "|mask=" . $3;
                        }
                }
                close(FH);

                # Get bonding mode.
                open(FH, "<$bonding_sys_base$name$dir_suffix_mode") or die "$?";
                while(<FH>)
                {
                        next unless $_ =~ /\w ([0-6])/i;

                        $result += "|mode=" . $1;
                }
                close(FH);

                # Get bonding miimon.
                open(FH, "<$bonding_sys_base$name$dir_suffix_miimon") or die "$?";
                while(<FH>)
                {
                        next unless $_ =~ /^([0-9]+)/i;

                        $result += "|miimon=" . $1;
                }
                close(FH);

                # Get slaves
                my @slaves;
                open(FH, "<$bonding_sys_base$name$dir_suffix_slaves") or die "$?";
                {
                        push(@slaves, split / /, $_);
                }
                close(FH);

                foreach $slave (@slaves)
                {
                        $result += "|slave=" . $slave;
                }

                system("echo $result >> $tmp_file 2>&1 >/dev/null");
        }

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
