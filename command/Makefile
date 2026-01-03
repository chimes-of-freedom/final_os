# commands/Makefile

#ENTRYPOINT	= 0x1000
HD		= ../80m.img

ASM		= nasm
DASM		= objdump
CC		= gcc
LD		= ld
ASMFLAGS	= -I ../include/ -I ../include/sys -f elf
CFLAGS		= -I ../include/ -I ../include/sys -c -fno-builtin -fno-stack-protector -Wall
LDFLAGS		= -Ttext 0x1000
DASMFLAGS	= -D
LIB		= ../lib/orangescrt.a
BIN		= echo pwd ps kill ls touch rm editor logman cat poc_fs

# All Phony Targets
.PHONY : everything final clean realclean disasm all install

# Default starting position
everything : $(BIN)

install : all clean
	cp ../kernel.bin ./ -v
	tar vcf inst.tar kernel.bin $(BIN)
	dd if=inst.tar of=$(HD) seek=`echo "obase=10;ibase=16;(\`egrep -e '^ROOT_BASE' ../boot/include/load.inc | sed -e 's/.*0x//g'\`+\`egrep -e '#define[[:space:]]*INSTALL_START_SECT' ../include/sys/config.h | sed -e 's/.*0x//g'\`)*200" | bc` bs=1 count=`ls -l inst.tar | awk '{print $$5}'` conv=notrunc

all : realclean everything

final : all clean

clean :
	rm -f *.o

realclean :
	rm -f $(BIN) *.o

kernel.bin :
	cp ../kernel.bin ./

start.o : start.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

echo.o: echo.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

echo : echo.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

pwd.o: pwd.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

pwd : pwd.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

ps.o: ps.c ../include/type.h ../include/stdio.h ../include/sys/proc.h ../include/sys/const.h ../include/string.h ../include/sys/proto.h
	$(CC) $(CFLAGS) -o $@ $<

ps : ps.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

kill.o: kill.c ../include/stdio.h ../include/sys/const.h ../include/string.h ../include/sys/proto.h
	$(CC) $(CFLAGS) -o $@ $<

kill : kill.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

logman.o: logman.c ../include/stdio.h ../include/sys/const.h ../include/string.h ../include/sys/proto.h
	$(CC) $(CFLAGS) -o $@ $<

logman : logman.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

ls.o: ls.c ../include/type.h ../include/stdio.h ../include/string.h ../include/sys/fs.h ../include/sys/const.h
	$(CC) $(CFLAGS) -o $@ $<

ls : ls.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

touch.o: touch.c ../include/type.h ../include/stdio.h ../include/string.h ../include/sys/fs.h ../include/sys/const.h ../include/sys/proto.h
	$(CC) $(CFLAGS) -o $@ $<

touch : touch.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

rm.o: rm.c ../include/type.h ../include/stdio.h ../include/string.h ../include/sys/fs.h
	$(CC) $(CFLAGS) -o $@ $<

rm : rm.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

editor.o: editor.c ../include/type.h ../include/stdio.h ../include/string.h ../include/sys/fs.h
	$(CC) $(CFLAGS) -o $@ $<

editor : editor.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

cat.o: cat.c ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

cat : cat.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

# ========== Vulnerability PoC Programs ==========

poc_fs.o: poc_fs.c ../include/stdio.h ../include/string.h
	$(CC) $(CFLAGS) -o $@ $<

poc_fs : poc_fs.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?
