cnt::Counter();
src::InfiniteSource(DATA "HELLO", LENGTH 40960, BURST 1);
//src::RandomSource(LENGTH 40960, BURST 1000);
sink::Discard(BURST 1);
src -> BasicElementNoSGXSHA256 -> Queue() -> cnt -> sink;
DriverManager(wait 10s, save cnt.count -, save src.count -, save sink.count -, 
              save cnt.byte_count -, save cnt.rate -, save cnt.bit_rate -,
              save cnt.byte_rate -, stop)
