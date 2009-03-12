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
my $msg_type; 
my $msg_sub_type = 0x00;
my $msg_content = "";

while($#ARGV != -1)
{
	if($ARGV[0] =~ /-t/i)
	{
		if($ARGV[1] =~ /h|ha|han|hand|handshake/i)
		{
			$msg_type = 0x01;
		}
		elsif($ARGV[1] =~ /e|ev|evm|evms|/i)
		{
			$msg_type = 0x02;
		}
		elsif($ARGV[1] =~ /i|ib|/i)
		{
			$msg_type = 0x03;
		}
		else
		{
			&Usage();
		}

		shift;
	}
	elsif($ARGV[0] =~ /-s/i)
	{
		$server = $ARGV[1];
		shift;
	}
	else
	{
		$msg_content = $ARGV[0];
	}

	shift;
}

my $dest = sockaddr_in($port, inet_aton($server));
my $buf  = undef;

print "$server : $port\n";

socket(SOCK, PF_INET, SOCK_STREAM, 6) or die "Can't create socket: $!";
connect(SOCK, $dest)		      or die "Can't connect: $!";

my $header;

# Send header
$header = &PackHeader($msg_type, $msg_sub_type, length($msg_content));
syswrite(SOCK, $header, length($header));

# Send content
if($msg_content ne "")
{
	syswrite(SOCK, $msg_content, length($msg_content));
}

# Receive reply
my $bs = sysread(SOCK, $buf, 2048);
&UnpackReply($buf, $bs);

close(SOCK);


