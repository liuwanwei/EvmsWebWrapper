#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__


/*
 * Called by applications, including ib,iscsi,bonding apps, to
 * send back tcp packet to requestor on the peer.
 */
int SendAppFrame(int sock_fd, 
                 unsigned char type, 
                 unsigned char sub_type, 
                 unsigned short retcode, 
                 char * data, 
                 unsigned char data_len);

/*
 * Execute shell or perl scripts named by 'shell_script_name',
 * using 'execve()'.
 */
int CallShell(const char * shell_script_name, 
              const char * shell_func_name, 
              char * param_list);

/*
 * Get shell or perl scripts' output from memory filesystem.
 * Each line in file will be treated as one record to specific 
 * object.
 * Records will be delimited by C type ending charactor '\0'.
 * All records will be combined into one 'BIG' buffer referenced
 * by 'reply', and 'reply_len' represents the buffer's real length.
 */
int GetLinedupResultsFromFile(const char * filename, 
                              char ** reply, 
                              int * reply_len);

#endif
