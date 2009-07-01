#!/usr/bin/perl -w
use strict;

my $ret = -1;
my $volume_file="/proc/net/iet/volume";
my $session_file="/proc/net/iet/session";
my $ctrl_deny="/etc/initiators.deny";
my $ctrl_allow="/etc/initiators.allow";
my $tmp_file="/tmp/return_value_iscsi";

#print "Enter Iscsi.pl\n";

if($#ARGV < 0)
{
	print "Usage: ./Iscsi.pl FUNC_NAME [PARAM_LIST]\n";
	exit $ret;
}

if($ARGV[0] =~ /^AddTarget/)
{
	shift(@ARGV);
	$ret = &AddTarget(@ARGV);
}
elsif($ARGV[0] =~ /^DelTarget/)
{
	shift(@ARGV);
	$ret = &DelTarget(@ARGV);
}
elsif($ARGV[0] =~ /^GetAllTargets/)
{
	$ret = &GetAllTargets();
}
elsif($ARGV[0] =~ /^TargetAccessCtrl/)
{
	shift(@ARGV);
	$ret = &TargetAccessCtrl(@ARGV);
}
else
{
	print "can't be here!\n";
}

#print "ret = $ret\n";
exit $ret;


# subroutines definition

sub AddTarget()
{
	#print "$_[0]\n";
	my @pair_list = split(/\|/, $_[0]);

	# The basic attributes of an iscsi target.
	my ($lun, $path, $type, $name);

	foreach my $pair (@pair_list)
	{
		# The attributes pair.
		my ($key_name, $value) = split(/=/, $pair);
		$key_name = lc($key_name);
		if(lc($key_name) eq "lun")
		{
			$lun = $value;
			print "Get lun: $lun\n";
		}
		elsif(lc($key_name) eq "path")
		{
			$path = $value;
			print "Get path: $path\n";
		}
		elsif(lc($key_name) eq "iotype")
		{
			$type = $value;
			print "Get iotype: $type\n";
		}
		elsif(lc($key_name) eq "name")
		{
			$name = $value;
			print "Get name: $name\n";
		}
	}

	# check the parameters' validity.
	if(! $lun
	|| ! $path
	|| ! $type
	|| ! $name)
	{
		print "Not enough parameters\n";
		return -1;
	}
	else
	{
		print "$lun $path $type $name\n";
	}

	if(0 != &CheckIscsiEnv())
	{
		return -1;
	}

	my $tid = &GetUnusedTID();

	#print "$tid\n";
	#return 0;

	my $sh_ret = system("ietadm --op new --tid=$tid --params Name=$name");
	if($sh_ret != 0)
	{
		print "Iscsi.pl::AddTarget failed.\n";
		return -1;
	}

	$sh_ret = system("ietadm --op new --tid=$tid --lun=$lun --params Path=$path");
	if($sh_ret != 0)
	{
		print "Iscsi.pl::AddTarget failed!\n";
		return -1;
	}

	return 0;
}

sub DelTarget()
{
	my $tid;
	my $value;

	if($_[0] =~ /^name=(.*)/i)
	{
		$value=$1;
	}
	else
	{
		return -1;
	}

	#print $value."\n";

	if(0 == ($tid = &GetTIDByName($value)))
	{
		print "target [$value] not found!\n";
		return -1;
	}

	# FIXME check if return 0 when ietadm succeed.
	my $sh_ret = system("ietadm --op delete --tid=$tid");

	return $sh_ret;
}

sub GetAllTargets()
{
	my $found = 0;
	my $record;
	my $name;

	system("rm $tmp_file -rf");

	open(FH, $volume_file) or die $!;

	while(<FH>)
	{
		chomp();

		print $_."\n";

		#if($_ =~ /^tid:(\d+) name:([\w.:-]+)/)
		if($_ =~ /^tid:(\d+) name:(.+)/)
		{
			$name = $2;
			$found = 1;

			#print "found tid and name:$name\n";
		}
		elsif($_ =~ /lun:(\d+) state:(\d+) iotype:(\w+) iomode:(\w+) path:(.+)/)
		{
			if($found == 1)
			{
				$record = "lun=$1|" . "path=$5|" . "iotype=$3|" . "name=$name";
				system("echo \"$record\" >> $tmp_file");
			}

			$found = 0;
		}
	}

	close(FH);

	return 0;
}

sub TargetAccessCtrl()
{
	my @pair_list = split(/\|/, $_[0]);

	my ($ctrl_type, $tgt_name, $ctrl_string);

	foreach my $pair (@pair_list)
	{
		# The attributes pair.
		my ($key_name, $value) = split(/=/, $pair);
		$key_name = lc($key_name);
		if(lc($key_name) eq "type")
		{
			$ctrl_type = $value;
			print "Get ctrl_type: $ctrl_type\n";
		}
		elsif(lc($key_name) eq "name")
		{
			$tgt_name = $value;
			print "Get target name: $tgt_name\n";
		}
		elsif(lc($key_name) eq "ctrl")
		{
			$ctrl_string = $value;
			print "Get ctrl string: $ctrl_string\n";
		}
	}

	# check the parameters' validity.
	if(! $ctrl_type
	|| ! $tgt_name
	|| ! $ctrl_string)
	{
		print "Not enough parameter\n";
		return -1;
	}
	else
	{
		# Check if target already exists.
		if(0 == &GetTIDByName($tgt_name))
		{
			print "Iscsi target \"$tgt_name\" not exist!\n";
			return -1;
		}
		#print "$ctrl_type $tgt_name $ctrl_string\n";
	}

	if(0 != &CheckIscsiEnv())
	{
		return -1;
	}

	my $ctrl_file;
	if($ctrl_type eq "allow")
	{
		$ctrl_file = $ctrl_allow;
	}
	elsif($ctrl_type eq "deny")
	{
		$ctrl_file = $ctrl_deny;
	}
	else
	{
		print "Invalid iSCSI target ctrl type\n";
		return -1;
	}

	# grep return 1 if nothing found and 0 if found at least 1 matching line.
	my $ret = system("grep \"^[ \t]*#\" -v $ctrl_file | grep \"[ \t]*$tgt_name\"");

	if(0 == $ret)
	{
		#Convert "/" to "\/"  if $ctrl_string has.
		$ctrl_string =~ s/\//\\\//g;
		$ctrl_string =~ s/\[/\\\[/g;
		$ctrl_string =~ s/\]/\\\]/g;

		# The target already have control string, replace it.
		$ret = system("sed -e 's/^[ \t]*$tgt_name.*/$tgt_name $ctrl_string/g' $ctrl_file > $tmp_file");
		if($ret == 0)
		{
			system("cp $tmp_file $ctrl_file -f");
		}
		else
		{
			return $ret;
		}
	}
	else
	{
		# The target has not any control string, add it.
		return system("echo \"$tgt_name $ctrl_string\" >> $ctrl_file");
	}
}

# Check if the iscsi running environmenth properly configured.
sub CheckIscsiEnv()
{
	my $ret = system("lsmod | grep iscsi_trgt | grep -v grep 2>&1 >/dev/null");
	if($ret != 0)
	{
		print "[iscsi_trgt.ko] not ready!\n";
		return -1;
	}

	$ret = system("ps ax | grep ietd | grep -v grep 2>&1 >/dev/null");
	if($ret != 0)
	{
		print "[ietd] not ready!\n";
		return -1;
	}

	if(! -e "/usr/sbin/ietadm")
	{
		print "[ietadm] not ready!\n";
		return -1;
	}

	if(! -e $volume_file)
	{
		print "[$volume_file] not ready!\n";
		return -1;
	}

	if(! -e $session_file)
	{
		print "[$session_file] not ready!\n";
		return -1;
	}

	return 0;
}

sub GetUnusedTID()
{
	my $max_tid=1;
	my $tid;
	open(FH, $volume_file) or die $!;

	while(<FH>)
	{
		chomp();

		if($_ =~ /^tid:(\d+)/)
		{
			if($1 > $max_tid)
			{
				$max_tid = $1 + 1;
			}
		}
	}

	close(FH);

	return $max_tid;
}

sub GetTIDByName()
{
	my $tid = 0;
	my $name = $_[0];

	open(FH, $volume_file) or die $!;

	while(<FH>)
	{
		chomp();

		if($_ =~ /^tid:(\d+) name:(.*)/)
		{
			#print "$2\n";
			#print "$name\n";
			if($2 eq $name)
			{
				$tid = $1;
				last;
			}
		}
	}

	close(FH);

	return $tid;
}
