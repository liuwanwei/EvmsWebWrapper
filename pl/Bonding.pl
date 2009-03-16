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
        my ($name, $slave, $record);

        # Get all bondings' name
        open (FH, "<$bonding_master") or return -1;
        while(<FH>)
        {
                chomp;
                push(@bondings, split / /, $_);
        }
        close(FH);

        undef $record;

        # Get each bonding's details and add them to result file.
        foreach $name (@bondings)
        {
                if(defined $record)
                {
                        $record = $record . "device=" . $name;
                }
                else
                {
                        $record = "device=" . $name;
                }

                print "$name\n";

                # Get basic ehernet informations on device.
                if(0 != system("ifconfig $name > $tmp_file"))
                {
                        next;
                }

                open(FH, "<$tmp_file") or die "$?";
                while(<FH>)
                {
                        chomp;
                        if($_ =~ /HWaddr ([0-9a-f:]+)/i)
                        {
                                $record = $record . "|hwaddr=" . $1;
                        }
                        elsif($_ =~ /inet addr:([0-9\.]+)  Bcast:([0-9\.]+)  Mask:([0-9\.]+)/)
                        {
                                $record = $record . "|ip=" . $1 . "|bcase=". $2 . "|mask=" . $3;
                        }
                }
                close(FH);

                # Get bonding mode.
                open(FH, "<$bonding_sys_base$name$dir_suffix_mode") or die "$?";
                while(<FH>)
                {
                        chomp;
                        next unless $_ =~ /\w ([0-6])/i;

                        $record = $record . "|mode=" . $1;
                }
                close(FH);

                # Get bonding miimon.
                open(FH, "<$bonding_sys_base$name$dir_suffix_miimon") or die "$?";
                while(<FH>)
                {
                        chomp;
                        next unless $_ =~ /^([0-9]+)/i;

                        $record = $record . "|miimon=" . $1;
                }
                close(FH);

                # Get slaves
                my @slaves;
                open(FH, "<$bonding_sys_base$name$dir_suffix_slaves") or die "$?";
                while(<FH>)
                {
                        chomp;
                        push(@slaves, split / /, $_);
                }
                close(FH);

                foreach $slave (@slaves)
                {
                        $record = $record . "|slave=" . $slave;
                }

                $record = $record . "\n";
        }

        system("echo \"$record\" > $tmp_file");

	return 0;
}

sub AddBonding()
{
        my $pair_list = split /\|/ $_[0]);
        my ($name, $ip, $mask, $mode, $miimon);
        my @slaves;

        foreach my $pair (@pair_list)
        {
                my $($key, $value) = split(/=/, $pair);
                $key = lc($key);

                if($key eq "device")
                {
                        $name = $value;
                        print "Get name: $name\n";
                }
                elsif($key eq "ip")
                {
                        $ip = $value;
                        print "Get ip: $ip\n";
                }
                elsif($key eq "mask")
                {
                        $mask = $value;
                        print "Get mask: $mask\n";
                }
                elsif($key eq "mode")
                {
                        $mode = $value;
                        print "Get mode: $mode\n";
                }
                elsif($key eq "miimon")
                {
                        $miimon = $value;
                        print "Get miimon: $miimon\n";
                }
                elsif($key eq "slave")
                {
                        push(@slaves, $value);
                }
        }

        if(! $name)
        {
                print "Bonding: can't find bonding!\n";
                return -1;
        }

        if(0 == system("ip -o link | grep -E \"^[0-9]+: $name\" > /dev/null"))
        {
                print "Bonding: $name already exists!\n";
                return -1;
        }

        # create bonding object
        if(0 != system("echo +$name > $bonding_master"))
        {
                print "Bonding: creating $name failed\n";
                return -1;
        }

        if($mode)
        {
                my $mode_ctrl = $bonding_sys_base.$name.$dir_suffix_mode;
                if(0 != system("echo $mode > $mode_ctrl"))
                {
                        print "Bonding: set bonding mode failed\n";
                        return -1;
                }
        }

        if(0 != system("ifconfig $name $ip netmask $mask up"))
        {
                print "Bonding: configuring $name failed\n";
                return -1;
        }

        if($miimon)
        {
                my $miimon_ctrl = $bonding_sys_base.$name.$dir_suffix_miimon;
                if(0 != system("echo $miimon > $miimon_ctrl"))
                {
                        print "Bonding: set bonding miimon failed\n";
                        return -1;
                }
        }

        my $slave_ctrl = $bonding_sys_base.$name.$dir_suffix_slaves;
        foreach my $slave (@slaves)
        {
                if(0 != system("echo +$slave > $slave_ctrl"))
                {
                        print "Bonding: add slave $slave failed\n";
                        return -1;
                }
        }

        # FIXME create config file for this bonding

	return 0;
}

