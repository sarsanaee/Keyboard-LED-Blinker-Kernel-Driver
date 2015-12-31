rmmod mydev
make
insmod mydev.ko
rm /dev/mysimpledriver
mknod /dev/mysimpledriver c 246 0
#echo "alireza sanae is someone mad in computer science" > /dev/mysimpledriver
echo "test" > /dev/mysimpledriver
cat /dev/mysimpledriver
