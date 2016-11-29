devIn0::FromDevice(ens9, SNIFFER false, PROMISC true);
devIn1::FromDevice(ens10, SNIFFER false, PROMISC true);

devIn0 -> BasicElement -> Queue(100) -> ToDevice(ens10);
devIn1 -> Queue(100) -> ToDevice(ens9);
