#sector from 1 to 19999: no partition(this is for kernel binary)
#sector from 20000 to end: minix partition(this is for root file system)

#create device for os.img
#sudo losetup /dev/loop0 os.img

#read partitions and create devices accordingly
#sudo kpartx -av /dev/loop0

#format it to the minix filesystem
#sudo mkfs.minix -1 -s /dev/mapper/loop0p1

#mount the first partition
#sudo mount /dev/mapper/loop0p1 /mnt/ix_root

#umount and release devices
#sudo umount /mnt/ix_root
#sudo kpartx -dv /dev/loop0
#sudo losetup -d /dev/loop0

rm -rf os.img
# Create a floppy disk image 
#dd if=/dev/zero of=os.img bs=512 count=720

# Create a hard disk image
bximage -mode=create -hd=10 os.img -q
# Create a partition startting section# 20000
echo -e "n\n\n\n20000\n\nt\n0\nw\n" | sudo fdisk os.img

# Format the partition and copy command binaries 
	#create device for os.img
	sudo losetup /dev/loop0 os.img

	#read partitions and create devices accordingly
	sudo kpartx -av /dev/loop0

	#format it to the minix filesystem
	sudo mkfs.minix -1 /dev/mapper/loop0p1

	#mount the filesystem
	sudo mount /dev/mapper/loop0p1 /mnt/ix_root/

	#copy root file system on the system partition
	sudo mknod ./rootfs/dev/tty c 2 0	# creating /dev/tty device with major 2 and minor 0 
	sudo cp -rv ./sh/sh ./rootfs/bin/
	sudo cp -rv ./bin/ls/ls ./rootfs/bin/
	sudo cp -rv ./rootfs/* /mnt/ix_root/

	#unmount the device
	sudo umount /mnt/ix_root
	sudo kpartx -dv /dev/loop0
	sudo losetup -d /dev/loop0

# Copy os image to disk
dd status=noxfer conv=notrunc if=ix.bin of=os.img bs=1 count=446	# copy only bootloader code area and preserve the partition talbe in MBR
dd status=noxfer conv=notrunc if=ix.bin of=os.img bs=1 seek=508 skip=508 count=4		# copy boot signature 
dd status=noxfer conv=notrunc if=ix.bin of=os.img bs=512 seek=1 skip=1		# copy the rest of os 
