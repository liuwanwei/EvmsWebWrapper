#!/usr/bin/perl -w

use strict;
use Socket;

require 'header.pl';

if($#ARGV eq -1)
{
	&Usage();
}

my $server = '127.0.0.1';
my $port = '6018';

my $addr;
my $main_msg_type; 
my $sub_msg_type = 0x00;
my $msg_content;
my $msg_len = 0;

while($#ARGV != -1)
{
	if($ARGV[0] =~ /-t/i)
	{
		$main_msg_type = $ARGV[1];
		if($main_msg_type =~ /evm|evms/i)
		{
			$main_msg_type = 0x02;
		}
		elsif($main_msg_type =~ /ib/i)
		{
			$main_msg_type = 0x03;
		}
		elsif($main_msg_type =~ /iscsi/i)
		{
			$main_msg_type = 0x04;
		}
                elsif($main_msg_type =~ /bonding/i)
                {
                        $main_msg_type = 0x06;
                }

		shift;
	}
	elsif($ARGV[0] =~ /-s/i)
	{
		$sub_msg_type = $ARGV[1];
		#print "$sub_msg_type\n";
		if($sub_msg_type =~ /cmd/i)
		{
			$sub_msg_type = 0x26;
		}
		# iSCSI related sub message type.
		elsif($sub_msg_type =~ /AddTarget/i)
		{
			$sub_msg_type = 0x21;
		}
		elsif($sub_msg_type =~ /DelTarget/i)
		{
			$sub_msg_type = 0x22;
		}
		elsif($sub_msg_type =~ /GetAllTargets/i)
		{
			$sub_msg_type = 0x23;
		}
		elsif($sub_msg_type =~ /TargetAccessCtrl/i)
		{
			$sub_msg_type = 0x24;
		}
                elsif($sub_msg_type =~ /TargetCtrlList/i)
                {
                        $sub_msg_type = 0x25;
                }
		# Bonding related sub message type.
                elsif($sub_msg_type =~ /GetAllBondings/i)
                {
                        $sub_msg_type = 0x21;
                }
                elsif($sub_msg_type =~ /AddBonding/i)
                {
                        $sub_msg_type = 0x22;
                }
                elsif($sub_msg_type =~ /ModBonding/i)
                {
                        $sub_msg_type = 0x23;
                }
                elsif($sub_msg_type =~ /DelBonding/i)
                {
                        $sub_msg_type = 0x24;
                }
                elsif($sub_msg_type =~ /GetAllEthPorts/i)
                {
                        $sub_msg_type = 0x25;
                }

		shift;
	}
	else
	{
		print "A: $ARGV[0]\n";

		if($msg_content)
		{
			$msg_content = $msg_content."|".$ARGV[0];
		}
		else
		{
			$msg_content = $ARGV[0];
		}

		print "B: $msg_content\n";
	}

	shift;
}

my $dest = sockaddr_in($port, inet_aton($server));
my $buf  = undef;

print "net_server: $server:$port\n";
print "Send message: \n";
print "main_msg_type : $main_msg_type\n";
print "sub_msg_type  : $sub_msg_type\n";
if($msg_content)
{
	print "msg_content   : $msg_content\n";
	$msg_len = length($msg_content);
	print "msg_body_len  : $msg_len\n\n\n";
}


socket(SOCK, PF_INET, SOCK_STREAM, 6) or die "Can't create socket: $!";
connect(SOCK, $dest)		      or die "Can't connect: $!";

my $header;

# Send header
$header = &PackHeader($main_msg_type, $sub_msg_type, $msg_len);
syswrite(SOCK, $header, length($header));

# Send body if has
if($msg_len gt 8)
{
	syswrite(SOCK, $msg_content, $msg_len);
}

# Send content
if($msg_content)
{
	syswrite(SOCK, $msg_content, length($msg_content));
}

# Receive reply
my $bs = sysread(SOCK, $buf, 2048);
&UnpackReply($buf, $bs);

close(SOCK);

