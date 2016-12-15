cnt::Counter();
src::InfiniteSource(DATA "HELLOOOOOOOOOOO\n", LENGTH 40960, BURST 1);
//src::RandomSource(LENGTH 40960, BURST 1);
sink::Discard(BURST 1);
sgx::BasicElement();
src -> sgx -> Queue() -> cnt -> sink;
DriverManager(wait 10s, print "$(cnt.count),$(src.count),$(sink.count),$(cnt.byte_count),$(cnt.rate),$(cnt.bit_rate),$(cnt.byte_rate)", stop);
