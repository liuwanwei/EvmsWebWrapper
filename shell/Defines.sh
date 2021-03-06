# error code definition
error_ok=0			# success
error_fail=1			# failed
error_param=3			# invalid paramerater 
error_notexist=4		# target not exist
error_exist=5			# target already exist

error_modules=10		# driver modules not ready
error_directories=20		# directories not ready
error_daemons=30		# service daemons not ready

base_srpt_dir="/proc/scsi_tgt/"
group_ctrl_file=$base_srpt_dir"scsi_tgt"
base_group_dir=$base_srpt_dir"groups"

base_vdisk_dir=$base_srpt_dir"vdisk/"
vdisk_ctrl_file=$base_vdisk_dir"vdisk"

tmp_group_name_file="/tmp/groups_list"

shell_scripts_dir="/usr/sbin/sanager/"
groups_bk_dir=$shell_scripts_dir"groups_bk"
group_backup_script=$shell_scripts_dir/"GroupInfoBackup.sh"
group_restore_script=$shell_scripts_dir/"GroupInfoRestore.sh"
vdisk_bk_file=$shell_scripts_dir"vdisk"

iscsi_proc_dir="/proc/net/iet/"
iscsi_proc_volume_ctrl_file=$iscsi_proc_dir"volume"
iscsi_proc_session_ctrl_file=$iscsi_proc_dir"session"

mount_dir="/mnt/"

startup_log="/tmp/sanager_startup"
return_value_file="/tmp/return_value"
