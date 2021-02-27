
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/hash.h>
#include <linux/gcd.h>
#include <linux/jiffies.h>

#define BUFFER_SIZE 128
#define PROC_NAME "seconds"
#define MESSAGE "Hello World\n"

static long jiffiesInit = 0;

/**
 * Function prototypes
 */
ssize_t proc_read(struct file *file, char *buf, size_t count, loff_t *pos);
static struct file_operations proc_ops = {
        .owner = THIS_MODULE,
        .read = proc_read,
};
/* This function is called when the module is loaded. */
int proc_init(void) {
        // creates the /proc/seconds entry
        // the following function call is a wrapper for
        // proc_create_data() passing NULL as the last argument
        proc_create(PROC_NAME, 0, NULL, &proc_ops);
        printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
        jiffiesInit = (long)jiffies;
        printk( KERN_INFO "In proc_init(), Jiffies is: %lu and HZ is: %d\n", jiffies, HZ);
    return 0;
}
/* This function is called when the module is removed. */
void proc_exit(void) {
        // removes the /proc/seconds entry
        remove_proc_entry(PROC_NAME, NULL);
        printk( KERN_INFO "In proc_exit(), seconds is: %lu\n", (jiffies-jiffiesInit) / HZ);
        printk( KERN_INFO "/proc/%s removed\n", PROC_NAME);
}
/**
 * This function is called each time the /proc/seconds is read,
 * and is called repeatedly until it returns 0, so
 * there must be logic that ensures it ultimately returns 0
 * once it has collected the data that is to go into the 
 * corresponding /proc file.
 * params:
 * file:
 * buf: buffer in user space
 * count:
 * pos:
 */
ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos) {
        int rv = 0;
        char buffer[BUFFER_SIZE];
        static int completed = 0;
        if (completed) {
                completed = 0;
                return 0;
        }
        completed = 1;
        rv = sprintf(buffer, "seconds in /proc/read is: %lu\n", (jiffies-jiffiesInit) / HZ);

        printk( KERN_INFO "in proc_read(), seconds is: %lu\n", (jiffies-jiffiesInit) / HZ);

        // copies the contents of buffer to userspace usr_buf
        copy_to_user(usr_buf, buffer, rv);
        return rv;
}
/* Macros for registering module entry and exit points. */
module_init( proc_init );
module_exit( proc_exit );
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("seconds Module");
MODULE_AUTHOR("SGG");

//-------------------------------------------------------------------
// shell script for seconds.ko
//-------------------------------------------------------------------

// #!/bin/bash
// ​
// # clear the message queue, then
// # insert simple.ko into the kernel and remove it again, then
// # display the messages as a result of doing so
// ​
// sudo -p "osc"
// sudo dmesg -c 
// clear 
// ​
// printf "beginning script...\n\n"
// sudo insmod hello.ko

// cat /proc/seconds
// sleep 1
// cat /proc/seconds
// sleep 2
// cat /proc/seconds
// sleep 3
// cat /proc/seconds

// sudo rmmod seconds
// sudo dmesg 
// printf "\n...end of script\n\n"

//--------------------------------------------------------------
//output from seconds.ko
//--------------------------------------------------------------

// beginning script...

// seconds in /proc/read is: 0
// seconds in /proc/read is: 1
// seconds in /proc/read is: 3
// seconds in /proc/read is: 6
// [ 8413.436429] /proc/seconds created
// [ 8413.436432] In proc_init(), Jiffies is: 4296995604 and HZ is: 250
// [ 8413.439736] in proc_read(), seconds is: 0
// [ 8414.453734] in proc_read(), seconds is: 1
// [ 8416.456295] in proc_read(), seconds is: 3
// [ 8419.459028] in proc_read(), seconds is: 6
// [ 8419.466096] In proc_exit(), seconds is: 6
// [ 8419.466100] /proc/seconds removed

// ...end of script

//----------------------------------------------------------------
//end of ouput from seconds.ko
//----------------------------------------------------------------
