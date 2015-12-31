rmmod mydev
make
insmod mydev.ko
rm /dev/mysimpledriver
mknod /dev/mysimpledriver c 246 0
echo "test" > /dev/mysimpledriver
cat /dev/mysimpledriver
