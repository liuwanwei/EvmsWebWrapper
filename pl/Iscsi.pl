#!/usr/bin/perl -w
use strict;

my $ret = -1;
my $volume_file="/proc/net/iet/volume";
my $session_file="/proc/net/iet/session";

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
		}
		elsif(lc($key_name) eq "path")
		{
			$path = $value;
		}
		elsif(lc($key_name) eq "type")
		{
			$type = $value;
		}
		elsif(lc($key_name) eq "name")
		{
			$name = $value;
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
	my ($key_name, $value) = split(/=/);

	if(! $key_name =~ /^name/i)
	{
		return -1;
	}

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
}

sub TargetAccessCtrl()
{
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

		if($_ =~ /^tid:(\d*)/)
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
	my $name = $_;

	open(FH, $volume_file) or die $!;

	while(<FH>)
	{
		chomp();

		if($_ =~ /^tid:([/d]*) name:$name/)
		{
			$tid = $1;
			last;
		}
	}

	close(FH);

	return $tid;
}
