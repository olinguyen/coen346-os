mkdir bin/
echo -e "\nBuilding binaries\n"
cd src

files=`ls -1 *.c | awk -F"." '{ print $1 }'`

for i in $files
do
   echo Compiling ${i}.c
   gcc ${i}.c -lpthread -o ../bin/${i}
   if [ $? != 0 ]; then
		echo Compilation failed, please check your configuration.
		exit 1
   fi
done
