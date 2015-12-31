rmmod mydev
make
insmod mydev.ko
rm /dev/mysimpledriver
mknod /dev/mysimpledriver c 246 0 #be sure to check the correct number from : cat /proc/devices | grep eadriver
echo "test" > /dev/mysimpledriver
cat /dev/mysimpledriver
