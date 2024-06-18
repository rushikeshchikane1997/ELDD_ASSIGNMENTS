#include<linux/module.h>
#include<linux/init.h>
#include<linux/device.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/kfifo.h>
#include<linux/slab.h>

#define MAX 32 

static int pchar_open(struct inode *, struct file *);
static int pchar_close(struct inode *, struct file *);
static ssize_t pchar_read(struct file *, char *, size_t, loff_t *);
static ssize_t pchar_write(struct file *, const char *, size_t, loff_t *);


// device private struct  creation
struct pchar_device {
	struct kfifo buf;
	dev_t devno;
	struct cdev cdev;
};





static int major,minor;
static struct class *pclass;
static int devcnt ;
struct pchar_device *devices;

module_param(devcnt,int,0644);


static struct file_operations pchar_fops = {
	.owner = THIS_MODULE,
	.open = pchar_open,
	.release = pchar_close,
	.write = pchar_write,
	.read = pchar_read
};





static __init int pchar_init(void) {
	int ret,i ;
	struct device *pdevice;
	dev_t devno ;

	printk(KERN_INFO "%s : pchar_init module is called \n",THIS_MODULE->name);

	devices = (struct pchar_device*)kmalloc(devcnt*sizeof(struct pchar_device),GFP_KERNEL);
	if(devices==NULL) { 
		ret = -ENOMEM;
		printk(KERN_ERR "%s : kmalloc () failed to allocate devices private struct memory.\n", THIS_MODULE->name);
		goto devices_kmalloc_failed ;
	}
	printk(" %s kmalloc succesfully allocated the memory.\n",THIS_MODULE->name);
	for(i =0; i<devcnt; i++) { 
		ret = kfifo_alloc(&devices[i].buf,MAX, GFP_KERNEL);
		if(ret!=0){
			printk(KERN_ERR "%s kfifo allloc is fasiled to allcoate devices %d.\n",THIS_MODULE->name,i);
			goto kfifo_alloc_failed;
		}
	}
	printk(KERN_INFO "%s: kfifo_alloc succesfully created  %d devices \n",THIS_MODULE->name,devcnt);

	ret=alloc_chrdev_region(&devno,0,devcnt,"pchar");
	if(ret!=0) {
		printk(KERN_ERR "%s chrdev failed to allocate the region\n", THIS_MODULE->name);
		goto alloc_chrdev_region_failed;
	}
	major = MAJOR(devno);
	minor = MINOR(devno);
	printk(KERN_INFO "%s alloc chrdev region suceesfully dev no. %d/%d.\n", THIS_MODULE->name,major,minor);
	pclass = class_create("pchar_class");

	printk(KERN_INFO"%s : class is successfully created .\n",THIS_MODULE->name);
	for(i=0;i<devcnt;i++){
		devno = MKDEV(major,i);
		pdevice = device_create(pclass,NULL,devno,NULL,"pchar%d",i);
		if(IS_ERR(pdevice)) {
			printk(KERN_ERR "%s: device create is failed for device %d.\n",THIS_MODULE->name,i);
			ret = -1;
			goto device_create_failed;
		}
	}
	printk(KERN_INFO "%s device_create  is created device  files.\n", THIS_MODULE->name);

	for(i=0; i<devcnt; i++)
	{
		devno = MKDEV(major,i);
		devices[i].devno = devno;
		cdev_init(&devices[i].cdev,&pchar_fops);
		ret = cdev_add(&devices[i].cdev,devno,3);
		if(ret != 0) {
			printk(KERN_ERR "%s: cdev_add() failed to add cdev %d in kernel db.\n", THIS_MODULE->name, i);
			goto cdev_add_failed;
		}


		printk(KERN_INFO "%s: cdev_add() added devices in kernel db.\n", THIS_MODULE->name);
	}

	return 0;

cdev_add_failed:
	for(i=i-1; i>=0; i--)
		cdev_del(&devices[i].cdev);
	i = devcnt;	
device_create_failed:
	for(i=i-1; i>=0; i--) {
		devno = MKDEV(major, i);
		device_destroy(pclass, devno);
	}
	unregister_chrdev_region(devno,3);
alloc_chrdev_region_failed:
	i=devcnt;
kfifo_alloc_failed:
	for(i=i-1; i>=0; i--)
		kfifo_free(&devices[i].buf);
	kfree(devices);
devices_kmalloc_failed:
	return ret;
}




static __exit void pchar_exit(void) {  
	int i ;
	dev_t devno;
	printk(KERN_INFO "%s : pchar exit is called ",THIS_MODULE->name );
	for(i=devcnt-1; i>=0; i--)
		{
		cdev_del(&devices[i].cdev);
		}
	printk(KERN_INFO "%s: cdev_del() removed devices from kernel db.\n", THIS_MODULE->name);
	for(i=devcnt-1; i>=0; i--) {
		devno = MKDEV(major, i);
		device_destroy(pclass, devno);
	}
	printk(KERN_INFO "%s: device_destroy() destroyed device files.\n", THIS_MODULE->name);
	class_destroy(pclass);
	printk(KERN_INFO "%s: class_destroy() destroyed device class.\n", THIS_MODULE->name);
	unregister_chrdev_region(devno, 1);
	printk(KERN_INFO "%s: unregister_chrdev_region() released device number.\n", THIS_MODULE->name);

	for(i=devcnt-1; i>=0; i--){
		kfifo_free(&devices[i].buf);
	}
	printk(KERN_INFO "%s: kfifo_free() destroyed devices.\n", THIS_MODULE->name);
	kfree(devices);
	printk(KERN_INFO "%s: kfree() released devices private struct memory.\n", THIS_MODULE->name);

}

