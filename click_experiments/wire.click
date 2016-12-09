devIn0::FromDevice(eno1, SNIFFER false, PROMISC true);
devIn1::FromDevice(enxa0cec806f10f, SNIFFER false, PROMISC true);

devIn0 -> Queue(100) -> ToDevice(enxa0cec806f10f);
devIn1 -> Queue(100) -> ToDevice(eno1);
