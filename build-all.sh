mkdir -p bin/
echo -e "\nBuilding binaries\n"
cd src

files=`ls -1 *.cpp | awk -F"." '{ print $1 }'`

for i in $files
do
   echo Compiling ${i}.cpp
   g++ ${i}.cpp -lpthread -g -o ../bin/${i}
   if [ $? != 0 ]; then
		echo Compilation failed, please check your configuration.
		exit 1
   fi
done
