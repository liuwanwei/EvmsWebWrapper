#!/usr/bin/perl


sub test()
{
	print join(':', split /[ \n\|]/, $_[0]) ."\n";
}

#&test("liuwan|17 \n");

$module_conf = "/etc/modprobe.d/modprobe.conf";
$tmp_file = "/tmp/return_value_bonding";

#system("sed -r -e 's/(^options bond0).*/\\1 mode=333/g' $module_conf");

#system("cat $tmp_file");

$cfg_file="test.cfg";
$name="bond0";
$ip="192.168.1.66";
$mask="255.255.255.0";

#system("echo \"DEVICE=$name \nIPADDR=$ip \nNETMASK=$mask \nONBOOT=yes \nBOOTPROTO=none \nUSERCTRL=no \n\" > $cfg_file");

#system("cat $cfg_file");

system("sed -e '/^alias $value/d' -e '/^options $value/d' $module_conf > $tmp_file");
system("cp $tmp_file $module_conf -f");
