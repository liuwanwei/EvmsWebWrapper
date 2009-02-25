#!/bin/perl

my $PACKET_HDR_LEN = 8;

sub PackHeader()
{
	my $header;

	$header = pack("S C C S S", $PACKET_HDR_LEN + $_[2], $_[0], $_[1], 0, 0);

	return $header;
}

sub UnpackReply()
{
	my $header = $_[0];
	my $len = $_[1];
	my $content_len= $len - $PACKET_HDR_LEN;
	my @list;
	my @sub_list;

	if($content_len gt 0)
	{
		@list = unpack("S C C S S a$content_len", $header);
	}
	else
	{
		@list = unpack("S C C S S", $header);
	}

	print "msg_len:      $list[0]\n";
	print "msg_type:     $list[1]\n";
	print "msg_sub_type: $list[2]\n";
	print "msg_order:    $list[3]\n";
	print "msg_retcode:  $list[4]\n";
	print "msg_body:     $list[5]\n" if($content_len gt 0);
}

sub Usage()
{
	print "Usage : \n";
	print "./client -t main_msg_type -s sub_msg_type msg_body\n";
	exit 1;
}

1;
