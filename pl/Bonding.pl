#!/usr/bin/perl -w
use strict;

my $tmp_file="/tmp/return_value_bonding";
my $bonding_sys_base ="/sys/class/net/";
my $bonding_master   = $bonding_sys_base . "bonding_masters";
my $dir_suffix_mode  = "/bonding/mode";
my $dir_suffix_miimon= "/bonding/miimon";
my $dir_suffix_slaves= "/bonding/slaves";
my $dir_suffix_arp_target="/bonding/arp_ip_target";
my $cfg_file_prefix  = "/etc/sysconfig/network-scripts/ifcfg-";
#my $bk_file_prefix   = "bak-";

# Bonding driver module configuration file
my $module_conf ="/etc/modprobe.d/modprobe.conf";

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
	my $cfg;

        # Get all bondings' name
        open (FH, "<$bonding_master") or return -1;
        while(<FH>)
        {
                chomp;

		next unless($_ =~ /^\w+/i);

                push(@bondings, split /[ \t]/);
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
		$cfg = $bonding_sys_base.$name.$dir_suffix_mode;
                open(FH, "<$cfg") or die "$?";
                while(<FH>)
                {
                        chomp;
                        next unless $_ =~ /\w ([0-6])/i;

                        $record = $record . "|mode=" . $1;
                }
                close(FH);

                # Get bonding miimon.
		$cfg = $bonding_sys_base.$name.$dir_suffix_miimon;
                open(FH, "<$cfg") or die "$?";
                while(<FH>)
                {
                        chomp;
                        next unless $_ =~ /^([0-9]+)/i;

                        $record = $record . "|miimon=" . $1;
                }
                close(FH);

                # Get bonding arp_target parameter.
                $cfg = $bonding_sys_base.$name.$dir_suffix_arp_target;
                open(FH, "<$cfg") or die "$?";
                while(<FH>)
                {
                        chomp;
                        next unless $_ =~/^([0-9.,]+)/i;

                        $record = $record . "|arp_ip_target" . $1;
                }
                close();

                # Get slaves
                my @slaves;
		$cfg = $bonding_sys_base.$name.$dir_suffix_slaves;
                open(FH, "<$cfg") or die "$?";
                while(<FH>)
                {
			next unless ($_ =~ /^\w+/);

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

	#print "$record\n";

        system("echo \"$record\" > $tmp_file");

	return 0;
}

sub AddBonding()
{
        my @pair_list = split /\|/, $_[0];
        my ($name, $ip, $mask, $mode, $miimon, $arp_target);
        my @slaves;

        foreach my $pair (@pair_list)
        {
                my ($key, $value) = split(/=/, $pair);
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
                elsif($key eq "arp_target")
                {
                        $arp_target = $value;
                        print "Get arp_target: $arp_target\n";
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
		system("ifconfig $name down 2>&1 > /dev/null");

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

        if($arp_target)
        {
                my $arp_target_ctrl = $bonding_sys_base.$name.$dir_suffix_arp_target;
                if(0 != system("echo +$arp_target > $arp_target_ctrl"))
                {
                        print "Bonding: set bonding arp_target failed\n";
                        return -1;
                }
        }

	my $cfg_file;
	my $cfg_file_bk;

        my $slave_ctrl = $bonding_sys_base.$name.$dir_suffix_slaves;
        foreach my $slave (@slaves)
        {
                # Shut down device before insert into bonding device.
                system("ifconfig $slave down 2>&1 >/dev/null");

                if(0 != system("echo +$slave > $slave_ctrl"))
                {
                        print "Bonding: add slave $slave failed\n";
                        return -1;
                }
		
		# Make this slave device as bonding slave.
		if(0 != &CreateBondingSlaveConfiguration($name, $slave))
		{
			return -1;
		}
        }

	# Finally, creating the bonding master's configuration file
	if(0 != &CreateBondingMasterConfiguration($name, $ip, $mask))
	{
		return -1;
	}

	return 0;
}

sub ModBonding()
{
        my @pair_list = split /\|/, $_[0];
        my ($name, $ip, $mask, $mode, $miimon, $arp_target);
        my @slaves;

        foreach my $pair (@pair_list)
        {
                my ($key, $value) = split(/=/, $pair);
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
                elsif($key eq "arp_ip_target")
                {
                        $arp_target = $value;
                        print "Get arp_ip_target: $arp_target\n";
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

		&ModifyBondingMasterConfiguration($name, "mode", $mode);
        }       

        if($miimon)
        {
                my $miimon_ctrl = $bonding_sys_base.$name.$dir_suffix_miimon;
                if(0 != system("echo $miimon > $miimon_ctrl"))
                {
                        print "Bonding: modify miimon failed\n";
                        return -1;
                }

		&ModifyBondingMasterConfiguration($name, "miimon", $miimon);
        }

        # FIXME if($arp_target)
        #{
        #}

        if($ip)
        {
                if(0 != system("ifconfig $name $ip"))
                {
                        print "Bonding: modify ip failed\n";
                        return -1;
                }

		&ModifyBondingMasterConfiguration($name, "IPADDR", $ip);
        }

        if($mask)
        {
                if(0 != system("ifconfig $name netmask $mask"))
                {
                        print "Bonding: modify mask failed\n";
                        return -1;
                }
		&ModifyBondingMasterConfiguration($name, "NETMASK", $mask);
        }

        my $slave_ctrl = $bonding_sys_base.$name.$dir_suffix_slaves;
        foreach my $slave (@slaves)
        {
                if(0 != system("echo $slave > $slave_ctrl"))
                {
                        print "Bonding: modify slave failed\n";
                        return -1;
                }

		&ModifyBondingMasterConfiguration($name, "slave", $slave);
        }

	return 0;
}

sub DelBonding()
{
        my ($key, $value) = split /=/, $_[0];
        my @slaves;
	my $cfg_file;
	my $cfg_file_bk;

        if(lc($key) eq "device")
        {
                if(0 != system("ip -o link | grep -E \"^[0-9]+: $value\" > /dev/null"))
                {
                        print "Bonding: $value not exist!\n";
                        return -1;
                }
                else
                {
			# Get slaves before deleting the bonding
                	open(FH, "<$bonding_sys_base$value$dir_suffix_slaves") or die "$?";
                	while(<FH>)
                	{
                        	chomp;
                        	push(@slaves, split / /, $_);
                	}
                	close(FH);

                        # Shut down device before remove it.
                        system("ifconfig $value down > /dev/null");

                        if(0 != system("echo -$value > $bonding_master"))
                        {
                                print "Bonding: remove $value failed!\n";
                                return -1;
                        }
                }
        }

        # Delete all slaves' configuration file
        foreach my $slave (@slaves)
        {
		$cfg_file    = $cfg_file_prefix . $slave;
                #$cfg_file_bk = $bk_file_prefix . $cfg_file;
		system("rm $cfg_file -f");
        }

	# Delete Bonding configuration file.
	$cfg_file    = $cfg_file_prefix . $value;
        #$cfg_file_bk = $bk_file_prefix . $cfg_file;
	system("rm $cfg_file -f");

        # Delete configure lines in config file.
        system("sed -e '/^alias $value/d' -e '/^options $value/d' $module_conf > $tmp_file");
	system("cp $tmp_file $module_conf -f");

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

# create config file for this bonding
sub CreateBondingMasterConfiguration()
{
	my $name = shift; 
	my $ip   = shift;
	my $mask = shift;
	my ($cfg_file, $cfg_file_bk);
	$cfg_file=$cfg_file_prefix . $name;

	# Move old configuration file if exists.
	if( -e $cfg_file )
	{
		$cfg_file_bk = $cfg_file . "-bk";
		if(0 != system("mv $cfg_file $cfg_file_bk "))
		{
			print "Bonding: move previous $cfg_file failed\n";
			return -1;
		}
	}

	# Create configuration file for this bonding.

	system("echo \"DEVICE=$name \nIPADDR=$ip \nNETMASK=$mask \nONBOOT=yes \nBOOTPROTO=none \nUSERCTRL=no \n\" > $cfg_file");

        if(0 != &ModifyBondingMasterConfiguration($name, "mode",   1))
        {
                return -1;
        }

        if(0 != &ModifyBondingMasterConfiguration($name, "miimon", 100))
        {
                return -1;
        }
	
	return 0;
}

sub CreateBondingSlaveConfiguration()
{
	my $master = shift;
	my $slave  = shift; 
	my ($cfg_file, $cfg_file_bk);

	$cfg_file    = $cfg_file_prefix . $slave;
        #$cfg_file_bk = $bk_file_prefix . $cfg_file;

	if( -e $cfg_file )
	{
		if(0 != system("rm $cfg_file -f"))
		{
			return -1;
		}
	}

	# Create configuration file for this bonding.
	system("echo \"DEVICE=$slave\nUSERCTRL=no\nONBOOT=yes\nMASTER=$master\nSLAVE=yes\nBOOTPROTO=none\n\" > $cfg_file");
	
	return 0;
}

sub ModifyBondingMasterConfiguration()
{
	my $master = shift;
	my $type   = shift;
	my $value  = shift;

	# Bonding user level configuration file
	my $bonding_conf=$cfg_file_prefix . $master;

	if($type =~ /mode/i or $type =~ /miimon/i)
	{
		if(! -e $module_conf )
		{
			system("touch $module_conf");
		}

		# Modify /etc/modprobe.conf to change the mode configuration
		if(0 != system("grep -E \"^alias\[ \t\]+$master\[ \t\]+bonding\" $module_conf"))
		{
			# Insert default configuration if not exist.
			system("echo \"alias $master bonding\" >> $module_conf");
		}

		if(0 != system("grep -E \"^options[ \t]+$master\" $module_conf"))
		{
			# Insert default configuration if not exist.
			system("echo \"options $master miimon=100 mode=1\" >> $module_conf");
		}


		if($type =~ /mode/i)
		{
			# Found, modify old configuration line.
			system("sed -r -e 's/(^options[ \t]+$master.*mode=).*/\\1$value/g' $module_conf > $tmp_file");
			system("cp $tmp_file $module_conf -f");
		}
		else
		{
			# Found, modify old configuration line.
			system("sed -r -e 's/(^options[ \t]+$master\[ \t\]+miimon=)[0-9]+(.*)/\\1$value\\2/g' $module_conf > $tmp_file");
			system("cp $tmp_file $module_conf -f");
		}
	}
        # FIXME elsif($type =~ /arp_ip_target/)
        #{
        #}
	elsif($type =~ /slave/)
	{
		# Modify corresponding ifcfg-ethX to change the nomral configuration
		my $sign = substr($value, 0, 1);
		my $slave= substr($value, 1);

		if($sign eq "+")
		{
			# Add new ethernet device to bonding
			return &CreateBondingSlaveConfiguration($master, $slave);
		}
		elsif($sign eq "-")
		{
			# Remove old ethernet device to bonding
			my $old_conf    = $cfg_file_prefix . $slave;
                        #my $old_conf_bk = $bk_file_prefix . $old_conf;
			system("rm $old_conf -f");
		}
		else
		{
			return -1;
		}
	}
	else
	{
		# Modify "$master"'s bonding configuration file
		my $cfg_file = $cfg_file_prefix . $master;

		if(! -e $cfg_file )
		{
			print "Bonding: $cfg_file does not exist!\n";
			return -1;
		}

		system("sed -e 's/^$type=.*/$type=$value/g' $cfg_file > $tmp_file");
		system("cp $tmp_file $cfg_file -f");
	}

	return 0;
}
