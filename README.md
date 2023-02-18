# threadpool
C++ thread pool (SHARED library after building)

how to use:

```shell
$ git clone git@github.com:shuming1998/threadpool.git
$ cd threadpool
$ ./autobuild.sh
```



```C++
#include "threadpool.h"
// define the your task class
class YourTask : public Task {
public:
    // define your construct 
    YourTask(params);
    // all works you need to do will be done there
    void run() override;
private:
   	// define anything you need to do
};

int main() {
    ThreadPool pool;
    // choose work mode if needed, default is MODE_FIXED，which set thread nums by machine core nums
    pool.setMode(PoolMode::MODE_CACHED);
    pool.setMaxThreadFreeTime_(60);
    pool.start();
    
    // submit Task，define your construct
    Result res = pool.submitTask(std::make_shared<YourTask>(YourTask(params)));
    // get task's return value, [type] is the type you need
    type t = res.get().cast<type>();
	return 0;
}
```

