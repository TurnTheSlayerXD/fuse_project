mkdir -p ./mountpoint;
umount -q ./mountpoint;
cmake --build . && ./fuse_test ./mountpoint/

