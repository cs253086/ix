l=/usr/lib
all:	
	make bootblok
	make build
	make fsck

bootblok:	bootblok.s
		asld bootblok.s
		dd if=a.out of=bootblok bs=16w skip=1 count=16 2>/dev/null
		rm a.out
		@echo bootblok done

build:	build.c
	cc -o build build.c
	@echo build done

fsck:	fsck.s fsck1.s
	@echo Start linking fsck...
	asld -o fsck -T. fsck1.s fsck.s $l/libc.a $l/end.s
	@echo fsck done
fsck.s: fsck.c
	cc -c -w -F -T. fsck.c

image: 	build bootblok kernel fsck
	@echo building os image...
	build bootblok kernel fsck /dev/fd1
	@echo os image build done
