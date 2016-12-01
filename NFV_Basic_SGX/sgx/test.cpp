#include "App.h"

int main(){
    int ret = initialize_enclave();
    if(ret < 0){
      std::cout << "Enclave intialization failed" << std::endl;
      return -1;
    }
    run_server();
    return 0;
}