static int pchar_open(struct inode *pinode , struct file *pfile) 
{
	printk(KERN_INFO "%s pchar_open is called \n", THIS_MODULE->name);
	struct pchar_device *pdev = container_of(pinode->i_cdev, struct pchar_device , cdev);
	pfile->private_data = pdev;

	return 0;
}

static int pchar_close(struct inode *pinode , struct file *pfile)
{
	printk(KERN_INFO " %s PCHAR_CLOSE IS CALLED \n",THIS_MODULE->name);
	struct pchar_device *pdev = (struct pchar_device *)pfile->private_data;
	(void)pdev;

	return 0;
}


static ssize_t pchar_read(struct file *pfile, char *ubuf, size_t size, loff_t *poffset) {
    int nbytes=0, ret;
    printk(KERN_INFO "%s: pchar_read() called.\n", THIS_MODULE->name);
    struct pchar_device *pdev = (struct pchar_device *)pfile->private_data;
    ret = kfifo_to_user(&pdev->buf, ubuf, size, &nbytes);
    if(ret < 0) {
        printk(KERN_ERR "%s: pchar_read() failed to copy data from kernel space using kfifo_to_user().\n", THIS_MODULE->name);
        return ret;     
    }
    printk(KERN_INFO "%s: pchar_read() copied %d bytes to user space.\n", THIS_MODULE->name, nbytes);
    return nbytes;
}

static ssize_t pchar_write(struct file *pfile, const char *ubuf, size_t size, loff_t *poffset) {
    int nbytes=size, ret;
    printk(KERN_INFO "%s: pchar_write() called.\n", THIS_MODULE->name);
    struct pchar_device *pdev = (struct pchar_device *)pfile->private_data;
    ret = kfifo_from_user(&pdev->buf, ubuf, size, &nbytes);
    if(ret < 0) {
        printk(KERN_ERR "%s: pchar_write() failed to copy data in kernel space using kfifo_from_user().\n", THIS_MODULE->name);
        return ret;     
    }
    printk(KERN_INFO "%s: pchar_write() copied %d bytes from user space.\n", THIS_MODULE->name, nbytes);
    return nbytes;
}

static long pchar_ioctl(struct file *pfile, unsigned int cmd, unsigned long param) {
	info_t info;
	switch (cmd)
	{
		case FIFO_CLEAR:
			printk(KERN_INFO "%s: pchar_ioctl() fifo clear.\n", THIS_MODULE->name);
			kfifo_reset(&buf);
			break;
		case FIFO_INFO:
			printk(KERN_INFO "%s: pchar_ioctl() get fifo info.\n", THIS_MODULE->name); 
			info.size = kfifo_size(&buf);
			info.avail = kfifo_avail(&buf);
			info.len = kfifo_len(&buf);
			copy_to_user((void*)param, &info, sizeof(info_t));
			break;
		case FIFO_RESIZE:
			printk(KERN_INFO "%s: pchar_ioctl() resize fifo.\n", THIS_MODULE->name);

			// Copy the new size from user space
			copy_from_user((long *)param,&new_size,sizeof(long)); 
			//printk(KERN_ERR "%s: pchar_ioctl() copy_from_user() failed.\n", THIS_MODULE->name);
			//return -EFAULT;


			// Ensure the new size is valid
			if (new_size <= 0) {
				printk(KERN_ERR "%s: pchar_ioctl() invalid new size.\n", THIS_MODULE->name);
				return -EINVAL;




				fifo_len = kfifo_len(&buf);

				// Allocate temporary buffer of current FIFO length
				temp_buf = kmalloc(fifo_len, GFP_KERNEL);
				if (!temp_buf) {
					printk(KERN_ERR "%s: pchar_ioctl() kmalloc() failed.\n", THIS_MODULE->name);
					return -ENOMEM;
				}

				// Copy data from FIFO to temporary buffer
				kfifo_out(&buf, temp_buf, fifo_len);

				// Release existing FIFO memory
				kfifo_free(&buf);


				// Allocate new FIFO with new size
				ret = kfifo_alloc(&buf, new_size, GFP_KERNEL);
				if (ret != 0) {
					printk(KERN_ERR "%s: pchar_ioctl() kfifo_alloc() failed for new size.\n", THIS_MODULE->name);
					kfree(temp_buf);
					return ret;
				}


				// Copy data back from temporary buffer to new FIFO

				kfifo_in(&buf, temp_buf, fifo_len);
				//
				//        copy_from_user((void*)param, &newsize, sizeof(long));   
				// Release temporary buffer

				kfree(temp_buf);

				// copy_from_user((long*)param, &newsize, sizeof(long));
				break;
				default:
				printk(KERN_ERR "%s: pchar_ioctl() unsupported command.\n", THIS_MODULE->name); 
				return -EINVAL;
			}
			return 0;
	}
















 module_init(pchar_init);
 module_exit(pchar_exit);
 
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("Nilesh Ghule <nilesh@sunbeaminfo.com>");
 MODULE_DESCRIPTION("Simple pchar driver with kfifo as device.");

