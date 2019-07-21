rm -rf os.img
# Create a floppy disk image 
#dd if=/dev/zero of=os.img bs=512 count=720

# Create a hard disk image
bximage -mode=create -hd=10 os.img -q
# Copy os image to disk
dd status=noxfer conv=notrunc if=ix.bin of=os.img 
