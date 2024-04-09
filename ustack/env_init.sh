ip link set eth0 down
ip link set eth1 down
modprobe uio_pci_generic
# cd /home/dpdk/dpdk-stable-22.11.4/usertools
dpdk-devbind.py --status
dpdk-devbind.py --bind=uio_pci_generic 03:00.0
dpdk-devbind.py --bind=uio_pci_generic 0b:00.0
dpdk-devbind.py --status