sub ModBonding()
{
        my $pair_list = split /\|/ $_[0]);
        my ($name, $ip, $mask, $mode, $miimon);
        my @slaves;

        foreach my $pair (@pair_list)
        {
                my $($key, $value) = split(/=/, $pair);
                $key = lc($key);

                if($key eq "device")
                {
                        $name = $value;
                        print "Get name: $name\n";
                }
                elsif($key eq "ip")
                {
                        $ip = $value;
                        print "Get ip: $ip\n";
                }
                elsif($key eq "mask")
                {
                        $mask = $value;
                        print "Get mask: $mask\n";
                }
                elsif($key eq "mode")
                {
                        $mode = $value;
                        print "Get mode: $mode\n";
                }
                elsif($key eq "miimon")
                {
                        $miimon = $value;
                        print "Get miimon: $miimon\n";
                }
                elsif($key eq "slave")
                {
                        push(@slaves, $value);
                }
        }

        if(! $name)
        {
                print "Bonding: can't find $name!\n";
                return -1;
        }

        if(0 != system("ip -o link | grep -E \"^[0-9]+: $name\" > /dev/null"))
        {
                print "Bonding: $name not exist!\n";
                return -1;
        }

        if($mode)
        {
                my $mode_ctrl = $bonding_sys_base.$name.$dir_suffix_mode;
                if(0 != system("ifconfig $name down")
                || 0 != system("echo $mode > $mode_ctrl")
                || 0 != system("ifconfig $name up"))
                {
                        print "Bonding: modify mode failed\n";
                        return -1;
                }
        }       

        if($miimon)
        {
                my $miimon_ctrl = $bonding_sys_base.$name.$dir_suffix_miimon;
                if(0 != system("echo $miimon > $miimon_ctrl"))
                {
                        print "Bonding: modify miimon failed\n";
                        return -1;
                }
        }

        if($ip)
        {
                if(0 != system("ifconfig $name $ip"))
                {
                        print "Bonding: modify ip failed\n";
                        return -1;
                }
        }

        if($netmask)
        {
                if(0 != system("ifconfig $name netmask $mask"))
                {
                        print "Bonding: modify mask failed\n";
                        return -1;
                }
        }

        my $slave_ctrl = $bonding_sys_base.$name.$dir_suffix_slaves;
        foreach my $slave (@slaves)
        {
                if(0 != system("echo $slave > $slave_ctrl"))
                {
                        print "Bonding: modify slave failed\n";
                        return -1;
                }
        }

        # FIXME modify config file for this bonding

	return 0;
}
sub DelBonding()
{
        my ($key, $value) = split /=/ $_[0];

        if($key eq "device")
        {
                if(0 != system("ip -o link | grep -E \"^[0-9]+: $value\" > /dev/null"))
                {
                        print "Bonding: $name not exist!\n";
                        return -1;
                }
                else
                {
                        if(0 != system("echo -$value > $bonding_master > /dev/null"))
                        {
                                print "Bonding: remove $value failed!\n";
                                return -1;
                        }
                }
        }

        # FIXME modify config file for this bonding

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
