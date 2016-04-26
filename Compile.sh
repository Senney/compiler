BINDIR=./qemu
AS=$BINDIR/arm-none-elf-as
LD=$BINDIR/arm-none-elf-ld
EM=qemu-system-arm
COMPILER=./Compiler

ASM=out.s
KERNEL=a.out
OBJS=a.out.o

if [ $1 == "run" ]; then
	echo "Running a.out."
	$EM -cpu any -nographic -monitor null -serial null -semihosting -kernel $KERNEL
	exit 0
fi

# Compile to arm ASM
$COMPILER $1

# Compile the file
$AS $ASM -o $OBJS

# Link it
$LD -o $KERNEL -Ttext 0x10000 $OBJS
rm $OBJS
echo "Program saved to 'a.out'."

