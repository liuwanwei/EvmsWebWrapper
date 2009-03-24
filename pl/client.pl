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

while($#ARGV != -1)
{
	if($ARGV[0] =~ /-t/i)
	{
		$main_msg_type = $ARGV[1];
		shift;
	}
	elsif($ARGV[0] =~ /-s/i)
	{
		$sub_msg_type = $ARGV[1];
		shift;
	}
	else
	{
		#print "A: $ARGV[0]\n";

		if($msg_content)
		{
			$msg_content = $msg_content."|".$ARGV[0];
		}
		else
		{
			$msg_content = $ARGV[0];
		}

		#print "B: $msg_content\n";
	}

	shift;
}

my $dest = sockaddr_in($port, inet_aton($server));
my $buf  = undef;

print "$server : $port\n";
print "main_msg_type : $main_msg_type\n";
print "sub_msg_type  : $sub_msg_type\n";
print "msg_content   : $msg_content\n";
my $msg_len = length($msg_content);
print "msg_body_len  : $msg_len\n\n\n";


socket(SOCK, PF_INET, SOCK_STREAM, 6) or die "Can't create socket: $!";
connect(SOCK, $dest)		      or die "Can't connect: $!";

my $header;

# Send header
$header = &PackHeader($main_msg_type, $sub_msg_type, length($msg_content));
syswrite(SOCK, $header, length($header));

# Send body if has
if($msg_len gt 8)
{
	syswrite(SOCK, $msg_content, $msg_len);
}

# Send content
if($msg_content ne "")
{
	syswrite(SOCK, $msg_content, length($msg_content));
}

# Receive reply
my $bs = sysread(SOCK, $buf, 2048);
&UnpackReply($buf, $bs);

close(SOCK);

