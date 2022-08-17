#!/bin/bash

FTFILENAME=fattestsmall.img
FTFSIZE=256
USERNAME=`echo $1`

if [[ $1 == "--help" ]]; then
	echo "generates a fat32 filesystem in a file "
	echo "the file generated is $FTFILENAME with a size of $FTFSIZE MB"
	echo "this is defined in this script"
	echo "usage: sudo mktestvfat.sh <username>"
	echo "optional arguments (after username): -m: mount only, -u: unmount only"
else

	if [[ $# -eq 2 ]]; then
	  case $2 in
	  -m)
	    echo "mounting only"
	    LOOPDEV=`losetup --show --offset 1048576 -f $FTFILENAME`
	    echo "attached to $LOOPDEV" 
	    echo "mounting the file system"
	    mount $LOOPDEV  /home/$USERNAME/fattest 
	    ;;
	  -u)
	    echo "unmounting"
	    LOOPDEV=`mount | grep fattest | grep -o ^/[a-zA-Z0-9/]*`
	    umount $LOOPDEV
	    echo "detaching loop device"
	    losetup -d $LOOPDEV
	    ;;
	  *)
	    echo "unknown command option $2"
	    ;;
	  esac
	else
	
		echo "generating empty file"
		dd if=/dev/zero of=$FTFILENAME bs=1M count=$FTFSIZE
		chown $USERNAME ./$FTFILENAME
		echo "creating the partition table"
		fdisk ./$FTFILENAME <<EEOF
n
p
1


t
c
w
EEOF
		echo "attach loop device"
		LOOPDEV=`losetup --show --offset 1048576 -f $FTFILENAME`
		echo "attached to $LOOPDEV" 
		echo "creating fat file system"
		mkfs.vfat -F 32 -s 4 $LOOPDEV
		echo "mounting the created file system"
		mount $LOOPDEV  /home/$USERNAME/fattest 
		echo "creating some directories"
		mkdir /home/$USERNAME/fattest/dir1
		mkdir /home/$USERNAME/fattest/ndir
		echo "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet." > /home/$USERNAME/fattest/file1.txt
		echo " A volume boot record (VBR) (also known as a volume boot sector, a partition boot record or a partition boot sector) is a type of boot sector introduced by the IBM Personal Computer. It may be found on a partitioned data storage device, such as a hard disk, or an unpartitioned device, such as a floppy disk, and contains machine code for bootstrapping programs (usually, but not necessarily, operating systems) stored in other parts of the device. On non-partitioned storage devices, it is the first sector of the device. On partitioned devices, it is the first sector of an individual partition on the device, with the first sector of the entire device being a Master Boot Record (MBR) containing the partition table. The code in volume boot records is invoked either directly by the machine\'s firmware or indirectly by code in the master boot record or a boot manager. Code in the MBR and VBR is in essence loaded the same way. Invoking a VBR via a boot manager is known as chain loading. Some dual-boot systems, such as NTLDR (the boot loader for all releases of Microsoft\'s Windows NT-derived operating systems up to and including Windows XP and Windows Server 2003), take copies of the bootstrap code that individual operating systems install into a single partitions VBR and store them in disc files, loading the relevant VBR content from file after the boot loader has asked the user which operating system to bootstrap. In Windows Vista, Windows Server 2008 and newer versions, NTLDR was replaced; the boot-loader functionality is instead provided by two new components: WINLOAD.EXE and the Windows Boot Manager. In file systems such as FAT12 (except for in DOS 1.x), FAT16, FAT32, HPFS and NTFS, the VBR also contains a BIOS Parameter Block (BPB) that specifies the location and layout of the principal on-disk data structures for the file system. (A detailed discussion of the sector layout of FAT VBRs, the various FAT BPB versions and their entries can be found in the FAT article.)" > /home/$USERNAME/fattest/test1.txt
		echo "unmounting"
		umount $LOOPDEV
		echo "detaching loop device"
		losetup -d $LOOPDEV
		echo "done"

	   
	fi

fi


