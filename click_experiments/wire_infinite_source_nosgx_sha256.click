cnt::Counter();
src::InfiniteSource(DATA "HELLO", LENGTH 40960, BURST 1);
sink::Discard(BURST 1);
src -> BasicElementNoSGXSHA256 -> Queue() -> cnt -> sink;
DriverManager(wait 10s, print "$(cnt.count),$(src.count),$(sink.count),$(cnt.byte_count),$(cnt.rate),$(cnt.bit_rate),$(cnt.byte_rate)", stop);
