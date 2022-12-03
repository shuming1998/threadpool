# CPP_Thread_Pooling
C++ thread pooling (SHARED library after building)

how to use:

```shell
$ ./autobuild.sh
```



```C++
// what you want do
class YourTask : public Task {
public:
	// define your construct 
    YourTask(params);
    // all works you need to do will be done there
	void run() override;
private:
   	// define anything you need
};

int main() {
    ThreadPool pool;
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

