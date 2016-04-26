COMPILE=./Compile.sh

DIR=Tests/M4/
FILES=(gen.t1 gen.t10 gen.t11 gen.t12 gen.t13 gen.t14 gen.t15 gen.t22 gen.t26 gen.t29 gen.t30 gen.t31 gen.t32 gen.34)

for f in "${FILES[@]}"
do
	echo "******************************"
	echo "Testing $f."
	echo "******************************"
	$COMPILE $DIR/$f
	$COMPILE run
	echo "******************************"
	echo "$f testing is complete!"
	echo "******************************"
done

